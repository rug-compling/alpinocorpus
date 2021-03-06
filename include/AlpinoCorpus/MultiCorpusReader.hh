#ifndef MULTI_CORPUSREADER_HH
#define MULTI_CORPUSREADER_HH

#include <string>

#include <AlpinoCorpus/CorpusReader.hh>

namespace alpinocorpus {

class MultiCorpusReaderPrivate;

class ALPINO_CORPUS_EXPORT MultiCorpusReader : public CorpusReader
{
public:
  MultiCorpusReader();
  virtual ~MultiCorpusReader();
  void push_back(std::string const &name, std::string const &filename,
      bool recursive);
private:
  EntryIterator getEntries(SortOrder sortOrder) const;
  std::string getName() const;
  size_t getSize() const;
  std::string readEntry(std::string const &) const;
  std::string readEntryMarkQueries(std::string const &entry, std::list<MarkerQuery> const &queries) const;
  EntryIterator runXPath(std::string const &query, SortOrder sortOrder) const;
  EntryIterator runXQuery(std::string const &query, SortOrder sortOrder) const;
  Either<std::string, Empty> validQuery(QueryDialect d, bool variables, std::string const &query) const;

  MultiCorpusReaderPrivate *d_private;
};

}

#endif // MULTI_CORPUSREADER_HH
