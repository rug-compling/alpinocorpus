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
    // XXX mutable is hideous, but saves a lot of const_casts: the read
    // methods are nominally const (don't change future behavior and are
    // thread-safe), but DB XML doesn't expose const reading methods.
    DbXml::XmlManager   mutable mgr;
    DbXml::XmlContainer mutable container;
    std::string collection;

    class DbIter : public CorpusReader::IterImpl
    {
        DbXml::XmlResults r;

      public:
        DbIter(DbXml::XmlContainer &);
        DbIter(DbXml::XmlManager &);
        DbIter(DbXml::XmlResults const &);

        QString current() const;
        bool equals(IterImpl const *) const;
        void next();
    };

  public:
    DbCorpusReader(QString const &);
    ~DbCorpusReader();

  private:
    void setNameAndCollection(QString const &);

    EntryIterator getBegin() const;
    EntryIterator getEnd() const;
    QString readEntry(QString const &) const;
    EntryIterator runXPath(QString const &) const;
    EntryIterator runXQuery(QString const &) const;

    size_t getSize() const
    {
        return const_cast<DbXml::XmlContainer &>(container).getNumDocuments();
    }
};

}   // namespace alpinocorpus

#endif
