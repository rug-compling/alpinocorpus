#ifndef ALPINO_DIRECTORYCORPUSREADER_HH
#define ALPINO_DIRECTORYCORPUSREADER_HH

#include <QDir>
#include <QString>
#include <QVector>

#include <AlpinoCorpus/CorpusReader.hh>

namespace alpinocorpus {

/**
 * Reader for Alpino treebanks represented as a directory of XML files.
 */
class DirectoryCorpusReader : public CorpusReader
{
    class DirIter : public IterImpl
    {
        QVector<QString>::const_iterator iter;

      public:
        DirIter(QVector<QString>::const_iterator const &i) : iter(i) { }
        QString current() const;
        bool equals(IterImpl const *) const;
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
    DirectoryCorpusReader(QString const &directory, bool cache = true);

    //QString name() const { return d_directory; }

private:
    virtual EntryIterator getBegin() const;
    virtual EntryIterator getEnd() const;
    virtual QString readEntry(const QString &entry) const;
    virtual size_t getSize() const { return d_entries.size(); }

    void cacheFile(QFile &) const;
    bool readCache();
    void writeCache();

    QDir d_directory;
    QVector<QString> d_entries;
};

}

#endif  // ALPINO_DIRECTORYCORPUSREADER_HH
