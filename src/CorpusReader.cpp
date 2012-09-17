#include <algorithm>
#include <cassert>
#include <cstring>
#include <list>
#include <string>
#include <tr1/unordered_map>
#include <vector>

#include <boost/algorithm/string/regex.hpp>
#include <boost/regex.hpp>

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/Error.hh>
#include <AlpinoCorpus/IterImpl.hh>
#include <AlpinoCorpus/RecursiveCorpusReader.hh>

#include <typeinfo>

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlerror.h>
#include <libxml/xpath.h>

#include "FilterIter.hh"
#include "StylesheetIter.hh"
#include "util/parseString.hh"

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


    std::vector<alpinocorpus::LexItem> collectLexicals(xmlDoc *doc,
        std::tr1::unordered_map<xmlNode *, std::set<size_t> > const &matchDepth)
    {
        std::vector<alpinocorpus::LexItem> items;

        xmlXPathContextPtr xpCtx = xmlXPathNewContext(doc);
        if (xpCtx == 0)
        {
            //qDebug() << "Could not make XPath context.";
            return items;
        }

        xmlXPathObjectPtr xpObj = xmlXPathEvalExpression(
            toXmlStr("//node[@word]"), xpCtx);
        if (xpObj == 0) {
            //qDebug() << "Could not make XPath expression to select active nodes.";
            xmlXPathFreeContext(xpCtx);
            return items;
        }

        xmlNodeSet *nodeSet = xpObj->nodesetval;
        if (nodeSet != 0)
        {
            for (int i = 0; i < nodeSet->nodeNr; ++i)
            {
                xmlNode *node = nodeSet->nodeTab[i];

                if (node->type == XML_ELEMENT_NODE)
                {
                    xmlAttrPtr wordAttr = xmlHasProp(node, toXmlStr("word"));
                    xmlChar *word = xmlNodeGetContent(wordAttr->children);

                    xmlAttrPtr beginAttr = xmlHasProp(node, toXmlStr("begin"));
                    size_t begin = 0;
                    if (beginAttr)
                    {
                        xmlChar *beginStr = xmlNodeGetContent(beginAttr->children);
                        try {
                            begin = alpinocorpus::util::parseString<size_t>(fromXmlStr(beginStr));
                        } catch (std::invalid_argument &e) {
                            //qDebug() << e.what();
                        }
                    }

                    alpinocorpus::LexItem item = {fromXmlStr(word), begin, std::set<size_t>() };

                    std::tr1::unordered_map<xmlNode *, std::set<size_t> >::const_iterator matchIter =
                        matchDepth.find(node);
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
    void markLexicals(xmlNode *node, std::tr1::unordered_map<xmlNode *,
        std::set<size_t> > *matchDepth, size_t matchId)
    {
        // Don't attempt to handle a node that we can't.
        if (node->type != XML_ELEMENT_NODE ||
              std::strcmp(fromXmlStr(node->name), "node") != 0)
            return;

        xmlAttrPtr wordProp = xmlHasProp(node, toXmlStr("word"));
        if (wordProp != 0)
            (*matchDepth)[node].insert(matchId);
        else // Attempt to recurse...
        {
            for (xmlNodePtr child = node->children;
                child != NULL; child = child->next)
              markLexicals(child, matchDepth, matchId);
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
    
    
    CorpusReader::EntryIterator CorpusReader::entries() const
    {
        return getEntries();
    }

    CorpusReader::EntryIterator CorpusReader::entriesWithStylesheet(
        std::string const &stylesheet,
        std::list<MarkerQuery> const &markerQueries) const
    {
        return EntryIterator(new StylesheetIter(getEntries(),
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
        std::string const &query) const
    {
        std::vector<std::string> queries;
        boost::split_regex(queries, query, boost::regex("\\+\\|\\+"));
        assert(queries.size() > 0);

        // Discard pre-filters
        std::string q = queries.back();

        std::list<MarkerQuery> markers;

        if (!q.empty())
        {
            MarkerQuery marker(q, "active", "1");
            markers.push_back(marker);
        }
        std::string xmlData(read(entry, markers));

        xmlDocPtr doc;
        doc = xmlReadMemory(xmlData.c_str(), xmlData.size(), NULL, NULL, 0);
        if (doc == NULL)
            return std::vector<LexItem>();

        // We get the sentence node, we should process its children.
        xmlNode *sentenceNode = xmlDocGetRootElement(doc);
        if (sentenceNode == NULL) {
            xmlFreeDoc(doc);
            return std::vector<LexItem>();
        }

        xmlXPathContextPtr xpCtx = xmlXPathNewContext(doc);
        if (xpCtx == 0)
        {
            //qDebug() << "Could not make XPath context.";
            xmlFreeDoc(doc);
            return std::vector<LexItem>();
        }

        xmlXPathObjectPtr xpObj = xmlXPathEvalExpression(
            toXmlStr("//node[@active='1']"), xpCtx);
        if (xpObj == 0) {
            //qDebug() << "Could not make XPath expression to select active nodes.";
            xmlXPathFreeContext(xpCtx);
            xmlFreeDoc(doc);
            return std::vector<LexItem>();
        }

        // Do we have matches?
        xmlNodeSet *nodeSet = xpObj->nodesetval;
        std::tr1::unordered_map<xmlNode *, std::set<size_t> > matchDepth;
        if (nodeSet != 0)
        {
            for (int i = 0; i < nodeSet->nodeNr; ++i)
              if (nodeSet->nodeTab[i]->type == XML_ELEMENT_NODE)
                  markLexicals(nodeSet->nodeTab[i], &matchDepth, i);
        }

        std::vector<LexItem> items = collectLexicals(doc, matchDepth);

        xmlXPathFreeObject(xpObj);
        xmlXPathFreeContext(xpCtx);
        xmlFreeDoc(doc);

        return items;
    }

    
    bool CorpusReader::isValidQuery(QueryDialect d, bool variables, std::string const &q) const
    {
        std::vector<std::string> queries;
        boost::split_regex(queries, q, boost::regex("\\+\\|\\+"));
        assert(queries.size() > 0);

        for (std::vector<std::string>::const_iterator iter = queries.begin();
                iter != queries.end(); ++iter)
            if (!validQuery(d, variables, *iter))
                return false;

        return true;
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
            std::vector<std::string> queries;
            boost::split_regex(queries, iter->query, boost::regex("\\+\\|\\+"));
            assert(queries.size() > 0);
            iter->query = queries.back();
        }

        return readEntryMarkQueries(entry, effectiveQueries);
    }
        
    std::string CorpusReader::readEntryMarkQueries(std::string const &entry,
        std::list<MarkerQuery> const &queries) const
    {
        std::string xmlData = readEntry(entry);
        
        xmlDocPtr doc = xmlParseMemory(xmlData.c_str(), xmlData.size());
        if (doc == 0)
            throw Error("Could not parse XML data.");

        for (std::list<MarkerQuery>::const_iterator iter = queries.begin();
             iter != queries.end(); ++iter)
        {            
            xmlXPathContextPtr xpathCtx = xmlXPathNewContext(doc);
            if (xpathCtx == 0) {
                xmlFreeDoc(doc);
                throw Error("Unable to create new XPath context.");
            }
            
            xmlXPathObjectPtr xpathObj = xmlXPathEvalExpression(
                reinterpret_cast<xmlChar const *>(iter->query.c_str()), xpathCtx);
            if (xpathObj == 0) {
                xmlXPathFreeContext(xpathCtx);
                xmlFreeDoc(doc);
                throw Error(std::string("Could not evaluate expression:") + iter->query);
            }
            
            if (xpathObj->nodesetval == 0)
                continue;
            
            xmlNodeSetPtr nodeSet = xpathObj->nodesetval;
            
            std::list<xmlNodePtr> nodes;
            for (int i = 0; i < nodeSet->nodeNr; ++i) {
                xmlNodePtr node = nodeSet->nodeTab[i];
                
                if (node->type != XML_ELEMENT_NODE)
                    continue;
                
                nodes.push_back(node);
            }
            
            for (std::list<xmlNodePtr>::iterator nodeIter = nodes.begin();
                 nodeIter != nodes.end(); ++nodeIter) {
                xmlAttrPtr attrPtr = xmlSetProp(*nodeIter,
                    reinterpret_cast<xmlChar const *>(iter->attr.c_str()),
                    reinterpret_cast<xmlChar const *>(iter->value.c_str()));
                if (attrPtr == 0) {
                    xmlXPathFreeObject(xpathObj);
                    xmlXPathFreeContext(xpathCtx);
                    xmlFreeDoc(doc);
                    throw Error(std::string("Could not set attribute '") + iter->attr +
                        "' for the expression: " + iter->query);
                    
                }
            }
            
            xmlXPathFreeObject(xpathObj);
            xmlXPathFreeContext(xpathCtx);
        }
        
        // Dump Data
        xmlChar *newData;
        int size;
        xmlDocDumpMemory(doc, &newData, &size);
        std::string newXmlData(reinterpret_cast<char const *>(newData), size);
        
        // Cleanup
        xmlFree(newData);
        xmlFreeDoc(doc);
        
        return newXmlData;
    }

    std::vector<LexItem> CorpusReader::sentence(std::string const &entry,
        std::string const &query) const
    {
        return getSentence(entry, query);
    }
    
    size_t CorpusReader::size() const
    {
        return getSize();
    }
    
    bool CorpusReader::validQuery(QueryDialect d, bool variables, std::string const &query) const
    {
        if (d != XPATH)
            return false;
        
        // XXX - strip/trim
        if (query.empty())
            return true;

        // Prepare context
        xmlXPathContextPtr ctx = xmlXPathNewContext(0);
        if (!variables)
            ctx->flags = XML_XPATH_NOVAR;
        xmlSetStructuredErrorFunc(ctx, &ignoreStructuredError);
        
        // Compile expression
        xmlXPathCompExprPtr r = xmlXPathCtxtCompile(ctx,
                                                    reinterpret_cast<xmlChar const *>(query.c_str()));
        
        if (!r) {
            xmlXPathFreeContext(ctx);
            return false;
        }
        
        xmlXPathFreeCompExpr(r);
        xmlXPathFreeContext(ctx);
        
        return true;
    }
    
    CorpusReader::EntryIterator CorpusReader::query(QueryDialect d,
        std::string const &q) const
    {
        /*
        switch (d) {
          case XPATH:  return runXPath(q);
          case XQUERY: return runXQuery(q);
          default:     throw NotImplemented("unknown query language");
        }
        */

        std::vector<std::string> queries;
        boost::split_regex(queries, q, boost::regex("\\+\\|\\+"));
        assert(queries.size() > 0);

        EntryIterator qIter = runXPath(queries[0]);
        for (std::vector<std::string>::const_iterator iter = queries.begin() + 1;
                iter != queries.end(); ++iter)
            qIter = EntryIterator(new FilterIter(*this, qIter, *iter));

        return qIter;
    }

    CorpusReader::EntryIterator CorpusReader::queryWithStylesheet(
        QueryDialect d, std::string const &query,
      std::string const &stylesheet,
      std::list<MarkerQuery> const &markerQueries) const
    {
      return runQueryWithStylesheet(d, query, stylesheet, markerQueries);
    }

    CorpusReader::EntryIterator CorpusReader::runQueryWithStylesheet(
        QueryDialect d, std::string const &q,
      std::string const &stylesheet,
      std::list<MarkerQuery> const &markerQueries) const
    {
        if (d == XQUERY)
            throw NotImplemented(typeid(*this).name(),
                "XQuery functionality");
        
        return EntryIterator(new StylesheetIter(query(XPATH, q), stylesheet,
            markerQueries));
    }

    CorpusReader::EntryIterator CorpusReader::runXPath(std::string const &query) const
    {        
        //throw NotImplemented(typeid(*this).name(), "XQuery functionality");
        return EntryIterator(new FilterIter(*this, getEntries(), query));
    }

    CorpusReader::EntryIterator CorpusReader::runXQuery(std::string const &) const
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
