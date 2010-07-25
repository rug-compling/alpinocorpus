#ifndef CORPUSREADER_HH
#define CORPUSREADER_HH

#include <QString>
#include <QVector>

#include "DLLDefines.hh"

namespace alpinocorpus {

class CorpusReader
{
public:
	virtual ~CorpusReader() {}
    virtual QString read(QString const &entry) = 0;
    virtual QVector<QString> entries() const = 0;

    static INDEXED_CORPUS_EXPORT CorpusReader *newCorpusReader(QString const &corpusPath);
};

}

#endif // CORPUSREADER_HH
