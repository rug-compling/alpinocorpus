#ifndef ALPINO_COMPACT_CORPUSREADER_HH
#define ALPINO_COMPACT_CORPUSREADER_HH

#include <string>

#include <AlpinoCorpus/CorpusReader.hh>

namespace alpinocorpus
{

class CompactCorpusReaderPrivate;

class CompactCorpusReader : public CorpusReader
{
public:
    /**
     * Construct from a single file (data or index); the other file is sought
     * for in the same directory.
     */
    CompactCorpusReader(std::string const &path);
    /** Construct from data and index file. */
    CompactCorpusReader(std::string const &dataFilename, std::string const &indexFilename);
    virtual ~CompactCorpusReader();

private:
    virtual EntryIterator getEntries(SortOrder sortOrder) const;
    virtual std::string getName() const;
    virtual std::string readEntry(std::string const &filename) const;
    virtual size_t getSize() const;

    CompactCorpusReaderPrivate *d_private;
};

}

#endif  // ALPINO_COMPACT_CORPUSREADER_HH
