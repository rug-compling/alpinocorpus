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

#ifndef DICTZIPFILE_HH
#define DICTZIPFILE_HH

#include <QByteArray>
#include <QFile>
#include <QIODevice>
#include <QObject>
#include <QSharedPointer>
#include <QString>
#include <QTemporaryFile>
#include <QVector>

#include <zlib.h>

#include <AlpinoCorpus/DzChunk.hh>

namespace alpinocorpus {

size_t const DZ_MAX_COMPRESSED_SIZE = 0xffffUL;
size_t const DZ_MAX_UNCOMPRESSED_SIZE = static_cast<size_t>(DZ_MAX_COMPRESSED_SIZE - 12) * 0.999;

// This seems bogus to me, but there are too many dictunzipping programs out there with this as
// a fixed buffer size, prefer it by default.
size_t const DZ_PREF_UNCOMPRESSED_SIZE = static_cast<size_t>((DZ_MAX_COMPRESSED_SIZE - 12) * 0.89);

/*!
 * This class implements a reader for dictzip-compressed files. Dictzip is a
 * variant of the gzip format that allows 'nearly' for random-access. This
 * is achieved by compressing files in chunks. To decompress a block of data,
 * only the chunks encapsulating the data need to be decompressed. Since each
 * chunk is independent, compression is slightly worse than stock gzip.
 */

class QDictZipFile : public QIODevice
{
public:
    /*!
     * Construct a QDictZipFile instance.
     *
     * \param filename The name of the file associated with the instance.
     * \param parent Parent of the instance.
     */
    QDictZipFile(QString const &filename, QObject *parent = 0);

    ~QDictZipFile();

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
     *     QDictZipFile::ReadOnly is the only supported mode.
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
protected:
    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);
private:
    QDictZipFile(QDictZipFile const &other);
    QDictZipFile &operator=(QDictZipFile const &other);

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
    uLong d_crc32;
    qint64 d_dataOffset;
    qint64 d_curChunk;
    QVector<DzChunk> d_chunks;
    QByteArray d_buffer;
    qint64 d_bufferSize;
    qint64 d_bufferPos;
    size_t d_size;
    z_stream d_zStream;
};

}

#endif // DICTZIPFILE_HH
