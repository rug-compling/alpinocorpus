#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/DbCorpusReader.hh>
#include <AlpinoCorpus/DirectoryCorpusReader.hh>
#include <AlpinoCorpus/Error.hh>
#include <AlpinoCorpus/IndexedCorpusReader.hh>

#include <QString>
#include <typeinfo>

#include <libxml/parser.h>
#include <QDebug>

namespace alpinocorpus {
    CorpusReader *CorpusReader::open(QString const &corpusPath)
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
    
    bool CorpusReader::EntryIterator::operator==(EntryIterator const &other) const
    {
        if (!impl)
            return !other.impl;
        else if (!other.impl)
            return !impl;
        else
            return impl->equals(*other.impl.data());
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


    CorpusReader::EntryIterator CorpusReader::query(CorpusReader::Dialect d,
                                                    QString const &q) const
    {
        switch (d) {
          case XPATH:  return runXPath(q);
          case XQUERY: return runXQuery(q);
          default:     throw NotImplemented("unknown query language");
        }
    }

    CorpusReader::EntryIterator CorpusReader::runXPath(QString const &query) const
    {
        //throw NotImplemented(typeid(*this).name(), "XQuery functionality");
        return EntryIterator(new FilterIter(*this, getBegin(), getEnd(), query));
    }

    CorpusReader::EntryIterator CorpusReader::runXQuery(QString const &) const
    {
        throw NotImplemented(typeid(*this).name(), "XQuery functionality");
    }
    
    CorpusReader::FilterIter::FilterIter(CorpusReader const &corpus,
        EntryIterator itr, EntryIterator end, QString const &query)
    :
        d_corpus(corpus),
        d_itr(itr),
        d_end(end),
        d_query(query.toUtf8())
    {
        next();
    }
    
    QString CorpusReader::FilterIter::current() const
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
        {
            d_buffer.dequeue();
            return;
        }
        
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
    
    void CorpusReader::FilterIter::parseFile(QString const &file)
    {
        QString xml(d_corpus.read(file));
        QByteArray xmlData(xml.toUtf8());
        
        xmlDocPtr doc = xmlParseMemory(xmlData.constData(), xmlData.size());
        
        if (!doc)
        {
            qWarning() << "XPathMapper::run: could not parse XML data: " << *d_itr;
            return;
        }
        
        // Parse XPath query
        xmlXPathContextPtr ctx = xmlXPathNewContext(doc);
        if (!ctx)
        {
            xmlFreeDoc(doc);
            qWarning() << "XPathMapper::run: could not construct XPath context from document: " << *d_itr;
            return;
        }

        xmlXPathObjectPtr xpathObj = xmlXPathEvalExpression(
            reinterpret_cast<xmlChar const *>(d_query.constData()), ctx);
        if (!xpathObj)
        {
            xmlXPathFreeContext(ctx);
            xmlFreeDoc(doc);
            throw Error("XPathMapper::run: could not evaluate XPath expression.");
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
}
