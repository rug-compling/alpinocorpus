#ifndef ALPINO_DIRECTORYCORPUSREADER_HH
#define ALPINO_DIRECTORYCORPUSREADER_HH

#include <string>

#include <AlpinoCorpus/CorpusReader.hh>

namespace alpinocorpus {

class DirectoryCorpusReaderPrivate;

/**
 * Reader for Alpino treebanks represented as a directory of XML files.
 */
class DirectoryCorpusReader : public CorpusReader
{
public:
    /**
     * Open directory dir for reading.
     *
     * If cache is true, attempt to read the directory's cache file if present
     * or write one if not present.
     * Failure to read or write the cache file is not signalled to the caller.
     */
    DirectoryCorpusReader(std::string const &directory);
    ~DirectoryCorpusReader();

private:
    virtual EntryIterator getEntries(SortOrder sortOrder) const;
    virtual std::string getName() const;
    virtual std::string readEntry(std::string const &entry) const;
    virtual size_t getSize() const;

    DirectoryCorpusReaderPrivate *d_private;
};

}

#endif  // ALPINO_DIRECTORYCORPUSREADER_HH
