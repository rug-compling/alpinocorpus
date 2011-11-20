#include <string>

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/DirectoryCorpusReader.hh>
#include <AlpinoCorpus/Error.hh>
#include <AlpinoCorpus/CompactCorpusReader.hh>
#include <AlpinoCorpus/RecursiveCorpusReader.hh>
#include <config.hh>

#if defined(USE_DBXML)
    #include <AlpinoCorpus/DbCorpusReader.hh>
#endif

#include <typeinfo>

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlerror.h>
#include <libxml/xpath.h>

namespace {
    void ignoreStructuredError(void *userdata, xmlErrorPtr err)
    {
    }
}

namespace alpinocorpus {    
    CorpusReader::EntryIterator::EntryIterator(EntryIterator const &other)
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
        
    CorpusReader *CorpusReader::open(std::string const &corpusPath)
    {
        try {
            return new DirectoryCorpusReader(corpusPath);
        } catch (OpenError const &) {}

        try {
            return new CompactCorpusReader(corpusPath);
        } catch (OpenError const &) {}

#if defined(USE_DBXML)
        try {
            return new DbCorpusReader(corpusPath);
        } catch (OpenError const &) {}
#endif
        
        throw OpenError(corpusPath);
    }

    CorpusReader *CorpusReader::openRecursive(std::string const &path)
    {
      return new RecursiveCorpusReader(path);
    }

    std::string CorpusReader::read(std::string const &entry) const
    {
        return readEntry(entry);
    }
    
    std::string CorpusReader::readMarkQueries(std::string const &entry,
        std::list<MarkerQuery> const &queries) const
    {
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

    CorpusReader::EntryIterator CorpusReader::runXPath(std::string const &query) const
    {
        //throw NotImplemented(typeid(*this).name(), "XQuery functionality");
        return EntryIterator(new FilterIter(*this, getBegin(), getEnd(), query));
    }

    CorpusReader::EntryIterator CorpusReader::runXQuery(std::string const &) const
    {
        throw NotImplemented(typeid(*this).name(), "XQuery functionality");
    }
    
    CorpusReader::FilterIter::FilterIter(CorpusReader const &corpus,
        EntryIterator itr, EntryIterator end, std::string const &query)
    :
        d_corpus(corpus),
        d_itr(itr),
        d_end(end),
        d_query(query),
        d_initialState(true)
    {
        //next();
    }
    
    CorpusReader::IterImpl *CorpusReader::FilterIter::copy() const
    {
        // FilterIter is no pointer members.
        return new FilterIter(*this);
    }

    std::string CorpusReader::FilterIter::current() const
    {
        return d_file;
    }
    
    bool CorpusReader::FilterIter::equals(IterImpl const &itr) const
    {
        if (d_initialState) {
          d_initialState = false;
          FilterIter *self = const_cast<FilterIter *>(this);
          self->next();
        }

        try {
            // TODO fix me to be more like isEqual instead of hasNext.
            return d_itr == d_end
                && d_buffer.size() == 0;
        } catch (std::bad_cast const &e) {
            return false;
        }
    }

    void CorpusReader::FilterIter::interrupt()
    {
      d_interrupted = true;
    }
    
    void CorpusReader::FilterIter::next()
    {
        if (!d_buffer.empty())
            d_buffer.pop();
       
        d_interrupted = false;

        while (d_buffer.empty() && d_itr != d_end)
        {
            if (d_interrupted)
              throw IterationInterrupted();

            d_file = *d_itr;
            parseFile(d_file);
            
            ++d_itr;
        }
    }
    
    std::string CorpusReader::FilterIter::contents(CorpusReader const &rdr) const
    {
        return d_buffer.empty()
        ?   std::string() // XXX - should be a null string???
            : d_buffer.front();
    }
    
    void CorpusReader::FilterIter::parseFile(std::string const &file)
    {
        std::string xml(d_corpus.read(file));

        xmlDocPtr doc = xmlParseMemory(xml.c_str(), xml.size());

        if (!doc)
        {
            //qWarning() << "CorpusReader::FilterIter::parseFile: could not parse XML data: " << QString::fromUtf8((*d_itr).c_str());
            return;
        }
        
        // Parse XPath query
        xmlXPathContextPtr ctx = xmlXPathNewContext(doc);
        if (!ctx)
        {
            xmlFreeDoc(doc);
            //qWarning() << "CorpusReader::FilterIter::parseFile: could not construct XPath context from document: " << QString::fromUtf8((*d_itr).c_str());
            return;
        }
        
        xmlXPathObjectPtr xpathObj = xmlXPathEvalExpression(
            reinterpret_cast<xmlChar const *>(d_query.c_str()), ctx);
        if (!xpathObj)
        {
            xmlXPathFreeContext(ctx);
            xmlFreeDoc(doc);
            throw Error("CorpusReader::FilterIter::parseFile: could not evaluate XPath expression.");
        }

        if (xpathObj->nodesetval && xpathObj->nodesetval->nodeNr > 0)
        {
            for (int i = 0; i < xpathObj->nodesetval->nodeNr; ++i)
            {
                xmlChar *str = xmlNodeListGetString(doc, xpathObj->nodesetval->nodeTab[i]->children, 1);
                
                std::string value;
                if (str != 0) // XXX - is this correct?
                    value = reinterpret_cast<const char *>(str);
                
                xmlFree(str);
                
                if (value.empty()) // XXX - trim!
                    d_buffer.push(std::string());
                else
                    d_buffer.push(value);
            }
        }

        xmlXPathFreeObject(xpathObj);
        xmlXPathFreeContext(ctx);
        xmlFreeDoc(doc);
    }
    
    std::string CorpusReader::IterImpl::contents(CorpusReader const &rdr) const
    {
        //return rdr.read(current());
        return std::string(); // XXX - should be a null string
    }

    void CorpusReader::IterImpl::interrupt()
    {
        // XXX no default behavior implemented
    }
}
