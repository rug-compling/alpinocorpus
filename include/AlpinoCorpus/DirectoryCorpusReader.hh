#ifndef ALPINO_DIRECTORYCORPUSREADER_HH
#define ALPINO_DIRECTORYCORPUSREADER_HH

#include <tr1/memory>

#include <QString>

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
    DirectoryCorpusReader(QString const &directory, bool wantCache = true);
    ~DirectoryCorpusReader();

private:
    virtual EntryIterator getBegin() const;
    virtual EntryIterator getEnd() const;
    virtual QString readEntry(QString const &entry) const;
    virtual size_t getSize() const;

    std::tr1::shared_ptr<DirectoryCorpusReaderPrivate> d_private;
};

}

#endif  // ALPINO_DIRECTORYCORPUSREADER_HH
