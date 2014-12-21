#include <set>
#include <sstream>
#include <string>

#include <iostream>

#include <boost/shared_ptr.hpp>

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlerror.h>
#include <libxml/xpath.h>

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/Entry.hh>
#include <AlpinoCorpus/Error.hh>
#include <AlpinoCorpus/ExportXMLCorpusReader.hh>
#include <AlpinoCorpus/IterImpl.hh>

namespace alpinocorpus
{

class ExportXMLCorpusReaderPrivate : public CorpusReader
{
public:
    /**
     * Open an ExportXML file for reading.
     */
    ExportXMLCorpusReaderPrivate(std::string const &filename);
    ~ExportXMLCorpusReaderPrivate();

private:
    virtual EntryIterator getEntries() const;
    virtual std::string getName() const;
    virtual std::string readEntry(std::string const &entry) const;
    virtual size_t getSize() const;

    class ExportIter : public IterImpl
    {
    public:
        ExportIter(boost::shared_ptr<xmlDoc> doc);
        IterImpl *copy() const;
        bool hasNext();
        Entry next(CorpusReader const &rdr);
    private:
        bool isValid();

        std::set<std::string> d_entries;
        std::set<std::string>::const_iterator d_iter;
    };

    boost::shared_ptr<xmlDoc> doc;
};

namespace {
xmlChar const *toXmlStr(char const *str)
{
    return reinterpret_cast<xmlChar const *>(str);
}

char const *fromXmlStr(xmlChar const *str)
{
    return reinterpret_cast<char const *>(str);
}

}

ExportXMLCorpusReader::ExportXMLCorpusReader(std::string const &filename) :
    d_private(new ExportXMLCorpusReaderPrivate(filename))
{}

ExportXMLCorpusReader::~ExportXMLCorpusReader()
{
    delete d_private;
}

CorpusReader::EntryIterator ExportXMLCorpusReader::getEntries() const
{
  return d_private->entries();
}

std::string ExportXMLCorpusReader::getName() const
{
    return d_private->name();
}


std::string ExportXMLCorpusReader::readEntry(std::string const &entry) const
{
    return d_private->read(entry);
}

size_t ExportXMLCorpusReader::getSize() const
{
    return d_private->size();
}

ExportXMLCorpusReaderPrivate::ExportXMLCorpusReaderPrivate(std::string const &filename)
{
    doc.reset(
        xmlReadFile(filename.c_str(), NULL, 0),
        xmlFreeDoc);
    
    if (!doc)
        throw OpenError(std::string("Could not parse ExportXML file"));
}

ExportXMLCorpusReaderPrivate::~ExportXMLCorpusReaderPrivate()
{
}

CorpusReader::EntryIterator ExportXMLCorpusReaderPrivate::getEntries() const
{
    return EntryIterator(new ExportIter(doc));
}

std::string ExportXMLCorpusReaderPrivate::getName() const
{
    return doc->name;
}


std::string ExportXMLCorpusReaderPrivate::readEntry(std::string const &entry) const
{
    boost::shared_ptr<xmlXPathContext> xpCtx(
        xmlXPathNewContext(doc.get()), xmlXPathFreeContext);
    if (!xpCtx)
      throw Error(std::string("Could not construct XPath context"));

    std::ostringstream queryStream;
    queryStream << "//sentence[@xml:id='"
                << entry
                << "']";
    std::string query = queryStream.str();

//std::cerr << query << std::endl;

    boost::shared_ptr<xmlXPathObject> xpObj(
        xmlXPathEvalExpression(toXmlStr(query.c_str()), xpCtx.get()),
        xmlXPathFreeObject);
    if (!xpObj)
      throw Error(std::string("Could not evaluate sentence query"));

    xmlNodeSetPtr nodeSet = xpObj->nodesetval;
    if (nodeSet == 0)
        throw Error(std::string("Could not find entry"));

    if (nodeSet->nodeNr != 1)
        throw Error(std::string("Could not find entry"));

    xmlNodePtr node = nodeSet->nodeTab[0];
    boost::shared_ptr<xmlBuffer> buffer(xmlBufferCreate(), xmlBufferFree);
    int r = xmlNodeDump(buffer.get(), doc.get(), node, 0, 1);
    if (r == -1)
      throw Error(std::string("Could not save XML fragment"));

    return fromXmlStr(buffer->content);
}

size_t ExportXMLCorpusReaderPrivate::getSize() const
{
}

ExportXMLCorpusReaderPrivate::ExportIter::ExportIter(boost::shared_ptr<xmlDoc> doc)
{
    boost::shared_ptr<xmlXPathContext> xpCtx(
        xmlXPathNewContext(doc.get()), xmlXPathFreeContext);
    if (!xpCtx)
      throw Error(std::string("Could not construct XPath context"));

    boost::shared_ptr<xmlXPathObject> xpObj(
        xmlXPathEvalExpression(toXmlStr("//sentence"), xpCtx.get()),
        xmlXPathFreeObject);
    if (!xpObj)
      throw Error(std::string("Could not evaluate sentence query"));

    xmlNodeSetPtr nodeSet = xpObj->nodesetval;
    if (nodeSet == 0)
      throw Error(std::string("Could not evaluate sentence query"));

    for (int i = 0; i < nodeSet->nodeNr; ++i)
    {
        xmlNodePtr node = nodeSet->nodeTab[i];

        if (node->type == XML_ELEMENT_NODE)
        {
            boost::shared_ptr<xmlChar> ident(
                xmlGetProp(node, toXmlStr("id")), xmlFree);
            if (!ident)
                continue; // XXX: warn?
            std::cerr << fromXmlStr(ident.get()) << std::endl;
            d_entries.insert(fromXmlStr(ident.get()));
        }
    }

    d_iter = d_entries.begin();
}

IterImpl *ExportXMLCorpusReaderPrivate::ExportIter::copy() const
{
    // No pointer members
    ExportIter *exportIter = new ExportIter(*this);
    exportIter->d_iter = exportIter->d_entries.begin();
    return exportIter;
}

bool ExportXMLCorpusReaderPrivate::ExportIter::hasNext()
{
    return d_iter != d_entries.end();
}

Entry ExportXMLCorpusReaderPrivate::ExportIter::next(CorpusReader const &rdr)
{
    if (d_iter == d_entries.end())
        throw Error(std::string("Calling next() on invalid iterator."));

    Entry entry = {*d_iter, ""};
    ++d_iter;
    return entry;
}


}
