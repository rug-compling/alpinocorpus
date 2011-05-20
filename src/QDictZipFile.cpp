/*
 * Copyright 2010 Daniel de Kok
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */


#include <QDateTime>
#include <QFile>
#include <QIODevice>
#include <QSharedPointer>
#include <QString>
#include <QTemporaryFile>

#ifdef _MSC_VER
    #if _MSC_VER >= 1600
        #include <cstdint>
    #else
        #include <AlpinoCorpus/util/vs_stdint.h>
    #endif // _MSC_VER >= 1600
#else
    #include <stdint.h>
#endif // _MSC_VER

#include <cstddef>
#include <limits>
#include <stdexcept>

#include <AlpinoCorpus/QDictZipFile.hh>
#include <AlpinoCorpus/util/bufutil.hh>

#include <zlib.h>

namespace alpinocorpus {

size_t const GZ_HEADER_SIZE = 10;

// Gzip header fields
uint const GZ_HEADER_ID1 = 0;
uint const GZ_HEADER_ID2 = 1;
uint const GZ_HEADER_CM = 2;
uint const GZ_HEADER_FLG = 3;
uint const GZ_HEADER_MTIME = 4;
uint const GZ_HEADER_XFL = 8;
uint const GZ_HEADER_OS = 9;

// Gzip file magic
unsigned char const gzipId1 = 0x1f;
unsigned char const gzipId2 = 0x8b;

// Gzip compression method(s)
unsigned char const GZ_CM_DEFLATE = 8;

// Flags in GZ_HEADER_FLG
unsigned char const GZ_FLG_TEXT = 1;
unsigned char const GZ_FLG_HCRC = 1 << 1;
unsigned char const GZ_FLG_EXTRA = 1 << 2;
unsigned char const GZ_FLG_NAME = 1 << 3;
unsigned char const GZ_FLG_COMMENT = 1 << 4;

// GZ_HEADER_XFL values for deflate
unsigned char const GZ_XFL_MAX = 2;
unsigned char const GZ_XFL_FAST = 4;

// GZ_HEADER_OS values
unsigned char const GZ_OS_FAT = 0;
unsigned char const GZ_OS_AMIGA = 1;
unsigned char const GZ_OS_VMS = 2;
unsigned char const GZ_OS_UNIX = 3;
unsigned char const GZ_OS_VM_CMS = 4;
unsigned char const GZ_OS_TOS = 5;
unsigned char const GZ_OS_HPFS = 6;
unsigned char const GZ_OS_MAC = 7;
unsigned char const GZ_OS_ZSYSTEM = 8;
unsigned char const GZ_OS_CPM = 9;
unsigned char const GZ_OS_TOPS20 = 10;
unsigned char const GZ_OS_NTFS = 11;
unsigned char const GZ_OS_QDOS = 12;
unsigned char const GZ_OS_RISCOS = 13;
unsigned char const GZ_OS_UNKNOWN = 255;

size_t const GZ_TRAILER_SIZE = 8;

// Gzip trailer fields
size_t const GZ_TRAILER_CRC32 = 0;
size_t const GZ_TRAILER_ISIZE = 4;

QDictZipFile::QDictZipFile(QString const &filename, QObject *parent)
    : QIODevice(parent), d_filename(filename), d_crc32(0), d_size(0),
    d_zStream(new z_stream)
{
}

QDictZipFile::~QDictZipFile()
{
    if (openMode() != QIODevice::WriteOnly)
        return;

    // Flush leftovers.
    flushBuffer();

    QByteArray zBuf;
    zBuf.resize(DZ_PREF_UNCOMPRESSED_SIZE);

    d_zStream->next_in = reinterpret_cast<Bytef *>(d_buffer.data());
    d_zStream->avail_in = 0;
    d_zStream->next_out = reinterpret_cast<Bytef *>(zBuf.data());
    d_zStream->avail_out = DZ_PREF_UNCOMPRESSED_SIZE;

    if (deflate(d_zStream.data(), Z_FINISH) != Z_STREAM_END)
      qWarning("QDictZipFile::~QDictZipFile(): %s", d_zStream->msg);

    size_t zSize = DZ_PREF_UNCOMPRESSED_SIZE - d_zStream->avail_out;

    d_tempFile->write(zBuf.constData(), zSize);

    switch (deflateEnd(d_zStream.data())) {
    case Z_STREAM_ERROR:
        qWarning("QDictZipFile::~QDictZipFile(): stream state inconsistent");
    case Z_DATA_ERROR:
        qWarning("QDictZipFile::~QDictZipFile(): stream freed prematurely");
    }

    d_tempFile->close();    

    writeHeader();
    writeChunkInfo();
    writeZData();
    writeTrailer();
}

bool QDictZipFile::atEnd()
{
    if (!isOpen()) {
        qWarning("QDictZipFile::atEnd(): file is not open");
        return false;
    }

    if (openMode() != QIODevice::ReadOnly) {
        qWarning("QDictZipFile::atEnd(): only supported in read-only mode");
        return false;
    }

    if (d_curChunk == d_chunks.size() - 1 && d_bufferPos >= d_bufferSize)
        return true;

    return false;
}

void QDictZipFile::flushBuffer()
{
    if (d_bufferPos == 0)
      return;

    QByteArray zBuf;
    zBuf.resize(DZ_PREF_UNCOMPRESSED_SIZE);

    d_zStream->next_in = reinterpret_cast<Bytef *>(d_buffer.data());
    d_zStream->avail_in = d_bufferPos;
    d_zStream->next_out = reinterpret_cast<Bytef *>(zBuf.data());
    d_zStream->avail_out = DZ_PREF_UNCOMPRESSED_SIZE;

    if (deflate(d_zStream.data(), Z_FULL_FLUSH) != Z_OK) {
      qWarning("QDictZipFile::~QDictZipFile(): %s", d_zStream->msg);
      return;
    }

    size_t zSize = DZ_PREF_UNCOMPRESSED_SIZE - d_zStream->avail_out;

    d_tempFile->write(zBuf.constData(), zSize);

    d_size += d_bufferPos;
    d_crc32 = crc32(d_crc32, reinterpret_cast<Bytef *>(d_buffer.data()),
        d_bufferPos);

    d_bufferPos = 0;
    d_chunks.push_back(DzChunk(0, zSize));
}

bool QDictZipFile::open(OpenMode mode)
{    
    if (mode == QIODevice::ReadOnly) {
        bool status = readOpen();
        if (status)
            setOpenMode(mode);
        return status;
    }
    else if (mode == QIODevice::WriteOnly) {
      bool status = writeOpen();
      if (status)
	setOpenMode(mode);
      return status;
    }
    else
        return false;
}

void QDictZipFile::readChunk(qint64 n)
{
    if (n == d_curChunk)
        return;

    DzChunk chunkN = d_chunks[n];

    d_file->seek(d_dataOffset + chunkN.offset);
    QByteArray zBuf = d_file->read(chunkN.size);

    z_stream zStream;
    zStream.next_in = reinterpret_cast<Bytef *>(zBuf.data());
    zStream.avail_in = chunkN.size;
    zStream.next_out = reinterpret_cast<Bytef *>(d_buffer.data());
    zStream.avail_out = d_chunkLen;
    zStream.zalloc = NULL;
    zStream.zfree = NULL;

    if (inflateInit2(&zStream, -15) != Z_OK)
        throw std::runtime_error(zStream.msg);

    int r = inflate(&zStream, Z_PARTIAL_FLUSH);
    if (r != Z_OK && r != Z_STREAM_END)
        throw std::runtime_error(zStream.msg);

    if (inflateEnd(&zStream) != Z_OK)
        throw std::runtime_error(zStream.msg);

    d_curChunk = n;
    d_bufferPos = 0;
    d_bufferSize = zStream.total_out;
}

qint64 QDictZipFile::readData(char *data, qint64 maxlen)
{
    if (!isOpen()) {
        qWarning("QDictZipFile::readData(): file is not open");
        return -1;
    }

    if (openMode() != QIODevice::ReadOnly) {
        qWarning("QDictZipFile::readData(): instance not opened for reading");
        return -1;
    }

    if (d_bufferPos >= d_bufferSize && d_curChunk + 1 >= d_chunks.size())
        return -1;

    qint64 read = 0;
    while (read != maxlen)
    {
        // We have copied the full buffer, read the next chunk.
        if (d_bufferPos >= d_bufferSize) {
            if (d_curChunk + 1 >= d_chunks.size())
                break;
            readChunk(d_curChunk + 1);
        }

        // Determine the amount of data to read.
        qint64 toCopy = d_bufferSize - d_bufferPos;
        if (toCopy > maxlen - read)
            toCopy = maxlen - read;

        // Copy data to the caller's buffer.
        memcpy(data + read, d_buffer.constData() + d_bufferPos, toCopy);

        d_bufferPos += toCopy;
        read += toCopy;
    }

    return read;
}

void QDictZipFile::readExtra()
{
    QByteArray extraLenData(d_file->read(2));
    quint64 extraLen = static_cast<unsigned char>(extraLenData[0]) +
                       (static_cast<unsigned char>(extraLenData[1]) * 256);

    qint64 extraPos = d_file->pos();
    qint64 nextField = extraPos + extraLen;

    while (d_file->pos() < nextField)
    {
        // Read extra field 'header'
        QByteArray si(d_file->read(2));
        QByteArray lenData(d_file->read(2));
        int len = static_cast<unsigned char>(lenData[0]) +
                  (static_cast<unsigned char>(lenData[1]) * 256);

        // Does this field part provide chunk information?
        if (si[0] == 'R' && si[1] == 'A')
        {
            QByteArray verData(d_file->read(2));
            int ver = static_cast<unsigned char>(verData[0]) +
                      (static_cast<unsigned char>(verData[1]) * 256);
            if (ver != 1)
                throw("DzIstreamBuf::readExtra: unknown dictzip version");

            // Chunk length/count.
            QByteArray chunkLenData(d_file->read(2));
            d_chunkLen = static_cast<unsigned char>(chunkLenData[0]) +
                         (static_cast<unsigned char>(chunkLenData[1]) * 256);
            QByteArray chunkCountData(d_file->read(2));
            quint64 chunkCount = static_cast<unsigned char>(chunkCountData[0]) +
                                 (static_cast<unsigned char>(chunkCountData[1]) * 256);


            // Set up buffer.
            d_buffer.resize(d_chunkLen);
            d_bufferPos = d_buffer.size();
            d_bufferSize = 0;

            quint64 chunkPos = 0;
            for (size_t i = 0; i < chunkCount; ++i)
            {
                chunkLenData = d_file->read(2);
                size_t chunkLen = static_cast<unsigned char>(chunkLenData[0]) +
                                  (static_cast<unsigned char>(chunkLenData[1]) * 256);
                d_chunks.push_back(DzChunk(chunkPos, chunkLen));
                chunkPos += chunkLen;
            }
        }
        else
            d_file->seek(d_file->pos() + len);
    }
}

void QDictZipFile::readHeader()
{
    d_header = d_file->read(GZ_HEADER_SIZE);

    if (static_cast<unsigned char>(d_header[GZ_HEADER_ID1]) != gzipId1 ||
            static_cast<unsigned char>(d_header[GZ_HEADER_ID2]) != gzipId2)
        throw std::runtime_error("DzIstreamBuf::readHeader: not a gzip file");

    if (static_cast<unsigned char>(d_header[GZ_HEADER_CM]) != GZ_CM_DEFLATE)
        throw std::runtime_error("DzIstreamBuf::readHeader: unknown compression method");

    if (!(d_header[GZ_HEADER_FLG] & GZ_FLG_EXTRA))
        throw std::runtime_error("DzIstreamBuf::readHeader: no extra fields, cannot be a dictzip file");
}

bool QDictZipFile::seek(qint64 pos)
{
  if (!isOpen()) {
      qWarning("QDictZipFile::seek(): cannot seek on unopened file");
      return false;
  }

  if (openMode() != QIODevice::ReadOnly) {
      qWarning("QDictZipFile::seek(): seeking is only allowed on read-only streams");
      return false;
  }

    QIODevice::seek(pos);

    if (pos < 0)
        return false;

    qint64 targetChunk = pos / d_chunkLen;
    qint64 chunkPos = pos % d_chunkLen;

    if (targetChunk >= d_chunks.size())
        return false;

    readChunk(targetChunk);

    d_bufferPos = chunkPos;

    // The last chunk can be shorter that uncompressed chunk size.
    if (chunkPos >= d_bufferSize)
        return false;

    return true;
}

void QDictZipFile::skipOptional()
{
    if (d_header[GZ_HEADER_FLG] & GZ_FLG_NAME)
        while (true)
        {
            char c;
            d_file->getChar(&c);
            if (c == 0)
                break;
        }

    if (d_header[GZ_HEADER_FLG] & GZ_FLG_COMMENT)
        while (true)
        {
            char c;
            d_file->getChar(&c);
            if (c == 0)
                break;
        }

    if (d_header[GZ_HEADER_FLG] & GZ_FLG_HCRC)
        d_file->seek(d_file->pos() + 2);
}

bool QDictZipFile::readOpen()
{
    d_file = QSharedPointer<QFile>(new QFile(d_filename));
    bool r = d_file->open(QFile::ReadOnly);
    if (!r)
        return r;

    readHeader();
    readExtra();
    skipOptional();
    d_dataOffset = d_file->pos();
    d_curChunk = -1;

    return r;
}

void QDictZipFile::writeChunkInfo()
{
    size_t xlen = 10 + (2 * d_chunks.size());
    d_file->putChar(xlen % 256);
    d_file->putChar(xlen / 256);
        
    d_file->putChar('R');
    d_file->putChar('A');

    // Length
    size_t len = 6 + (2 * d_chunks.size());
    d_file->putChar(len % 256);
    d_file->putChar(len / 256);
        
    // Version
    d_file->putChar(1);
    d_file->putChar(0);
        
    // Uncompressed chunk length
    d_file->putChar(DZ_PREF_UNCOMPRESSED_SIZE % 256);
    d_file->putChar(DZ_PREF_UNCOMPRESSED_SIZE / 256);

    // Chunk count
    d_file->putChar(d_chunks.size() % 256);
    d_file->putChar(d_chunks.size() / 256);
        
    // Put chunk information
    for (QVector<DzChunk>::const_iterator iter = d_chunks.begin();
         iter != d_chunks.end(); ++iter)
      {
        d_file->putChar(iter->size % 256);
        d_file->putChar(iter->size / 256);
      }
}

qint64 QDictZipFile::writeData(const char *data, qint64 maxSize)
{
    if (!isOpen())
    {
        qWarning("QDictZipFile::writeData(): file is not open");
        return -1;
    }

    if (openMode() != QIODevice::WriteOnly) {
        qWarning("QDictZipFile::writeData(): instance not opened for writing");
        return -1;
    }

    qint64 written = 0;
    while (written < maxSize)
    {
        qint64 avail = d_bufferSize - d_bufferPos;
        if (avail == 0) {
            flushBuffer();
            avail = d_bufferSize;
        }

        qint64 n = maxSize - written;
        if (n > avail)
            n = avail;
        memcpy(d_buffer.data() + d_bufferPos, data + written, n);

        written += n;
        d_bufferPos += n;
    }

    return written;
}

void QDictZipFile::writeHeader()
{
    QByteArray header;
    header.resize(GZ_HEADER_SIZE);

    // Get the current time. gzip only allows for 32-bit timestamps.
    uint32_t sec = QDateTime::currentDateTime().toTime_t();
    if (sec > std::numeric_limits<uint32_t>::max())
      sec = 0;

    header[GZ_HEADER_ID1] = gzipId1;
    header[GZ_HEADER_ID2] = gzipId2;
    header[GZ_HEADER_CM] = GZ_CM_DEFLATE;
    header[GZ_HEADER_FLG] = GZ_FLG_EXTRA;
    writeToBuf<uint32_t>(header.data() + GZ_HEADER_MTIME, sec);
    header[GZ_HEADER_XFL] = GZ_XFL_MAX;
    header[GZ_HEADER_OS] = GZ_OS_UNIX;
    
    d_file->write(header.data(), GZ_HEADER_SIZE);
}

bool QDictZipFile::writeOpen()
{
    d_tempFile = QSharedPointer<QTemporaryFile>(new QTemporaryFile);
    if (!d_tempFile->open()) {
        qWarning("Could not open a temporary file for writing.");
        return false;
    }

    d_file = QSharedPointer<QFile>(new QFile(d_filename));
    if (!d_file->open(QIODevice::WriteOnly)) {
        qWarning("Could not open target file for writing.");
        return false;
    }

    d_zStream->next_in = Z_NULL;
    d_zStream->avail_in = 0;
    d_zStream->next_out = Z_NULL;
    d_zStream->avail_out = 0;
    d_zStream->zalloc = Z_NULL;
    d_zStream->zfree = Z_NULL;

    if (deflateInit2(d_zStream.data(), Z_BEST_COMPRESSION, Z_DEFLATED, -15,
		     Z_BEST_COMPRESSION, Z_DEFAULT_STRATEGY) != Z_OK) {
      qWarning("QDictZipFile::writeOpen: %s", d_zStream->msg);
      return false;
    }

    d_bufferSize = DZ_PREF_UNCOMPRESSED_SIZE;
    d_bufferPos = 0;
    d_buffer.resize(d_bufferSize);

    return true;
}

void QDictZipFile::writeTrailer()
{
    QByteArray trailer;
    trailer.resize(GZ_TRAILER_SIZE);

    writeToBuf<uint32_t>(trailer.data() + GZ_TRAILER_CRC32, d_crc32);
    writeToBuf<uint32_t>(trailer.data() + GZ_TRAILER_ISIZE, d_size);
  
    d_file->write(trailer.data(), GZ_TRAILER_SIZE);
}

void QDictZipFile::writeZData()
{
    QFile chunkFile(d_tempFile->fileName());
    if (!chunkFile.open(QIODevice::ReadOnly)) {
        qWarning("QDictZipFile::~QDictZipFile(): could not open temporary chunk file");
        return;
    }

    qint64 const BUFSIZE = 0xffff;
    char buf[BUFSIZE];
    size_t n;
    while ((n = chunkFile.read(buf, BUFSIZE)))
        d_file->write(buf, n);
}

}
