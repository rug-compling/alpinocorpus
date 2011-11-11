#include <cmath>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

#include <zlib.h>

#include <boost/cstdint.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>

#include <util/bufutil.hh>

#include "DzOstreamBuf.hh"
#include "gzip.hh"

namespace bf = boost::filesystem;
namespace pt = boost::posix_time;
namespace bg = boost::gregorian;

namespace {

size_t const DZ_MAX_COMPRESSED_SIZE = 0xffffUL;
size_t const DZ_MAX_UNCOMPRESSED_SIZE = static_cast<size_t>(DZ_MAX_COMPRESSED_SIZE - 12) * 0.999;

// This seems bogus to me, but there are too many dictunzipping programs out there with this as
// a fixed buffer size, prefer it by default.
size_t const DZ_PREF_UNCOMPRESSED_SIZE = static_cast<size_t>((DZ_MAX_COMPRESSED_SIZE - 12) * 0.89);

}

namespace alpinocorpus {

DzOstreamBuf::DzOstreamBuf(char const *filename) : d_size(0), d_crc32(crc32(0L, Z_NULL, 0))
{
	d_dzStream = fopen(filename, "w");
	
	if (d_dzStream == NULL) {
		d_zDataStream = NULL;
		setp(reinterpret_cast<char *>(&d_buffer[0]),
			reinterpret_cast<char *>(&d_buffer[0]));
		return;
	}

  // XXX - There is a race condition here, but Boost does not seem to
  // provide a variant that returns a file descriptor. We used mkstemp
  // previously, but it is not portable.
  std::string tmpFilename =
    bf::unique_path(std::string(filename) + "-%%%%-%%%%-%%%%-%%%%").string();
  d_tmpFilename = tmpFilename;
  d_zDataStream = fopen(tmpFilename.c_str(), "w");
	
	if (d_zDataStream == NULL)
		throw std::runtime_error(std::string("DzOstreamBuf::DzOstreamBuf: Could not open ") +
			d_tmpFilename + " for writing!");

	d_zStream.next_in = Z_NULL;
	d_zStream.avail_in = 0;
	d_zStream.next_out = Z_NULL;
	d_zStream.avail_out = 0;
	d_zStream.zalloc = Z_NULL;
	d_zStream.zfree = Z_NULL;
	
	if (deflateInit2(&d_zStream, Z_BEST_COMPRESSION, Z_DEFLATED, -15,
			Z_BEST_COMPRESSION, Z_DEFAULT_STRATEGY) != Z_OK)
		throw std::runtime_error(d_zStream.msg);

	d_buffer.resize(DZ_PREF_UNCOMPRESSED_SIZE);
	
	// Set up the buffer. However, we act as if it is one character smaller,
	// so that we can write a chunk when we get the last character.
	setp(reinterpret_cast<char *>(&d_buffer[0]),
		reinterpret_cast<char *>(&d_buffer[0] + DZ_PREF_UNCOMPRESSED_SIZE));
}

DzOstreamBuf::~DzOstreamBuf()
{
	if (d_zDataStream == NULL || d_dzStream == NULL)
		return;

	// Flush leftovers.
	try {
		flushBuffer();
	} catch (std::exception &e) {

		std::cerr << e.what() << std::endl;
	}

	std::vector<unsigned char> zBuf(DZ_PREF_UNCOMPRESSED_SIZE);
	d_zStream.next_in = &d_buffer[0];
	d_zStream.avail_in = 0;
	d_zStream.next_out = &zBuf[0];
	d_zStream.avail_out = DZ_PREF_UNCOMPRESSED_SIZE;
	if (deflate(&d_zStream, Z_FINISH) != Z_STREAM_END)
		std::cerr << d_zStream.msg << std::endl;
	size_t zSize = DZ_PREF_UNCOMPRESSED_SIZE - d_zStream.avail_out;
	fwrite(&zBuf[0], 1, zSize, d_zDataStream);
	
	switch (deflateEnd(&d_zStream)) {
	case Z_STREAM_ERROR:
		std::cerr << "DzOstreamBuf::flushBuffer: stream state inconsistent!" << std::endl;
	case Z_DATA_ERROR:
		std::cerr << "DzOstreamBuf::flushBuffer: stream freed prematurely!" << std::endl;
	}

	fclose(d_zDataStream);
	
	writeHeader();
	writeChunkInfo();
	writeZData();
	writeTrailer();
	
	fclose(d_dzStream);

  bf::remove(d_tmpFilename);
}

void DzOstreamBuf::flushBuffer()
{
	size_t size = pptr() - pbase();
	
	std::vector<unsigned char> zBuf(DZ_PREF_UNCOMPRESSED_SIZE);
	
	d_zStream.next_in = reinterpret_cast<unsigned char *>(pbase());
	d_zStream.avail_in = size;
	d_zStream.next_out = &zBuf[0];
	d_zStream.avail_out = DZ_PREF_UNCOMPRESSED_SIZE;
	
	if (deflate(&d_zStream, Z_FULL_FLUSH) != Z_OK)
		throw std::runtime_error(d_zStream.msg);		
	
	size_t zSize = DZ_PREF_UNCOMPRESSED_SIZE - d_zStream.avail_out;
	
	fwrite(&zBuf[0], 1, zSize, d_zDataStream);

	d_size += size;
	d_crc32 = crc32(d_crc32, reinterpret_cast<unsigned char *>(pbase()), size);
	d_chunks.push_back(DzChunk(0, zSize));

	pbump(-size);
}

int DzOstreamBuf::overflow(int c)
{
	flushBuffer();

	if (c != EOF)
	{
		*pptr() = c;
		pbump(1);
	}
	
	return c;
}

void DzOstreamBuf::writeChunkInfo()
{
	size_t xlen = 10 + (2 * d_chunks.size());
	fputc(xlen % 256, d_dzStream);
	fputc(xlen / 256, d_dzStream);
	
	fputc('R', d_dzStream);
	fputc('A', d_dzStream);
	
	// Length
	size_t len = 6 + (2 * d_chunks.size());
	fputc(len % 256, d_dzStream);
	fputc(len / 256, d_dzStream);
	
	// Version
	fputc(1, d_dzStream);
	fputc(0, d_dzStream);
	
	// Uncompressed chunk length
	fputc(DZ_PREF_UNCOMPRESSED_SIZE % 256, d_dzStream);
	fputc(DZ_PREF_UNCOMPRESSED_SIZE / 256, d_dzStream);
	
	// Chunk count
	fputc(d_chunks.size() % 256, d_dzStream);
	fputc(d_chunks.size() / 256, d_dzStream);
	
	// Put chunk information
	for (std::vector<DzChunk>::const_iterator iter = d_chunks.begin();
		iter != d_chunks.end(); ++iter)
	{
		fputc(iter->size % 256, d_dzStream);
		fputc(iter->size / 256, d_dzStream);
	}
}

void DzOstreamBuf::writeHeader()
{
	std::vector<unsigned char> header(GZ_HEADER_SIZE);

	// Get the current time. gzip only allows for 32-bit timestamps.
  pt::ptime epoch(bg::date(1970, 1, 1));
  pt::time_duration diff = pt::second_clock::universal_time() - epoch;
 
  long secsSinceEpoch;
  if (diff.total_seconds() > std::numeric_limits<int32_t>::max())
      secsSinceEpoch = 0;
  else
      secsSinceEpoch = diff.total_seconds();

	header[GZ_HEADER_ID1] = gzipId1;
	header[GZ_HEADER_ID2] = gzipId2;
	header[GZ_HEADER_CM] = GZ_CM_DEFLATE;
	header[GZ_HEADER_FLG] = GZ_FLG_EXTRA;
	util::writeToBuf<uint32_t>(&header[0] + GZ_HEADER_MTIME, secsSinceEpoch);
	header[GZ_HEADER_XFL] = GZ_XFL_MAX;
	header[GZ_HEADER_OS] = GZ_OS_UNIX;
	
	fwrite(&header[0], 1, GZ_HEADER_SIZE, d_dzStream);
}

void DzOstreamBuf::writeTrailer()
{
	std::vector<unsigned char> trailer(GZ_TRAILER_SIZE);
	
	util::writeToBuf<uint32_t>(&trailer[0] + GZ_TRAILER_CRC32, d_crc32);
	util::writeToBuf<uint32_t>(&trailer[0] + GZ_TRAILER_ISIZE, d_size);

	fwrite(&trailer[0], 1, GZ_TRAILER_SIZE, d_dzStream);
}

void DzOstreamBuf::writeZData()
{
	FILE *chunkStream = fopen(d_tmpFilename.c_str(), "r");

	size_t const BUFSIZE = 0xffff;
	char buf[BUFSIZE];
	size_t n;
	
	while ((n = fread(buf, 1, BUFSIZE, chunkStream)))
		fwrite(buf, 1, n, d_dzStream);
	
	fclose(chunkStream);
}

std::streamsize DzOstreamBuf::xsputn(char const *s, std::streamsize n)
{
	int nWritten = 0;
	
	while (n) {
		int avail = epptr() - pptr();

		// If the buffer is full, flush it.
		if (avail == 0) {
			flushBuffer();
			avail = epptr() - pptr();
		}

		if (avail > n)
			avail = n;
		
		// Copy data, move pointer.
		memcpy(pptr(), s + nWritten, avail);
		pbump(avail);
		
		nWritten += avail;
		n -= avail;
	}
	
	return nWritten;
}

}
