#ifndef CORPUSREADER_HH
#define CORPUSREADER_HH

#include <QString>
#include <QVector>

namespace indexedcorpus {

class CorpusReader
{
public:
    virtual ~CorpusReader() {}
    virtual QString read(QString const &entry) = 0;
    virtual QVector<QString> entries() const = 0;
};

}

#endif // CORPUSREADER_HH
