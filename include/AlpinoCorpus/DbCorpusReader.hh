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

    class DbIter : public CorpusReader::IterImpl
    {
        QString cur;
        DbXml::XmlResults r;

      public:
        DbIter(DbXml::XmlContainer &);
        DbIter(DbXml::XmlManager &);

        QString const &current() const;
        bool equals(IterImpl const *) const;
        void next();
        void prev();
    };

  public:
    DbCorpusReader(QString const &);
    ~DbCorpusReader();
    EntryIterator begin() const;
    EntryIterator end() const;
    QVector<QString> entries() const;
    QString name() const;
    QString read(QString const &);

    size_t size() const
    {
        return const_cast<DbXml::XmlContainer &>(container).getNumDocuments();
    }
};

}   // namespace alpinocorpus

#endif
