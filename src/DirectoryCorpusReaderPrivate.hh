#ifndef ALPINO_DIRECTORYCORPUSREADER_PRIVATE_HH
#define ALPINO_DIRECTORYCORPUSREADER_PRIVATE_HH

#include <string>
#include <vector>

#include <boost/filesystem.hpp>

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/Entry.hh>
#include <AlpinoCorpus/IterImpl.hh>

namespace alpinocorpus {

/**
 * Reader for Alpino treebanks represented as a directory of XML files.
 */
class DirectoryCorpusReaderPrivate : public CorpusReader
{
    typedef std::vector<std::string> StrVector;

public:
    /**
     * Open directory dir for reading.
     */
    DirectoryCorpusReaderPrivate(std::string const &directory);
    virtual ~DirectoryCorpusReaderPrivate();

    virtual EntryIterator getEntries(SortOrder sortOrder) const;
    virtual std::string getName() const;
    virtual std::string readEntry(std::string const &entry) const;
    virtual size_t getSize() const;

private:
    boost::filesystem::path cachePath() const;

    boost::filesystem::path d_directory;
    mutable size_t d_nEntries;
    bool d_entriesRead;
};

}

#endif  // ALPINO_DIRECTORYCORPUSREADER_PRIVATE_HH
