#ifndef ALPINO_DBCORPUSREADER_HH
#define ALPINO_DBCORPUSREADER_HH

#include <AlpinoCorpus/CorpusReader.hh>
#include <dbxml/DbXml.hpp>
#include <QString>

namespace alpinocorpus {

/**
 * Corpus reader for Berkeley DB XML-based corpora.
 *
 * DBXML corpora store both names and contents as UTF-8.
 */
class DbCorpusReader : public CorpusReader
{
    DbXml::XmlManager mgr;
    DbXml::XmlContainer container;

  public:
    DbCorpusReader(QString const &);
    ~DbCorpusReader();
    EntryIterator begin() const;
    EntryIterator end() const;
    QString name() const;
    QString read(QString const &);
    QVector<QString> entries() const;
};

}   // namespace alpinocorpus

#endif
