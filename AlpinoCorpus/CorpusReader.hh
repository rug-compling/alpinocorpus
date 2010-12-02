#ifndef ALPINO_CORPUSREADER_HH
#define ALPINO_CORPUSREADER_HH

#include <QString>
#include <QVector>

#include <AlpinoCorpus/DLLDefines.hh>

namespace alpinocorpus {

class CorpusReader
{
public:
	virtual ~CorpusReader() {}
    virtual bool open() = 0;
    virtual QString read(QString const &entry) = 0;
    virtual QVector<QString> entries() const = 0;

    static INDEXED_CORPUS_EXPORT CorpusReader *newCorpusReader(QString const &corpusPath);
};

}

#endif  // ALPINO_CORPUSREADER_HH
