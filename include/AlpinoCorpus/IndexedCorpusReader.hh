#ifndef ALPINO_INDEXED_CORPUSREADER_HH
#define ALPINO_INDEXED_CORPUSREADER_HH

#include <tr1/memory>

#include <QString>

#include <AlpinoCorpus/CorpusReader.hh>

namespace alpinocorpus
{

class IndexedCorpusReaderPrivate;

class IndexedCorpusReader : public CorpusReader
{
public:
    /**
     * Construct from a single file (data or index); the other file is sought
     * for in the same directory.
     */
    IndexedCorpusReader(std::string const &path);
    /** Construct from data and index file. */
    IndexedCorpusReader(std::string const &dataFilename, std::string const &indexFilename);
    virtual ~IndexedCorpusReader();

private:
    virtual EntryIterator getBegin() const;
    virtual EntryIterator getEnd() const;
    virtual std::string readEntry(std::string const &filename) const;
    virtual size_t getSize() const;

    std::tr1::shared_ptr<IndexedCorpusReaderPrivate> d_private;
};

}

#endif  // ALPINO_INDEXED_CORPUSWRITER_HH
