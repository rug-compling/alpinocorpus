#ifndef ALPINO_DBCORPUSWRITER_HH
#define ALPINO_DBCORPUSWRITER_HH

#include <AlpinoCorpus/CorpusWriter.hpp>
#include <dbxml/DbXml.hpp>

namespace alpinocorpus {

/**
 * Corpus writer for DB XML-based file format.
 * See <AlpinoCorpus/DbCorpusReader> for file format.
 */
class DbCorpusWriter
{
  public:
    /** Open path for writing. */
    DbCorpusWriter(QString const &path);

    /** Write item (file) with specified name and (XML) content to corpus. */
    virtual void write(QString const &name, QString const &content) = 0;
};

}   // namespace alpinocorpus

#endif  ALPINO_DBCORPUSWRITER_HH
