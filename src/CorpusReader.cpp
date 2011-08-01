#include <string>

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/DbCorpusReader.hh>
#include <AlpinoCorpus/DirectoryCorpusReader.hh>
#include <AlpinoCorpus/Error.hh>
#include <AlpinoCorpus/IndexedCorpusReader.hh>

#include <QString>
#include <typeinfo>

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlerror.h>
#include <libxml/xpath.h>

#include <QDebug>

namespace {
    void ignoreStructuredError(void *userdata, xmlErrorPtr err)
    {
    }
}

namespace alpinocorpus {    

    bool CorpusReader::EntryIterator::operator!=(EntryIterator const &other) const
    {
        return !operator==(other);
    }
    
    
    CorpusReader::EntryIterator CorpusReader::begin() const
    {
        return getBegin();
    }
    
    QString CorpusReader::EntryIterator::contents(CorpusReader const &rdr) const
    {
        return impl->contents(rdr);
    }
    
    CorpusReader::EntryIterator CorpusReader::end() const
    {
        return getEnd();
    }
    
    bool CorpusReader::isValidQuery(QueryDialect d, bool variables, std::string const &q) const
    {
        return validQuery(d, variables, q);
    }
    
    std::string const &CorpusReader::name() const
    {
        return d_name;
    }
        
    CorpusReader *CorpusReader::open(std::string const &corpusPath)
    {
        try {
            return new DirectoryCorpusReader(corpusPath);
        } catch (OpenError const &e) {
        }

        try {
            return new IndexedCorpusReader(corpusPath);
        } catch (OpenError const &e) {
        }

        return new DbCorpusReader(corpusPath);
    }

    QString CorpusReader::read(std::string const &entry) const
    {
        return readEntry(entry);
    }
    
    QString CorpusReader::readMarkQueries(std::string const &entry,
        QList<MarkerQuery> const &queries) const
    {
        return readEntryMarkQueries(entry, queries);
    }
        
    QString CorpusReader::readEntryMarkQueries(std::string const &entry,
        QList<MarkerQuery> const &queries) const
    {
        QByteArray xmlData = readEntry(entry).toUtf8();
        
        xmlDocPtr doc = xmlParseMemory(xmlData.constData(), xmlData.size());
        if (doc == 0)
            throw Error("Could not parse XML data.");

        for (QList<MarkerQuery>::const_iterator iter = queries.begin();
             iter != queries.end(); ++iter)
        {
            QByteArray query(iter->query.toUtf8());
            QByteArray attr(iter->attr.toUtf8());
            QByteArray value(iter->value.toUtf8());
            
            xmlXPathContextPtr xpathCtx = xmlXPathNewContext(doc);
            if (xpathCtx == 0) {
                xmlFreeDoc(doc);
                throw Error("Unable to create new XPath context.");
            }
            
            xmlXPathObjectPtr xpathObj = xmlXPathEvalExpression(
                reinterpret_cast<xmlChar const *>(query.constData()), xpathCtx);
            if (xpathObj == 0) {
                xmlXPathFreeContext(xpathCtx);
                xmlFreeDoc(doc);
                throw Error(std::string("Could not evaluate expression:") + query.constData());
            }
            
            if (xpathObj->nodesetval == 0)
                continue;
            
            xmlNodeSetPtr nodeSet = xpathObj->nodesetval;
            
            QList<xmlNodePtr> nodes;
            for (int i = 0; i < nodeSet->nodeNr; ++i) {
                xmlNodePtr node = nodeSet->nodeTab[i];
                
                if (node->type != XML_ELEMENT_NODE)
                    continue;
                
                nodes << node;
            }
            
            for (QList<xmlNodePtr>::iterator nodeIter = nodes.begin();
                 nodeIter != nodes.end(); ++nodeIter) {
                xmlAttrPtr attrPtr = xmlSetProp(*nodeIter,
                    reinterpret_cast<xmlChar const *>(attr.constData()),
                    reinterpret_cast<xmlChar const *>(value.data()));
                if (attrPtr == 0) {
                    xmlXPathFreeObject(xpathObj);
                    xmlXPathFreeContext(xpathCtx);
                    xmlFreeDoc(doc);
                    throw Error(std::string("Could not set attribute '") + attr.constData() +
                        "' for the expression: " + query.constData());
                    
                }
            }
            
            xmlXPathFreeObject(xpathObj);
            xmlXPathFreeContext(xpathCtx);
        }
        
        // Dump Data
        xmlChar *newData;
        int size;
        xmlDocDumpMemory(doc, &newData, &size);
        QByteArray newXmlData(reinterpret_cast<char const *>(newData), size);
        
        // Cleanup
        xmlFree(newData);
        xmlFreeDoc(doc);
        
        return QString(newXmlData);
    }
    
    void CorpusReader::setName(std::string const &n)
    {
        d_name = n;
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
        return impl->current();
    }
    
    bool CorpusReader::EntryIterator::operator==(EntryIterator const &other) const
    {
        if (!impl)
            return !other.impl;
        else if (!other.impl)
            return !impl;
        else
            return impl->equals(*other.impl.get());
    }
    
    CorpusReader::EntryIterator &CorpusReader::EntryIterator::operator++()
    {
        impl->next();
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
        d_query(query)
    {
        next();
    }
    
    std::string CorpusReader::FilterIter::current() const
    {
        return d_file;
    }
    
    bool CorpusReader::FilterIter::equals(IterImpl const &itr) const
    {
        try {
            // TODO fix me to be more like isEqual instead of hasNext.
            return d_itr == d_end
                && d_buffer.size() == 0;
        } catch (std::bad_cast const &e) {
            return false;
        }
    }
    
    void CorpusReader::FilterIter::next()
    {
        if (!d_buffer.isEmpty())
            d_buffer.dequeue();
        
        while (d_buffer.isEmpty() && d_itr != d_end)
        {
            d_file = *d_itr;
            parseFile(d_file);
            
            ++d_itr;
        }
    }
    
    QString CorpusReader::FilterIter::contents(CorpusReader const &rdr) const
    {
        return d_buffer.isEmpty()
            ? QString()
            : d_buffer.head();
    }
    
    void CorpusReader::FilterIter::parseFile(std::string const &file)
    {
        QString xml(d_corpus.read(file));
        QByteArray xmlData(xml.toUtf8());
        
        xmlDocPtr doc = xmlParseMemory(xmlData.constData(), xmlData.size());
        
        if (!doc)
        {
            qWarning() << "CorpusReader::FilterIter::parseFile: could not parse XML data: " << QString::fromUtf8((*d_itr).c_str());
            return;
        }
        
        // Parse XPath query
        xmlXPathContextPtr ctx = xmlXPathNewContext(doc);
        if (!ctx)
        {
            xmlFreeDoc(doc);
            qWarning() << "CorpusReader::FilterIter::parseFile: could not construct XPath context from document: " << QString::fromUtf8((*d_itr).c_str());
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
                
                QString value(QString::fromUtf8(reinterpret_cast<const char *>(str)));
                
                xmlFree(str);
                
                if (value.trimmed().isEmpty())
                    d_buffer.enqueue(QString());
                else
                    d_buffer.enqueue(value);
            }
        }

        xmlXPathFreeObject(xpathObj);
        xmlXPathFreeContext(ctx);
        xmlFreeDoc(doc);
    }
    
    QString CorpusReader::IterImpl::contents(CorpusReader const &rdr) const
    {
        //return rdr.read(current());
        return QString();
    }
    
}
