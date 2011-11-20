#ifndef ALPINO_DBCORPUSWRITER_HH
#define ALPINO_DBCORPUSWRITER_HH

#include <string>

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/CorpusWriter.hh>

#include <AlpinoCorpus/DLLDefines.hh>

namespace alpinocorpus {

class DbCorpusWriterPrivate;
    
/**
 * Corpus writer for DB XML-based file format.
 * See <AlpinoCorpus/DbCorpusReader.hh> for file format.
 */
class ALPINO_CORPUS_EXPORT DbCorpusWriter : public CorpusWriter
{
  public:
    /** Open path for writing. */
    DbCorpusWriter(std::string const &path, bool overwrite);
    ~DbCorpusWriter();

  private:
    /**
     * Will write name as a portable (Unix, UTF-8) pathname.
     */
    void writeEntry(std::string const &name, std::string const &content);
    
    /**
     * Write the contents of an entire CorpusReader.
     * If the second argument is false, fails with an alpinocorpus::Error
     * on the first error. Else, keeps writing till completion but may throw
     * an alpinocorpus::BatchError.
     *
     * @bug Weakly exception-safe: does not clean up in fail-first mode.
     */
    void writeEntry(CorpusReader const &corpus, bool failsafe = false);

    DbCorpusWriterPrivate *d_private;
};

}   // namespace alpinocorpus

#endif  // ALPINO_DBCORPUSWRITER_HH
