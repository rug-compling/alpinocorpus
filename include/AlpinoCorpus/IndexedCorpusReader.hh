#ifndef ALPINO_INDEXED_CORPUSREADER_HH
#define ALPINO_INDEXED_CORPUSREADER_HH

#include <QHash>
#include <QMutex>
#include <QSharedPointer>
#include <QString>
#include <QVector>

#include <QDictZip/QDictZipFile.hh>

#include <AlpinoCorpus/CorpusReader.hh>

namespace alpinocorpus
{

class IndexedCorpusReader : public CorpusReader
{
    struct IndexItem
    {
        IndexItem(QString newName, size_t newOffset, size_t newSize)
		    : name(newName), offset(newOffset), size(newSize) {}
	    IndexItem() : name(""), offset(0), size(0) {}

        QString name;
	    size_t offset;
	    size_t size;
    };

    typedef QSharedPointer<IndexItem> IndexItemPtr;
    typedef QHash<QString, IndexItemPtr> IndexMap;
    typedef QSharedPointer<QDictZipFile> QDictZipFilePtr;

    class IndexIter : public IterImpl
    {
        QVector<IndexItemPtr>::const_iterator iter;

      public:
        IndexIter(QVector<IndexItemPtr>::const_iterator const &i) : iter(i) { }
        QString current() const;
        bool equals(IterImpl const *) const;
        void next();
    };

public:
	IndexedCorpusReader() {}
	IndexedCorpusReader(IndexedCorpusReader const &other);

    /** Construct from a single file (data or index); the other file is sought
     * for in the same directory.
     */
    IndexedCorpusReader(QString const &path);
    /** Construct from data and index file. */
    IndexedCorpusReader(QString const &dataFilename, QString const &indexFilename);
	virtual ~IndexedCorpusReader();
	IndexedCorpusReader &operator=(IndexedCorpusReader const &other);
    EntryIterator begin() const;
    EntryIterator end() const;
    QString name() const { return d_canonical; }
    QString read(QString const &filename);
    size_t size() const { return d_indices.size(); }

private:
    void canonicalize(QString const &);
    void construct();
    void construct2();
	void copy(IndexedCorpusReader const &other);
	void destroy();
    void open();
	
    QString d_canonical;
    QDictZipFilePtr d_dataFile;
    QString d_dataFilename;
    QVector<IndexItemPtr> d_indices;
    QString d_indexFilename;
	IndexMap d_namedIndices;

	QMutex d_mutex;
};

inline IndexedCorpusReader::IndexedCorpusReader(IndexedCorpusReader const &other)
{
	copy(other);
}

inline IndexedCorpusReader::~IndexedCorpusReader()
{
	destroy();
}

}

#endif  // ALPINO_INDEXED_CORPUSWRITER_HH
