#include <fstream>
#include <string>

#include <boost/config.hpp>

#if defined(BOOST_HAS_THREADS)
#include <boost/thread/mutex.hpp>
#endif

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/Error.hh>
#include <AlpinoCorpus/CompactCorpusWriter.hh>
#include <util/base64.hh>

#include "DzOstream.hh"

using namespace std;

namespace alpinocorpus {


CompactCorpusWriter::CompactCorpusWriter(std::string const &basename) :
	d_offset(0)
{
	std::string dataFilename = basename + ".data.dz";
	d_dataStream.reset(new DzOstream(dataFilename.c_str()));
	if (!d_dataStream)
		throw OpenError(dataFilename, "Could not open file for writing");

	std::string indexFilename = basename + ".index";
	d_indexStream.reset(new std::ofstream(indexFilename.c_str()));
	if (!d_indexStream)
		throw OpenError(indexFilename, "Could not open file for writing");
}

void CompactCorpusWriter::copy(CompactCorpusWriter const &other)
{
	d_dataStream = other.d_dataStream;
	d_indexStream = other.d_indexStream;
	d_offset = other.d_offset;
}

void CompactCorpusWriter::writeEntry(std::string const &name, std::string const &data)
{
#if defined(BOOST_HAS_THREADS)
    boost::mutex::scoped_lock lock(d_writeMutex);
#endif

	*d_dataStream << data;
	*d_indexStream << name << "\t" << util::b64_encode(d_offset) << "\t" <<
		util::b64_encode(data.size()) << endl;
	d_offset += data.size();
}

void CompactCorpusWriter::writeEntry(std::string const &name, char const *buf, size_t len)
{
#if defined(BOOST_HAS_THREADS)
    boost::mutex::scoped_lock lock(d_writeMutex);
#endif

	d_dataStream->write(buf, len);
	*d_indexStream << name << "\t" << util::b64_encode(d_offset) << "\t" <<
		util::b64_encode(len) << endl;
	d_offset += len;
}

void CompactCorpusWriter::writeEntry(CorpusReader const &corpus, bool fail_first)
{    
    if (fail_first)
        writeFailFirst(corpus);
    else
        writeFailSafe(corpus);
}

void CompactCorpusWriter::writeFailFirst(CorpusReader const &corpus)
{
    for (CorpusReader::EntryIterator i(corpus.begin()), end(corpus.end());
         i != end; ++i)
        write(*i, corpus.read(*i));
}

void CompactCorpusWriter::writeFailSafe(CorpusReader const &corpus)
{
    BatchError err;

    for (CorpusReader::EntryIterator i(corpus.begin()), end(corpus.end());
         i != end; ++i)
        try {
            write(*i, corpus.read(*i));
        } catch (Error const &e) {
            err.append(e);
        }

    if (!err.empty())
        throw err;
}

}   // namespace alpinocorpus
