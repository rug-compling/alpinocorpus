#ifndef ALPINO_DIRECTORYCORPUSREADER_HH
#define ALPINO_DIRECTORYCORPUSREADER_HH

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
        QString const &current() const;
        bool equals(IterImpl const *) const;
        void next();
    };

public:
    DirectoryCorpusReader(QString const &directory, bool cache = true);
    EntryIterator begin() const;
    EntryIterator end() const;
    QVector<QString> entries() const;
    QString name() const { return d_directory; }
    QString read(const QString &entry);
    size_t size() const { return d_entries.size(); }

private:
    void open();
    bool readCache();
    bool useCache();
    void writeCache();

    QString d_directory;
    QVector<QString> d_entries;
    bool d_cache;
};

}

#endif  // ALPINO_DIRECTORYCORPUSREADER_HH
