#ifndef ALPINO_DBCORPUSWRITER_HH
#define ALPINO_DBCORPUSWRITER_HH

#include <AlpinoCorpus/CorpusWriter.hh>
#include <dbxml/DbXml.hpp>

namespace alpinocorpus {

/**
 * Corpus writer for DB XML-based file format.
 * See <AlpinoCorpus/DbCorpusReader.hh> for file format.
 */
class DbCorpusWriter
{
    DbXml::XmlManager mgr;
    DbXml::XmlContainer container;

  public:
    /** Open path for writing. */
    DbCorpusWriter(QString const &path, bool overwrite);

    /**
     * Will write name as a portable (Unix, UTF-8) pathname.
     */
    void write(QString const &name, QString const &content);

    /**
     * Write the contents of an entire CorpusReader.
     * If the second argument is false, fails with an alpinocorpus::Error
     * on the first error. Else, keeps writing till completion but may throw
     * an alpinocorpus::BatchError.
     *
     * @bug Weakly exception-safe: does not clean up in fail-first mode.
     */
    void write(CorpusReader const &corpus, bool failsafe = false);

  private:
    DbXml::XmlUpdateContext &mkUpdateContext(DbXml::XmlUpdateContext &);
    void write(QString const &, QString const &, DbXml::XmlUpdateContext &);
    void writeFailFirst(CorpusReader const &, DbXml::XmlUpdateContext &);
    void writeFailSafe(CorpusReader const &, DbXml::XmlUpdateContext &);
};

}   // namespace alpinocorpus

#endif  // ALPINO_DBCORPUSWRITER_HH
