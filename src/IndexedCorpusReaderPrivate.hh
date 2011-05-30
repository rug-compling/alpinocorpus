#ifndef ALPINO_INDEXED_CORPUSREADER_PRIVATE_HH
#define ALPINO_INDEXED_CORPUSREADER_PRIVATE_HH

#include <QHash>
#include <QMutex>
#include <QSharedPointer>
#include <QString>
#include <QVector>

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/QDictZipFile.hh>

namespace alpinocorpus
{

struct IndexItem
{
    IndexItem(QString const &newName, size_t newOffset, size_t newSize)
     : name(newName), offset(newOffset), size(newSize) {}
    IndexItem() : name(""), offset(0), size(0) {}

    QString name;
    size_t offset;
    size_t size;
};

class IndexedCorpusReaderPrivate : public CorpusReader
{
    typedef QSharedPointer<IndexItem> IndexItemPtr;
    typedef QHash<QString, IndexItemPtr> IndexMap;
    typedef QSharedPointer<QDictZipFile> QDictZipFilePtr;
    typedef QVector<IndexItemPtr> ItemVector;

    class IndexIter : public IterImpl
    {
        ItemVector::const_iterator iter;

      public:
        IndexIter(ItemVector::const_iterator const &i) : iter(i) { }
        QString current() const;
        bool equals(IterImpl const &) const;
        void next();
    };

public:
    /**
     * Construct from a single file (data or index); the other file is sought
     * for in the same directory.
     */
    IndexedCorpusReaderPrivate(QString const &path);
    /** Construct from data and index file. */
    IndexedCorpusReaderPrivate(QString const &dataFilename, QString const &indexFilename);
    virtual ~IndexedCorpusReaderPrivate() {}

    virtual EntryIterator getBegin() const;
    virtual EntryIterator getEnd() const;
    virtual QString readEntry(QString const &filename) const;
    virtual size_t getSize() const;

private:
    static void canonicalize(QString &);
    void construct(QString const &);
    void construct(QString const &, QString const &, QString const &);
    void open(QString const &, QString const &);
	
    QDictZipFilePtr d_dataFile;
    QVector<IndexItemPtr> d_indices;
	IndexMap d_namedIndices;

	QMutex d_mutex;
};

}

#endif  // ALPINO_INDEXED_CORPUSREADER_PRIVATE_HH
