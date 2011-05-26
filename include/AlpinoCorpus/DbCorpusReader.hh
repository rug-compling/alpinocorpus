#ifndef ALPINO_DBCORPUSREADER_HH
#define ALPINO_DBCORPUSREADER_HH

#include <AlpinoCorpus/CorpusReader.hh>
#include <QString>

namespace alpinocorpus {

class DbCorpusReaderPrivate;
    
/**
 * Corpus reader for Berkeley DB XML-based corpora.
 *
 * DBXML corpora store both names and contents as UTF-8.
 */
class DbCorpusReader : public CorpusReader
{
  public:
    DbCorpusReader(QString const &);
    virtual ~DbCorpusReader();

  private:
    bool validQuery(QueryDialect d, bool variables, QString const &query) const;
    EntryIterator getBegin() const;
    EntryIterator getEnd() const;
    QString readEntry(QString const &) const;
    QString readEntryMarkQuery(QString const &entry, QueryDialect d, QString const &query,
        QString const &attr, QString const &value) const;
    EntryIterator runXPath(QString const &) const;
    EntryIterator runXQuery(QString const &) const;
    size_t getSize() const;
    
    QSharedPointer<DbCorpusReaderPrivate> d_private;
};

}   // namespace alpinocorpus

#endif
