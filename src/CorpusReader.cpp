#include <algorithm>
#include <cassert>
#include <cstring>
#include <list>
#include <memory>
#include <regex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include <AlpinoCorpus/CorpusInfo.hh>
#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/Error.hh>
#include <AlpinoCorpus/IterImpl.hh>
#include <AlpinoCorpus/RecursiveCorpusReader.hh>
#include <AlpinoCorpus/Stylesheet.hh>

#include <typeinfo>

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlerror.h>
#include <libxml/xpath.h>

#include <xercesc/dom/DOM.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/framework/MemBufFormatTarget.hpp>
#include <xercesc/framework/Wrapper4InputSource.hpp>
#include <xercesc/util/TransService.hpp>

#include <xqilla/xqilla-dom3.hpp>

#include "FilterIter.hh"
#include "StylesheetIter.hh"
#include "util/parseString.hh"
#include "util/split.hh"

namespace xerces = XERCES_CPP_NAMESPACE;

namespace {
    void ignoreStructuredError(void *userdata, xmlErrorPtr err)
    {
    }

    xmlChar const *toXmlStr(char const *str)
    {
        return reinterpret_cast<xmlChar const *>(str);
    }

    char const *fromXmlStr(xmlChar const *str)
    {
        return reinterpret_cast<char const *>(str);
    }


    std::vector<alpinocorpus::LexItem> collectLexicals(
        std::shared_ptr<xmlDoc> doc,
        std::unordered_map<xmlNode *, std::set<size_t> > const &matchDepth,
        std::string const &attribute,
        std::string const &defaultValue,
        alpinocorpus::CorpusInfo const &corpusInfo)
    {
        std::vector<alpinocorpus::LexItem> items;

        std::shared_ptr<xmlXPathContext> xpCtx(
            xmlXPathNewContext(doc.get()), xmlXPathFreeContext);
        if (xpCtx == 0)
        {
            return items;
        }

        std::ostringstream oss;
        oss << "//" << corpusInfo.lexicalElement() <<
          "[@" << corpusInfo.tokenAttribute() << "]";
        std::string lexNodeQuery = oss.str();

        std::shared_ptr<xmlXPathObject> xpObj(xmlXPathEvalExpression(
            toXmlStr(lexNodeQuery.c_str()), xpCtx.get()), xmlXPathFreeObject);
        if (xpObj == 0)
            return items;

        xmlNodeSet *nodeSet = xpObj->nodesetval;
        if (nodeSet != 0)
        {
            for (int i = 0; i < nodeSet->nodeNr; ++i)
            {
                xmlNode *node = nodeSet->nodeTab[i];

                if (node->type == XML_ELEMENT_NODE)
                {
                    xmlAttrPtr wordAttr = xmlHasProp(node, toXmlStr(attribute.c_str()));
                    std::shared_ptr<xmlChar> word;
                    if (wordAttr == 0)
                      word = std::shared_ptr<xmlChar>(xmlStrdup(toXmlStr(defaultValue.c_str())), xmlFree);
                    else
                      word = std::shared_ptr<xmlChar>(xmlNodeGetContent(wordAttr->children), xmlFree);

                    xmlAttrPtr beginAttr = xmlHasProp(node, toXmlStr("begin"));
                    size_t begin = 0;
                    if (beginAttr)
                    {
                        std::shared_ptr<xmlChar> beginStr(
                            xmlNodeGetContent(beginAttr->children), xmlFree);
                        try {
                            begin = alpinocorpus::util::parseString<size_t>(fromXmlStr(beginStr.get()));
                        } catch (std::invalid_argument &e) {
                        }
                    }

                    alpinocorpus::LexItem item = {fromXmlStr(word.get()), begin, std::set<size_t>() };

                    auto matchIter = matchDepth.find(node);
                    if (matchIter != matchDepth.end())
                        item.matches = matchIter->second;

                    items.push_back(item);
                }
            }
        }

        std::sort(items.begin(), items.end());

        return items;
    }

    // The function adds one to the count of lexical nodes that are dominated
    // by the given node. We could modify the DOM tree directly to store such
    // counts in the lexical nodes. But frankly, manipulating the DOM tree is
    // a drag. Since we do not modify the tree, we can keep the counts by
    // memory adres. Ugly, but effective. Don't we love that?
    void markLexicals(xmlNode *node, std::unordered_map<xmlNode *,
        std::set<size_t> > *matchDepth, size_t matchId, std::string wordAttr)
    {
        // Don't attempt to handle a node that we can't.
        if (node->type != XML_ELEMENT_NODE ||
              (std::strcmp(fromXmlStr(node->name), "node") != 0 &&
               std::strcmp(fromXmlStr(node->name), "word") != 0 &&
               std::strcmp(fromXmlStr(node->name), "ne") != 0))
            return;

        xmlAttrPtr wordProp = xmlHasProp(node, toXmlStr(wordAttr.c_str()));
        if (wordProp != 0)
            (*matchDepth)[node].insert(matchId);
        else // Attempt to recurse...
        {
            for (xmlNodePtr child = node->children;
                child != NULL; child = child->next)
              markLexicals(child, matchDepth, matchId, wordAttr);
        }

    }
}

namespace alpinocorpus {
    CorpusReader::EntryIterator::EntryIterator()
    { 
    }

    CorpusReader::EntryIterator::EntryIterator(IterImpl *p) :
        d_impl(p)
    { 
    }

    CorpusReader::EntryIterator::EntryIterator(EntryIterator const &other)
    {
        copy(other);
    }

    CorpusReader::EntryIterator::~EntryIterator()
    {
    }

    CorpusReader::EntryIterator &CorpusReader::EntryIterator::operator=(
        EntryIterator const &other)
    {
        if (this != &other)
            copy(other);

        return *this;
    }

    void CorpusReader::EntryIterator::copy(EntryIterator const &other)
    {        
        if (other.d_impl)
            d_impl.reset(other.d_impl->copy());
    }
    
    
    CorpusReader::EntryIterator CorpusReader::entries(SortOrder sortOrder) const
    {
        return getEntries(sortOrder);
    }

    CorpusReader::EntryIterator CorpusReader::entriesWithStylesheet(
        Stylesheet const &stylesheet,
        std::list<MarkerQuery> const &markerQueries,
        SortOrder sortOrder) const
    {
        return EntryIterator(new StylesheetIter(getEntries(sortOrder),
            stylesheet, markerQueries));
    }

    bool CorpusReader::EntryIterator::hasNext()
    {
        // The empty iterator has no other values
        if (!d_impl)
            return false;

        return d_impl->hasNext();
    }

    bool CorpusReader::EntryIterator::hasProgress() const
    {
        // The empty iterator has no progress.
        if (!d_impl)
            return false;

        return d_impl->hasProgress();
    }

    Entry CorpusReader::EntryIterator::next(CorpusReader const &reader)
    {
      return d_impl->next(reader);
    }

    double CorpusReader::EntryIterator::progress() const
    {
      return d_impl->progress();
    }
    
    void CorpusReader::EntryIterator::interrupt()
    {
        // XXX this shouldn't be necessary, we don't do this in other places
        if (d_impl)
            d_impl->interrupt();
    }

    std::vector<LexItem> CorpusReader::getSentence(std::string const &entry,
        std::string const &query, std::string const &attribute,
        std::string const &defaultValue,
        CorpusInfo const &corpusInfo) const
    {
        auto queries = split_string(query, std::regex("\\+\\|\\+"));
        assert(queries.size() > 0);

        // Discard pre-filters
        std::string q = queries.back();

        std::list<MarkerQuery> markers;

        if (!q.empty())
        {
            MarkerQuery marker(q, "alpinocorpusActive", "1");
            markers.push_back(marker);
        }
        std::string xmlData(read(entry, markers));

        std::shared_ptr<xmlDoc> doc(
            xmlReadMemory(xmlData.c_str(), xmlData.size(), NULL, NULL, 0),
            xmlFreeDoc);

        if (doc == NULL)
            return std::vector<LexItem>();

        // We get the sentence node, we should process its children.
        xmlNode *sentenceNode = xmlDocGetRootElement(doc.get());
        if (sentenceNode == NULL) {
            return std::vector<LexItem>();
        }

        std::shared_ptr<xmlXPathContext> xpCtx(
            xmlXPathNewContext(doc.get()), xmlXPathFreeContext);

        if (xpCtx == 0)
        {
            return std::vector<LexItem>();
        }

        std::shared_ptr<xmlXPathObject> xpObj(
            xmlXPathEvalExpression(toXmlStr("//*[@alpinocorpusActive='1']"),
                xpCtx.get()),
            xmlXPathFreeObject);
        if (xpObj == 0) {
            //qDebug() << "Could not make XPath expression to select active nodes.";
            return std::vector<LexItem>();
        }

        // Do we have matches?
        xmlNodeSet *nodeSet = xpObj->nodesetval;
        std::unordered_map<xmlNode *, std::set<size_t> > matchDepth;
        if (nodeSet != 0)
        {
            for (int i = 0; i < nodeSet->nodeNr; ++i)
              if (nodeSet->nodeTab[i]->type == XML_ELEMENT_NODE)
                  markLexicals(nodeSet->nodeTab[i], &matchDepth, i,
                      corpusInfo.tokenAttribute());
        }

        std::vector<LexItem> items = collectLexicals(doc, matchDepth,
            attribute, defaultValue, corpusInfo);

        return items;
    }

    
    Either<std::string, Empty> CorpusReader::isValidQuery(QueryDialect d, bool variables, std::string const &q) const
    {
        auto queries = split_string(q, std::regex("\\+\\|\\+"));
        assert(queries.size() > 0);

        for (std::vector<std::string>::const_iterator iter = queries.begin();
                iter != queries.end(); ++iter)
        {
            Either<std::string, Empty> result = validQuery(d, variables, *iter);
            if (result.isLeft())
                return result;
        }

        return Either<std::string, Empty>::right(Empty());
    }
    
    std::string CorpusReader::name() const
    {
        return getName();
    }

    std::string CorpusReader::read(std::string const &entry,
        std::list<MarkerQuery> const &queries) const
    {
        if (queries.size() == 0)
            return readEntry(entry);
    
        // Scrub any prefilters that we may have.
        std::list<MarkerQuery> effectiveQueries(queries);
        for (std::list<MarkerQuery>::iterator iter = effectiveQueries.begin();
            iter != effectiveQueries.end(); ++iter)
        {
            auto queries = split_string(iter->query, std::regex("\\+\\|\\+"));
            assert(queries.size() > 0);
            iter->query = queries.back();
        }

        return readEntryMarkQueries(entry, effectiveQueries);
    }
        
    std::string CorpusReader::readEntryMarkQueries(std::string const &entry,
        std::list<MarkerQuery> const &queries) const
    {
        std::string content = read(entry);
        
        // Prepare the DOM parser.
        xerces::DOMImplementation *xqillaImplementation =
            xerces::DOMImplementationRegistry::getDOMImplementation(X("XPath2 3.0"));
        AutoRelease<xerces::DOMLSParser> parser(xqillaImplementation->createLSParser(
            xerces::DOMImplementationLS::MODE_SYNCHRONOUS, 0));
        
        // Parse the document.
        xerces::MemBufInputSource xmlInput(reinterpret_cast<XMLByte const *>(content.c_str()),
            content.size(), "input");

        xerces::Wrapper4InputSource domInput(&xmlInput, false);

        xerces::DOMDocument *document;
        try {
            document = parser->parse(&domInput);
        } catch (xerces::DOMException const &e) {
            throw Error(std::string("Could not parse XML data: ") + UTF8(e.getMessage()));
        }

        // No exceptions according to the documentation...
        AutoRelease<xerces::DOMXPathNSResolver> resolver(
            document->createNSResolver(document->getDocumentElement()));
        resolver->addNamespaceBinding(X("fn"),
            X("http://www.w3.org/2005/xpath-functions"));
        resolver->addNamespaceBinding(X("xs"),
            X("http://www.w3.org/2001/XMLSchema"));

        for (std::list<MarkerQuery>::const_iterator iter = queries.begin();
             iter != queries.end(); ++iter)
        {
            AutoRelease<xerces::DOMXPathExpression> expression(0);
            try {
                expression.set(document->createExpression(X(iter->query.c_str()), resolver));
            } catch (xerces::DOMXPathException const &) {
                throw Error("Could not parse expression.");
            } catch (xerces::DOMException const &) {
                throw Error("Could not resolve namespace prefixes.");
            }

            AutoRelease<xerces::DOMXPathResult> result(0);
            try {
                result.set(expression->evaluate(document,
                                                xerces::DOMXPathResult::ITERATOR_RESULT_TYPE, 0));
            } catch (xerces::DOMXPathException const &e) {
                throw Error("Could not retrieve an iterator over evaluation results.");
            } catch (xerces::DOMException &e) {
                throw Error("Could not evaluate the expression on the given document.");
            }
            
            std::list<xerces::DOMNode *> markNodes;

            try {
                while (result->iterateNext())
                {
                    xerces::DOMNode *node;
                    try {
                      node = result->getNodeValue();
                    } catch (xerces::DOMXPathException &e) {
                      throw Error("Matching node value invalid while marking nodes.");
                    }

                    // Skip non-element nodes
                    if (node->getNodeType() != xerces::DOMNode::ELEMENT_NODE)
                        continue;

                    markNodes.push_back(node);
                }
            } catch (XQillaException &e) {
                throw Error("Matching node value invalid while marking nodes.");
            }

            for (std::list<xerces::DOMNode *>::iterator nodeIter = markNodes.begin();
                 nodeIter != markNodes.end(); ++nodeIter)
            {
                xerces::DOMNode *node = *nodeIter;
                
                xerces::DOMNamedNodeMap *map = node->getAttributes();
                if (map == 0)
                    continue;
                
                // Create new attribute node.
                xerces::DOMAttr *attr;
                try {
                    attr = document->createAttribute(X(iter->attr.c_str()));
                } catch (xerces::DOMException const &e) {
                    throw Error("Attribute name contains invalid character.");
                }
                attr->setNodeValue(X(iter->value.c_str()));
                
                map->setNamedItem(attr);
                
            }

        }

        // Serialize DOM tree
        AutoRelease<xerces::DOMLSSerializer> serializer(xqillaImplementation->createLSSerializer());
        AutoRelease<xerces::DOMLSOutput> output(xqillaImplementation->createLSOutput());
        xerces::MemBufFormatTarget target;
        output->setByteStream(&target);
        serializer->write(document, output.get());
        
        std::string outData(reinterpret_cast<char const *>(target.getRawBuffer()),
            target.getLen());
            
        return outData;
    }

    std::vector<LexItem> CorpusReader::sentence(std::string const &entry,
        std::string const &query, std::string const &attribute,
        std::string const &defaultValue, CorpusInfo const &corpusInfo) const
    {
        return getSentence(entry, query, attribute, defaultValue, corpusInfo);
    }

    std::string CorpusReader::type() const {
      if (d_type)
        return *d_type;

      EntryIterator iter = entries();
      if (!iter.hasNext()) {
        return "unknown";
      }

      Entry entry = iter.next(*this);
      std::string content = read(entry.name);

      // Prepare the DOM parser.
      xerces::DOMImplementation *xqillaImplementation =
          xerces::DOMImplementationRegistry::getDOMImplementation(X("XPath2 3.0"));
      AutoRelease<xerces::DOMLSParser> parser(xqillaImplementation->createLSParser(
          xerces::DOMImplementationLS::MODE_SYNCHRONOUS, 0));
        
      // Parse the document.
      xerces::MemBufInputSource xmlInput(reinterpret_cast<XMLByte const *>(content.c_str()),
          content.size(), "input");

      xerces::Wrapper4InputSource domInput(&xmlInput, false);

      xerces::DOMDocument *document;
      try {
          document = parser->parse(&domInput);
      } catch (xerces::DOMException const &e) {
          throw Error(std::string("Could not parse XML data: ") + UTF8(e.getMessage()));
      }

      xercesc::TranscodeToStr transcoder(
          document->getDocumentElement()->getTagName(), "UTF-8");

      const_cast<CorpusReader *>(this)->d_type.reset(
          new std::string(fromXmlStr(transcoder.str())));

      return *d_type;
    }
    
    size_t CorpusReader::size() const
    {
        return getSize();
    }
    
    Either<std::string, Empty> CorpusReader::validQuery(QueryDialect d, bool variables, std::string const &query) const
    {
        if (d != XPATH)
            return Either<std::string, Empty>::left("Only XPath2 queries are supported for this corpus.");
        
        // XXX - strip/trim
        if (query.empty())
            return Either<std::string, Empty>::right(Empty());

        // Prepare context
    
        std::shared_ptr<xmlXPathContext> ctx(xmlXPathNewContext(0),
            xmlXPathFreeContext);
        if (!variables)
            ctx->flags = XML_XPATH_NOVAR;
        xmlSetStructuredErrorFunc(ctx.get(), &ignoreStructuredError);
        
        // Compile expression
        std::shared_ptr<xmlXPathCompExpr> r(
            xmlXPathCtxtCompile(ctx.get(), toXmlStr(query.c_str())),
            xmlXPathFreeCompExpr);
        
        if (!r)
            return Either<std::string, Empty>::left("Invalid expression");
                
        return Either<std::string, Empty>::right(Empty());
    }
    
    CorpusReader::EntryIterator CorpusReader::query(QueryDialect d,
        std::string const &q, SortOrder sortOrder) const
    {
        if (d == XPATH)
        {
            auto queries = split_string(q, std::regex("\\+\\|\\+"));
            assert(queries.size() > 0);

            EntryIterator qIter = runXPath(queries[0], sortOrder);
            for (std::vector<std::string>::const_iterator iter = queries.begin() + 1;
                    iter != queries.end(); ++iter)
                qIter = EntryIterator(new FilterIter(*this, qIter, *iter));

            return qIter;
        }
        else if (d == XQUERY)
           return runXQuery(q, sortOrder);
        else
            throw NotImplemented("unknown query language");
    }

    CorpusReader::EntryIterator CorpusReader::queryWithStylesheet(
        QueryDialect d, std::string const &query,
      Stylesheet const &stylesheet,
      std::list<MarkerQuery> const &markerQueries,
      SortOrder sortOrder) const
    {
      return runQueryWithStylesheet(d, query, stylesheet, markerQueries,
          sortOrder);
    }

    CorpusReader::EntryIterator CorpusReader::runQueryWithStylesheet(
        QueryDialect d, std::string const &q,
      Stylesheet const &stylesheet,
      std::list<MarkerQuery> const &markerQueries,
      SortOrder sortOrder) const
    {
        if (d == XQUERY)
            throw NotImplemented(typeid(*this).name(),
                "XQuery functionality");
        
        return EntryIterator(new StylesheetIter(query(XPATH, q, sortOrder),
              stylesheet, markerQueries));
    }

    CorpusReader::EntryIterator CorpusReader::runXPath(std::string const &query,
        SortOrder sortOrder) const
    {        
        //throw NotImplemented(typeid(*this).name(), "XQuery functionality");
        return EntryIterator(new FilterIter(*this, getEntries(sortOrder), query));
    }

    CorpusReader::EntryIterator CorpusReader::runXQuery(std::string const &,
        SortOrder sortOrder) const
    {
        throw NotImplemented(typeid(*this).name(), "XQuery functionality");
    }
    
    bool CorpusReader::MarkerQuery::operator==(MarkerQuery const &other) const
    {
        return other.query == query &&
            other.attr == attr &&
            other.value == value;
    }
}
