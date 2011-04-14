#ifndef ALPINO_INDEXED_CORPUSREADER_HH
#define ALPINO_INDEXED_CORPUSREADER_HH

#include <QHash>
#include <QMutex>
#include <QSharedPointer>
#include <QString>
#include <vector>

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/QDictZipFile.hh>

namespace alpinocorpus
{

class IndexedCorpusReader : public CorpusReader
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

    typedef QSharedPointer<IndexItem> IndexItemPtr;
    typedef QHash<QString, IndexItemPtr> IndexMap;
    typedef QSharedPointer<QDictZipFile> QDictZipFilePtr;
    typedef std::vector<IndexItemPtr> ItemVector;

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
    IndexedCorpusReader(QString const &path);
    /** Construct from data and index file. */
    IndexedCorpusReader(QString const &dataFilename, QString const &indexFilename);
    virtual ~IndexedCorpusReader() {}

private:
    virtual EntryIterator getBegin() const;
    virtual EntryIterator getEnd() const;
    virtual QString readEntry(QString const &filename) const;
    virtual size_t getSize() const { return d_indices.size(); }

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

#endif  // ALPINO_INDEXED_CORPUSWRITER_HH
