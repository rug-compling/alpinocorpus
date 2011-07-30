#ifndef ALPINO_DBCORPUSREADER_HH
#define ALPINO_DBCORPUSREADER_HH

#include <string>
#include <tr1/memory>

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
    QString readEntry(std::string const &) const;
    QString readEntryMarkQueries(std::string const &entry, QList<MarkerQuery> const &queries) const;
    EntryIterator runXPath(std::string const &) const;
    EntryIterator runXQuery(std::string const &) const;
    size_t getSize() const;
    
    std::tr1::shared_ptr<DbCorpusReaderPrivate> d_private;
};

}   // namespace alpinocorpus

#endif
