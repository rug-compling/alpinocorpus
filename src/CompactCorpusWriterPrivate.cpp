#include <fstream>
#include <mutex>
#include <string>

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/Entry.hh>
#include <AlpinoCorpus/Error.hh>

#include "CompactCorpusWriterPrivate.hh"
#include "DzOstream.hh"
#include "util/base64.hh"

using namespace std;

namespace alpinocorpus {


CompactCorpusWriterPrivate::CompactCorpusWriterPrivate(std::string const &basename) :
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

void CompactCorpusWriterPrivate::copy(CompactCorpusWriterPrivate const &other)
{
	d_dataStream = other.d_dataStream;
	d_indexStream = other.d_indexStream;
	d_offset = other.d_offset;
}

void CompactCorpusWriterPrivate::writeEntry(std::string const &name, std::string const &data)
{
  std::lock_guard<std::mutex> lock(d_writeMutex);

	*d_dataStream << data;
	*d_indexStream << name << "\t" << util::b64_encode(d_offset) << "\t" <<
		util::b64_encode(data.size()) << endl;
	d_offset += data.size();
}

void CompactCorpusWriterPrivate::writeEntry(std::string const &name, char const *buf, size_t len)
{
  std::lock_guard<std::mutex> lock(d_writeMutex);

	d_dataStream->write(buf, len);
	*d_indexStream << name << "\t" << util::b64_encode(d_offset) << "\t" <<
		util::b64_encode(len) << endl;
	d_offset += len;
}

void CompactCorpusWriterPrivate::writeEntry(CorpusReader const &corpus, bool fail_first)
{    
    if (fail_first)
        writeFailFirst(corpus);
    else
        writeFailSafe(corpus);
}

void CompactCorpusWriterPrivate::writeFailFirst(CorpusReader const &corpus)
{
    CorpusReader::EntryIterator i = corpus.entries();
    while(i.hasNext())
    {
        Entry e = i.next(corpus);
        write(e.name, corpus.read(e.name));
    }
}

void CompactCorpusWriterPrivate::writeFailSafe(CorpusReader const &corpus)
{
    BatchError err;

    CorpusReader::EntryIterator i = corpus.entries();
    while(i.hasNext())
    {
        Entry e = i.next(corpus);
        try {
            write(e.name, corpus.read(e.name));
        } catch (Error const &e) {
            err.append(e);
        }
    }

    if (!err.empty())
        throw err;
}

}   // namespace alpinocorpus
