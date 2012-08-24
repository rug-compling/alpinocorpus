#include <string>
#include <typeinfo>

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/Entry.hh>
#include <AlpinoCorpus/Error.hh>

#include "FilterIter.hh"

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlerror.h>
#include <libxml/xpath.h>

namespace alpinocorpus {

    FilterIter::FilterIter(CorpusReader const &corpus,
        CorpusReader::EntryIterator itr,
        std::string const &query)
    :
        d_corpus(corpus),
        d_itr(itr),
        d_query(query),
        d_initialState(true)
    {
        //next();
    }
    
    IterImpl *FilterIter::copy() const
    {
        // FilterIter is no pointer members.
        return new FilterIter(*this);
    }

    bool FilterIter::hasNext()
    {
        d_interrupted = false;

        while (d_buffer.empty() && d_itr.hasNext())
        {
            Entry e = d_itr.next(d_corpus);

            if (d_interrupted)
              throw IterationInterrupted();

            d_file = e.name;
            parseFile(d_file);
        }

        return !d_buffer.empty();
    }

    bool FilterIter::hasProgress()
    {
      return d_itr.hasProgress();
    }

    void FilterIter::interrupt()
    {
      d_interrupted = true;
    }

    
    Entry FilterIter::next(CorpusReader const &rdr)
    {
        if (!d_buffer.empty())
            d_buffer.pop();

        Entry e = {d_file, d_buffer.empty() ? std::string() : d_buffer.front()};

        return e;
    }
    
    void FilterIter::parseFile(std::string const &file)
    {
        std::string xml(d_corpus.read(file));

        xmlDocPtr doc = xmlParseMemory(xml.c_str(), xml.size());

        if (!doc)
        {
            //qWarning() << "FilterIter::parseFile: could not parse XML data: " << QString::fromUtf8((*d_itr).c_str());
            return;
        }
        
        // Parse XPath query
        xmlXPathContextPtr ctx = xmlXPathNewContext(doc);
        if (!ctx)
        {
            xmlFreeDoc(doc);
            //qWarning() << "FilterIter::parseFile: could not construct XPath context from document: " << QString::fromUtf8((*d_itr).c_str());
            return;
        }
        
        xmlXPathObjectPtr xpathObj = xmlXPathEvalExpression(
            reinterpret_cast<xmlChar const *>(d_query.c_str()), ctx);
        if (!xpathObj)
        {
            xmlXPathFreeContext(ctx);
            xmlFreeDoc(doc);
            throw Error("FilterIter::parseFile: could not evaluate XPath expression.");
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

    double FilterIter::progress()
    {
        return d_itr.progress();
    }

}
