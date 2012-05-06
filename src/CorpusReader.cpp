#include <list>
#include <string>

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/DirectoryCorpusReader.hh>
#include <AlpinoCorpus/Error.hh>
#include <AlpinoCorpus/CompactCorpusReader.hh>
#include <AlpinoCorpus/IterImpl.hh>
#include <AlpinoCorpus/RecursiveCorpusReader.hh>
#include <AlpinoCorpus/RemoteCorpusReader.hh>
#include <config.hh>

#if defined(USE_DBXML)
    #include <AlpinoCorpus/DbCorpusReader.hh>
#endif

#include <typeinfo>

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlerror.h>
#include <libxml/xpath.h>

#include "FilterIter.hh"
#include "StylesheetIter.hh"

namespace {
    void ignoreStructuredError(void *userdata, xmlErrorPtr err)
    {
    }
}

namespace alpinocorpus {
    CorpusReader::EntryIterator::EntryIterator() : d_impl(0)
    { 
    }

    CorpusReader::EntryIterator::EntryIterator(IterImpl *p) :
        d_impl(p)
    { 
    }

    CorpusReader::EntryIterator::EntryIterator(EntryIterator const &other) :
        d_impl(0)
    {
        copy(other);
    }

    CorpusReader::EntryIterator::~EntryIterator()
    {
        delete d_impl;
    }

    CorpusReader::EntryIterator &CorpusReader::EntryIterator::operator=(
        EntryIterator const &other)
    {
        if (this != &other) {
            if (d_impl != 0) {
                delete d_impl;
                d_impl = 0;
            }

            copy(other);
        }

        return *this;
    }

    bool CorpusReader::EntryIterator::operator!=(EntryIterator const &other) const
    {
        return !operator==(other);
    }

    void CorpusReader::EntryIterator::copy(EntryIterator const &other)
    {        
        if (other.d_impl != 0)
            d_impl = other.d_impl->copy();
    }
    
    
    CorpusReader::EntryIterator CorpusReader::begin() const
    {
        return getBegin();
    }

    CorpusReader::EntryIterator CorpusReader::beginWithStylesheet(
        std::string const &stylesheet,
        std::list<MarkerQuery> const &markerQueries) const
    {
        return EntryIterator(new StylesheetIter(getBegin(), getEnd(),
            stylesheet, markerQueries));
    }
    
    std::string CorpusReader::EntryIterator::contents(CorpusReader const &rdr) const
    {
        return d_impl->contents(rdr);
    }

    void CorpusReader::EntryIterator::interrupt()
    {
        // XXX this shouldn't be necessary, we don't do this in other places
        if (d_impl != 0)
            d_impl->interrupt();
    }
    
    CorpusReader::EntryIterator CorpusReader::end() const
    {
        return getEnd();
    }
    
    bool CorpusReader::isValidQuery(QueryDialect d, bool variables, std::string const &q) const
    {
        return validQuery(d, variables, q);
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
        else
            return readEntryMarkQueries(entry, queries);
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
    
    CorpusReader::EntryIterator::value_type CorpusReader::EntryIterator::operator*() const
    {
        return d_impl->current();
    }
    
    bool CorpusReader::EntryIterator::operator==(EntryIterator const &other) const
    {
        if (d_impl == 0)
            return other.d_impl == 0;
        else if (other.d_impl == 0)
            return d_impl == 0;
        else
            return d_impl->equals(*other.d_impl);
    }
    
    CorpusReader::EntryIterator &CorpusReader::EntryIterator::operator++()
    {
        d_impl->next();
        return *this;
    }

    
    CorpusReader::EntryIterator CorpusReader::EntryIterator::operator++(int)
    {
        EntryIterator r(*this);
        operator++();
        return r;
    }


    CorpusReader::EntryIterator CorpusReader::query(QueryDialect d,
        std::string const &q) const
    {
        switch (d) {
          case XPATH:  return runXPath(q);
          case XQUERY: return runXQuery(q);
          default:     throw NotImplemented("unknown query language");
        }
    }

    CorpusReader::EntryIterator CorpusReader::queryWithStylesheet(
        QueryDialect d, std::string const &query,
      std::string const &stylesheet,
      std::list<MarkerQuery> const &markerQueries) const
    {
      return runQueryWithStylesheet(d, query, stylesheet, markerQueries);
    }

    CorpusReader::EntryIterator CorpusReader::runQueryWithStylesheet(
        QueryDialect d, std::string const &query,
      std::string const &stylesheet,
      std::list<MarkerQuery> const &markerQueries) const
    {
        if (d == XQUERY)
            throw NotImplemented(typeid(*this).name(),
                "XQuery functionality");
        
        EntryIterator iter = runXPath(query);

        return EntryIterator(new StylesheetIter(iter, getEnd(), stylesheet,
            markerQueries));
    }

    CorpusReader::EntryIterator CorpusReader::runXPath(std::string const &query) const
    {
        //throw NotImplemented(typeid(*this).name(), "XQuery functionality");
        return EntryIterator(new FilterIter(*this, getBegin(), getEnd(), query));
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
