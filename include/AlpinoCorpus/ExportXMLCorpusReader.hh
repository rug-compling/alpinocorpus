#ifndef ALPINO_EXPORT_XML_CORPUS_READER_HH
#define ALPINO_EXPORT_XML_CORPUS_READER_HH

#include <string>

#include "CorpusReader.hh"

namespace alpinocorpus
{

class ExportXMLCorpusReaderPrivate;

class ExportXMLCorpusReader : public CorpusReader
{
public:
    /**
     * Open an ExportXML file for reading.
     */
    ExportXMLCorpusReader(std::string const &filename);
    ~ExportXMLCorpusReader();

private:
    virtual EntryIterator getEntries() const;
    virtual std::string getName() const;
    virtual std::string readEntry(std::string const &entry) const;
    virtual size_t getSize() const;

    ExportXMLCorpusReaderPrivate *d_private;
};

}

#endif // ALPINO_EXPORT_XML_CORPUS_READER_HH
