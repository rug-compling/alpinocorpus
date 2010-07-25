#ifndef DICTZIPFILE_HH
#define DICTZIPFILE_HH

#include <QByteArray>
#include <QFile>
#include <QIODevice>
#include <QObject>
#include <QSharedPointer>
#include <QString>
#include <QVector>

#include "DzChunk.hh"

namespace alpinocorpus {

class DictZipFile : public QIODevice
{
public:
    DictZipFile(QString const &filename, QObject *parent = 0);
    ~DictZipFile();
    bool atEnd();
    bool open(OpenMode mode);
    bool seek(qint64 pos);
protected:
    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);
private:
    void readChunk(qint64 n);
    void readExtra();
    void readHeader();
    bool readOpen();
    void skipOptional();

    QString d_filename;
    QSharedPointer<QFile> d_file;
    QByteArray d_header;
    quint64 d_chunkLen;
    quint64 d_dataOffset;
    qint64 d_curChunk;
    QVector<DzChunk> d_chunks;
    QByteArray d_buffer;
    quint64 d_bufferSize;
    qint64 d_bufferPos;
};

}

#endif // DICTZIPFILE_HH
