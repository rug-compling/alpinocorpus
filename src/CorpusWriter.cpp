#include <QString>

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/CorpusWriter.hh>

namespace alpinocorpus {
    void CorpusWriter::write(QString const &name, QString const &content)
    {
        writeEntry(name, content);
    }
    
    void CorpusWriter::write(CorpusReader const &corpus, bool failsafe)
    {
        writeEntry(corpus, failsafe);
    }
}
