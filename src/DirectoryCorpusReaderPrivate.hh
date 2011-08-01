#ifndef ALPINO_DIRECTORYCORPUSREADER_PRIVATE_HH
#define ALPINO_DIRECTORYCORPUSREADER_PRIVATE_HH

#include <QDir>
#include <QString>
#include <QVector>

#include <AlpinoCorpus/CorpusReader.hh>

namespace alpinocorpus {

/**
 * Reader for Alpino treebanks represented as a directory of XML files.
 */
class DirectoryCorpusReaderPrivate : public CorpusReader
{
    typedef QVector<QString> StrVector;

    class DirIter : public IterImpl
    {
        StrVector::const_iterator iter;

      public:
        DirIter(StrVector::const_iterator const &i) : iter(i) { }
        std::string current() const;
        bool equals(IterImpl const &) const;
        void next();
    };

public:
    /**
     * Open directory dir for reading.
     *
     * If cache is true, attempt to read the directory's cache file if present
     * or write one if not present.
     * Failure to read or write the cache file is not signalled to the caller.
     */
    DirectoryCorpusReaderPrivate(std::string const &directory, bool cache = true);
    virtual ~DirectoryCorpusReaderPrivate();

    virtual EntryIterator getBegin() const;
    virtual EntryIterator getEnd() const;
    virtual QString readEntry(std::string const &entry) const;
    virtual size_t getSize() const { return d_entries.size(); }

private:
    QString cachePath() const;
    bool readCache();
    void writeCache();

    QDir d_directory;
    StrVector d_entries;
};

}

#endif  // ALPINO_DIRECTORYCORPUSREADER_PRIVATE_HH
