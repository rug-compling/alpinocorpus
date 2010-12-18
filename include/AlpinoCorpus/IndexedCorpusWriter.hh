#ifndef ALPINO_INDEXED_CORPUSWRITER_HH
#define ALPINO_INDEXED_CORPUSWRITER_HH

#include <QMutex>
#include <QSharedPointer>
#include <iostream>

namespace alpinocorpus
{

typedef QSharedPointer<std::ostream> ostreamPtr;

class IndexedCorpusWriter
{
    struct NullStream : std::ostream
    {
        NullStream() : std::ios(0), std::ostream(0) {}
    };

public:
	IndexedCorpusWriter(ostreamPtr dataStream, ostreamPtr indexStream) :
		d_dataStream(dataStream), d_indexStream(indexStream), d_offset(0) {}
	IndexedCorpusWriter() :
		d_dataStream(new NullStream), d_indexStream(new NullStream), d_offset(0) {}
	IndexedCorpusWriter(IndexedCorpusWriter const &other);
	IndexedCorpusWriter &operator=(IndexedCorpusWriter const &other);
	void write(std::string const &name, std::string const &data);
	void write(std::string const &name, char const *buf, size_t len);
private:
	void copy(IndexedCorpusWriter const &other);
	ostreamPtr d_dataStream;
	ostreamPtr d_indexStream;
	size_t d_offset;

	QMutex d_mutex;
};

inline IndexedCorpusWriter::IndexedCorpusWriter(IndexedCorpusWriter const &other)
{
	copy(other);
}

inline IndexedCorpusWriter &IndexedCorpusWriter::operator=(IndexedCorpusWriter const &other)
{
	if (this != &other)
		copy(other);
	
	return *this;
}

}

#endif  // ALPINO_INDEXED_CORPUSWRITER_HH
