#ifndef ALPINO_COMPACT_CORPUSWRITER_PRIVATE_HH
#define ALPINO_COMPACT_CORPUSWRITER_PRIVATE_HH

#include <iostream>

#include <tr1/memory>

#include <boost/config.hpp>

#if defined(BOOST_HAS_THREADS)
#include <boost/thread/mutex.hpp>
#endif

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/CorpusWriter.hh>

namespace alpinocorpus
{

typedef std::tr1::shared_ptr<std::ostream> ostreamPtr;

class CompactCorpusWriterPrivate : public CorpusWriter
{
    struct NullStream : std::ostream
    {
        NullStream() : std::ios(0), std::ostream(0) {}
    };

public:
	CompactCorpusWriterPrivate(std::string const &basename);
	CompactCorpusWriterPrivate(ostreamPtr dataStream, ostreamPtr indexStream) :
		d_dataStream(dataStream), d_indexStream(indexStream), d_offset(0) {}
	CompactCorpusWriterPrivate() :
		d_dataStream(new NullStream), d_indexStream(new NullStream), d_offset(0) {}
	CompactCorpusWriterPrivate(CompactCorpusWriterPrivate const &other);
	CompactCorpusWriterPrivate &operator=(CompactCorpusWriterPrivate const &other);
    void writeEntry(std::string const &name, std::string const &data);
    void writeEntry(CorpusReader const &corpus, bool fail_first);

private:
	void copy(CompactCorpusWriterPrivate const &other);
    void writeEntry(std::string const &name, char const *buf, size_t len);
    void writeFailFirst(CorpusReader const &corpus);
    void writeFailSafe(CorpusReader const &corpus);

	ostreamPtr d_dataStream;
	ostreamPtr d_indexStream;
	size_t d_offset;

#if defined(BOOST_HAS_THREADS)
    mutable boost::mutex d_writeMutex;
#endif
};

inline CompactCorpusWriterPrivate::CompactCorpusWriterPrivate(CompactCorpusWriterPrivate const &other)
{
	copy(other);
}

inline CompactCorpusWriterPrivate &CompactCorpusWriterPrivate::operator=(CompactCorpusWriterPrivate const &other)
{
	if (this != &other)
		copy(other);
	
	return *this;
}

}

#endif  // ALPINO_INDEXED_CORPUSWRITER_PRIVATE_HH
