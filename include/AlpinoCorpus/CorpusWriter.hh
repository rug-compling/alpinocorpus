#ifndef ALPINO_CORPUSWRITER_HH
#define ALPINO_CORPUSWRITER_HH

namespace alpinocorpus {

/**
 * Abstract base class for corpus writers: classes for saving/exporting
 * a corpus. Subclass per file format.
 */
class CorpusWriter
{
  public:
    /** Write item (file) name with content to corpus. */
    virtual void write(QString const &name, QString const &content) = 0;
};

}   // namespace alpinocorpus

#endif  // ALPINO_CORPUSWRITER_HH
