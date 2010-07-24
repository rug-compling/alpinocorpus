#include "DictZipFile.ih"

DictZipFile::DictZipFile(QString const &filename, QObject *parent)
        : QIODevice(parent), d_filename(filename)
{
}

DictZipFile::~DictZipFile()
{
}

bool DictZipFile::atEnd()
{
    if (!isOpen()) {
        qWarning("DictZipFile::atEnd(): file is not open");
        return false;
    }

    if (d_curChunk == d_chunks.size() - 1 && d_bufferPos >= d_bufferSize)
        return true;

    return false;
}

bool DictZipFile::open(OpenMode mode)
{    
    if (mode == QIODevice::ReadOnly) {
        bool status = readOpen();
        if (status)
            setOpenMode(mode);
        return status;
    }
    else
        return false;
}

void DictZipFile::readChunk(qint64 n)
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
        throw runtime_error(zStream.msg);

    int r = inflate(&zStream, Z_PARTIAL_FLUSH);
    if (r != Z_OK && r != Z_STREAM_END)
        throw runtime_error(zStream.msg);

    if (inflateEnd(&zStream) != Z_OK)
        throw runtime_error(zStream.msg);

    d_curChunk = n;
    d_bufferPos = 0;
    d_bufferSize = zStream.total_out;
}

qint64 DictZipFile::readData(char *data, qint64 maxlen)
{
    if (!isOpen())
    {
        qWarning("DictZipFile::readData(): file is not open");
        return -1;
    } else if (d_bufferPos >= d_bufferSize && d_curChunk + 1 >= d_chunks.size())
        return -1;

    quint64 read = 0;
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

void DictZipFile::readExtra()
{
    QByteArray extraLenData(d_file->read(2));
    quint64 extraLen = static_cast<unsigned char>(extraLenData[0]) +
                       (static_cast<unsigned char>(extraLenData[1]) * 256);

    quint64 extraPos = d_file->pos();
    quint64 nextField = extraPos + extraLen;

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
                throw("DzIstreamBuf::readExtra: unknown dictzip version!");

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

void DictZipFile::readHeader()
{
    d_header = d_file->read(GZ_HEADER_SIZE);

    if (static_cast<unsigned char>(d_header[GZ_HEADER_ID1]) != gzipId1 ||
            static_cast<unsigned char>(d_header[GZ_HEADER_ID2]) != gzipId2)
        throw runtime_error("DzIstreamBuf::readHeader: not a gzip file!");

    if (static_cast<unsigned char>(d_header[GZ_HEADER_CM]) != GZ_CM_DEFLATE)
        throw runtime_error("DzIstreamBuf::readHeader: unknown compression method!");

    if (!(d_header[GZ_HEADER_FLG] & GZ_FLG_EXTRA))
        throw runtime_error("DzIstreamBuf::readHeader: no extra fields, cannot be a dictzip file!");
}

bool DictZipFile::seek(qint64 pos)
{
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

void DictZipFile::skipOptional()
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

bool DictZipFile::readOpen()
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

qint64 DictZipFile::writeData(const char *data, qint64 len)
{
    return -1;
}
