#ifndef DZ_OSTREAMBUF_HH
#define DZ_OSTREAMBUF_HH

#include <cstdio>
#include <memory>
#include <streambuf>
#include <string>
#include <vector>

#include <zlib.h>

#include "gzip.hh"

namespace alpinocorpus {
struct DzChunk
{
    DzChunk(size_t newOffset, size_t newSize) : offset(newOffset), size(newSize) {}
    size_t offset;
    size_t size;
};

// Warning: streambufs are really too stateful for multithreading (see
// work done during destruction). Users of this classes should do locking.
class DzOstreamBuf : public std::streambuf {
public:
	DzOstreamBuf(char const *filename);
	virtual ~DzOstreamBuf();
protected:
	virtual int overflow(int c);
	virtual std::streamsize xsputn(char const *s, std::streamsize n);
private:
	DzOstreamBuf(DzOstreamBuf const &other);
	DzOstreamBuf &operator=(DzOstreamBuf const &other);
	void flushBuffer();
	void writeChunkInfo();
	void writeHeader();
	void writeTrailer();
	void writeZData();

	std::string d_tmpFilename;
	FILE *d_dzStream;
	FILE *d_zDataStream;
	z_stream d_zStream;
	std::vector<unsigned char> d_buffer;
	size_t d_size;
	uLong d_crc32;
	std::vector<DzChunk> d_chunks;
};

}

#endif //DZ_OSTREAMBUF_HH

