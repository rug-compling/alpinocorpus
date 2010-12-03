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
typedef QVector<IndexItemPtr> IndexPtrVec;

class IndexedCorpusReader : public CorpusReader
{
    typedef QHash<QString, IndexItemPtr> IndexMap;
    typedef QSharedPointer<QDictZipFile> QDictZipFilePtr;

public:
	IndexedCorpusReader() {}
	IndexedCorpusReader(IndexedCorpusReader const &other);
    IndexedCorpusReader(QString const &dataFilename, QString const &indexFilename);
	virtual ~IndexedCorpusReader();
	IndexedCorpusReader &operator=(IndexedCorpusReader const &other);
    QVector<QString> entries() const;
	IndexPtrVec const &indices() const;
    bool open();
    QString read(QString const &filename);
private:
	void copy(IndexedCorpusReader const &other);
	void destroy();
	
    QDictZipFilePtr d_dataFile;
    QString d_dataFilename;
	IndexPtrVec d_indices;
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

inline IndexPtrVec const &IndexedCorpusReader::indices() const
{
	return d_indices;
}

}

#endif  // ALPINO_INDEXED_CORPUSWRITER_HH
