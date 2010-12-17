#ifndef ALPINO_CORPUSWRITER_HH
#define ALPINO_CORPUSWRITER_HH

#include <AlpinoCorpus/CorpusReader.hh>
#include <QString>

namespace alpinocorpus {

/**
 * Abstract base class for corpus writers: classes for saving/exporting
 * a corpus. Subclass per file format.
 */
class CorpusWriter
{
  public:
    /** Write item with specified name and (XML) content to corpus. */
    virtual void write(QString const &name, QString const &content) = 0;

    /** Write entire corpus to file. */
    virtual void write(CorpusReader &corpus) = 0;
};

}   // namespace alpinocorpus

#endif  // ALPINO_CORPUSWRITER_HH
