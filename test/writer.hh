#ifndef ALPINOCORPUS_WRITER_TEST
#define ALPINOCORPUS_WRITER_TEST

#include <QObject>

#include <AlpinoCorpus/CorpusReader.hh>

namespace ac = alpinocorpus;

class DbCorpusWriterTest : public QObject
{
    Q_OBJECT
private slots:
    void canOpenDbForWriting();
    void canWriteEntireReader();
};

#endif // ALPINOCORPUS_WRITER_TEST
