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
    /**
     * Open path for writing. If the overwrite flag is set to
     * <tt>true</tt>, the corpus is removed before it is opened.
     * If this flag is set to <tt>false</tt>, the corpus will not
     * be removed, however entries that already exist are overwritten.
     */
    DbCorpusWriter(std::string const &path, bool overwrite);
    virtual ~DbCorpusWriter();

  private:
    /**
     * Will write name as a portable (Unix, UTF-8) pathname.
     */
    virtual void writeEntry(std::string const &name, std::string const &content);
    
    /**
     * Write the contents of an entire CorpusReader.
     * If the second argument is false, fails with an alpinocorpus::Error
     * on the first error. Else, keeps writing till completion but may throw
     * an alpinocorpus::BatchError.
     *
     * @bug Weakly exception-safe: does not clean up in fail-first mode.
     */
    virtual void writeEntry(CorpusReader const &corpus, bool failsafe = false);

    DbCorpusWriterPrivate *d_private;
};

}   // namespace alpinocorpus

#endif  // ALPINO_DBCORPUSWRITER_HH
