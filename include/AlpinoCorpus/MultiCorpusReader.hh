#ifndef MULTI_CORPUSREADER_HH
#define MULTI_CORPUSREADER_HH

#include <string>
#include <tr1/memory>

#include <AlpinoCorpus/CorpusReader.hh>

namespace alpinocorpus {

class MultiCorpusReaderPrivate;

class MultiCorpusReader : public CorpusReader
{
public:
  MultiCorpusReader(std::string const &directory);
  virtual ~MultiCorpusReader();
private:
  EntryIterator getBegin() const;
  EntryIterator getEnd() const;
  std::string getName() const;
  size_t getSize() const;
  std::string readEntry(std::string const &) const;
  std::string readEntryMarkQueries(std::string const &entry, std::list<MarkerQuery> const &queries) const;
  EntryIterator runXPath(std::string const &query) const;
  bool validQuery(QueryDialect d, bool variables, std::string const &query) const;

  std::tr1::shared_ptr<MultiCorpusReaderPrivate> d_private;
};

}

#endif // MULTI_CORPUSREADER_HH
