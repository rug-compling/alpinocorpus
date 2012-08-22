#ifndef RECURSIVE_CORPUSREADER_HH
#define RECURSIVE_CORPUSREADER_HH

#include <string>

#include <AlpinoCorpus/CorpusReader.hh>

namespace alpinocorpus {

class RecursiveCorpusReaderPrivate;

class RecursiveCorpusReader : public CorpusReader
{
public:
  RecursiveCorpusReader(std::string const &directory);
  virtual ~RecursiveCorpusReader();
private:
  EntryIterator getEntries() const;
  std::string getName() const;
  size_t getSize() const;
  std::string readEntry(std::string const &) const;
  std::string readEntryMarkQueries(std::string const &entry, std::list<MarkerQuery> const &queries) const;
  EntryIterator runXPath(std::string const &query) const;
  bool validQuery(QueryDialect d, bool variables, std::string const &query) const;

  RecursiveCorpusReaderPrivate *d_private;
};

}

#endif // RECURSIVE_CORPUSREADER_HH
