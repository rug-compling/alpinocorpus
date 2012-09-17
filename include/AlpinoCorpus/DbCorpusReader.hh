#ifndef ALPINO_DBCORPUSREADER_HH
#define ALPINO_DBCORPUSREADER_HH

#include <list>
#include <string>

#include <AlpinoCorpus/CorpusReader.hh>

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
    DbCorpusReader(std::string const &);
    virtual ~DbCorpusReader();

  private:
    bool validQuery(QueryDialect d, bool variables, std::string const &query) const;
    EntryIterator getEntries() const;
    std::string getName() const;
    std::string readEntry(std::string const &) const;
    EntryIterator runXPath(std::string const &) const;
    EntryIterator runXQuery(std::string const &) const;
    size_t getSize() const;
    
    DbCorpusReaderPrivate *d_private;
};

}   // namespace alpinocorpus

#endif
