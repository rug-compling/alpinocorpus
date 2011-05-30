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

#ifndef ALPINO_DICTZIPFILE_PRIVATE_HH
#define ALPINO_DICTZIPFILE_PRIVATE_HH

#include <QByteArray>
#include <QIODevice>
#include <QSharedPointer>
#include <QString>
#include <QVector>

#include <zlib.h>

extern "C" {
    struct z_stream_s;
    typedef z_stream_s z_stream;
}

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
    
    size_t const DZ_MAX_COMPRESSED_SIZE = 0xffffUL;
    size_t const DZ_MAX_UNCOMPRESSED_SIZE = static_cast<size_t>(DZ_MAX_COMPRESSED_SIZE - 12) * 0.999;
    
    // This seems bogus to me, but there are too many dictunzipping programs out there with this as
    // a fixed buffer size, prefer it by default.
    size_t const DZ_PREF_UNCOMPRESSED_SIZE = static_cast<size_t>((DZ_MAX_COMPRESSED_SIZE - 12) * 0.89);
    
    struct DzChunk
    {
        DzChunk(quint64 newOffset, quint64 newSize) : offset(newOffset), size(newSize) {}
        DzChunk() : offset(0), size(0) {} // hmz
        quint64 offset;
        quint64 size;
    };
    
    /*!
     * This class implements a reader for dictzip-compressed files. Dictzip is a
     * variant of the gzip format that allows 'nearly' for random-access. This
     * is achieved by compressing files in chunks. To decompress a block of data,
     * only the chunks encapsulating the data need to be decompressed. Since each
     * chunk is independent, compression is slightly worse than stock gzip.
     */
    
    class QDictZipFilePrivate : public QIODevice
    {
    public:
        /*!
         * Construct a QDictZipFile instance.
         *
         * \param filename The name of the file associated with the instance.
         * \param parent Parent of the instance.
         */
        QDictZipFilePrivate(QString const &filename, QObject *parent = 0);
        
        ~QDictZipFilePrivate();
        
        /*!
         * Check whether the end of a dictzip file has been reached.
         *
         * \return true if the end has been reached, false otherwise.
         */
        bool atEnd();
        
        /*!
         * Open the dictzip file.
         *
         * \param mode The mode for opening the file. Currently,
         *     QDictZipFilePrivate::ReadOnly is the only supported mode.
         * \return true if the file could be opened, false otherwise.
         */
        bool open(OpenMode mode);
        
        /*!
         * Seek in the dictzip file.
         *
         * \param pos Position in the uncompressed data.
         * \return true if the seek was succesful, false otherwise.
         */
        bool seek(qint64 pos);
        
        qint64 readData(char *data, qint64 maxlen);
        qint64 writeData(const char *data, qint64 len);

    private:    
        void flushBuffer();
        void readChunk(qint64 n);
        void readExtra();
        void readHeader();
        bool readOpen();
        void skipOptional();
        void writeChunkInfo();
        void writeHeader();
        bool writeOpen();
        void writeTrailer();
        void writeZData();
        
        QString d_filename;
        QSharedPointer<QFile> d_file;
        QSharedPointer<QTemporaryFile> d_tempFile;
        QByteArray d_header;
        qint64 d_chunkLen;
        unsigned long d_crc32;
        qint64 d_dataOffset;
        qint64 d_curChunk;
        QVector<DzChunk> d_chunks;
        QByteArray d_buffer;
        qint64 d_bufferSize;
        qint64 d_bufferPos;
        size_t d_size;
        QSharedPointer<z_stream> d_zStream;
    };
}

#endif // ALPINO_DICTZIPFILE_PRIVATE_HH
