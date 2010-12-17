#ifndef ALPINO_DBCORPUSWRITER_HH
#define ALPINO_DBCORPUSWRITER_HH

#include <AlpinoCorpus/CorpusWriter.hh>
#include <dbxml/DbXml.hpp>

namespace alpinocorpus {

/**
 * Corpus writer for DB XML-based file format.
 * See <AlpinoCorpus/DbCorpusReader> for file format.
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

    void write(CorpusReader const &corpus);

  private:
    DbXml::XmlUpdateContext &mkUpdateContext(DbXml::XmlUpdateContext &);
    void write(QString const &, QString const &, DbXml::XmlUpdateContext &);
};

}   // namespace alpinocorpus

#endif  // ALPINO_DBCORPUSWRITER_HH
