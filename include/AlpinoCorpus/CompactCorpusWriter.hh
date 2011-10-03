#ifndef ALPINO_COMPACT_CORPUSWRITER_HH
#define ALPINO_COMPACT_CORPUSWRITER_HH

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

class CompactCorpusWriter : public CorpusWriter
{
    struct NullStream : std::ostream
    {
        NullStream() : std::ios(0), std::ostream(0) {}
    };

public:
	CompactCorpusWriter(std::string const &basename);
	CompactCorpusWriter(ostreamPtr dataStream, ostreamPtr indexStream) :
		d_dataStream(dataStream), d_indexStream(indexStream), d_offset(0) {}
	CompactCorpusWriter() :
		d_dataStream(new NullStream), d_indexStream(new NullStream), d_offset(0) {}
	CompactCorpusWriter(CompactCorpusWriter const &other);
	CompactCorpusWriter &operator=(CompactCorpusWriter const &other);
protected:
	void writeEntry(std::string const &name, std::string const &data);
	void writeEntry(std::string const &name, char const *buf, size_t len);
    void writeEntry(CorpusReader const &corpus, bool fail_first);
private:
	void copy(CompactCorpusWriter const &other);
    void writeFailFirst(CorpusReader const &corpus);
    void writeFailSafe(CorpusReader const &corpus);

	ostreamPtr d_dataStream;
	ostreamPtr d_indexStream;
	size_t d_offset;

#if defined(BOOST_HAS_THREADS)
    mutable boost::mutex d_writeMutex;
#endif
};

inline CompactCorpusWriter::CompactCorpusWriter(CompactCorpusWriter const &other)
{
	copy(other);
}

inline CompactCorpusWriter &CompactCorpusWriter::operator=(CompactCorpusWriter const &other)
{
	if (this != &other)
		copy(other);
	
	return *this;
}

}

#endif  // ALPINO_INDEXED_CORPUSWRITER_HH
