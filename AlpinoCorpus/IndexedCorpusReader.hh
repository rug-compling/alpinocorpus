#ifndef INDEXED_CORPUSREADER_HH
#define INDEXED_CORPUSREADER_HH

#include <QHash>
#include <QMutex>
#include <QSharedPointer>
#include <QString>
#include <QVector>

#include "CorpusReader.hh"
#include "DictZipFile.hh"

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

typedef QHash<QString, IndexItemPtr> IndexMap;
typedef QVector<IndexItemPtr> IndexPtrVec;

typedef QSharedPointer<DictZipFile> DictZipFilePtr;

class IndexedCorpusReader : public CorpusReader
{
public:
	IndexedCorpusReader() {}
	IndexedCorpusReader(IndexedCorpusReader const &other);
    IndexedCorpusReader(QString const &dataFilename, QString const &indexFilename);
	virtual ~IndexedCorpusReader();
	IndexedCorpusReader &operator=(IndexedCorpusReader const &other);
    QVector<QString> entries() const;
	IndexPtrVec const &indices() const;
    QString read(QString const &filename);
private:
	void copy(IndexedCorpusReader const &other);
	void destroy();
	
    DictZipFilePtr d_dataFile;
	IndexPtrVec d_indices;
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

#endif // INDEXED_CORPUSWRITER_HH
