#ifndef RECURSIVE_CORPUSREADER_HH
#define RECURSIVE_CORPUSREADER_HH

#include <string>

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/util/Either.hh>

namespace alpinocorpus {

class RecursiveCorpusReaderPrivate;

/**
 * Read a directory of Dact or compact corpora
 */
class RecursiveCorpusReader : public CorpusReader
{
public:
  /**
   * Construct a recursive CorpusReader. If <i>dactOnly</i> is
   * <tt>false</tt>, compact corpora are also opened. <b>Warning:</b>
   * RecursiveCorpusReader will always use DBXML for query validation.
   */
  RecursiveCorpusReader(std::string const &directory, bool dactOnly = true);
  virtual ~RecursiveCorpusReader();
private:
  EntryIterator getEntries() const;
  std::string getName() const;
  size_t getSize() const;
  std::string readEntry(std::string const &) const;
  std::string readEntryMarkQueries(std::string const &entry, std::list<MarkerQuery> const &queries) const;
  EntryIterator runXPath(std::string const &query) const;
  EntryIterator runXQuery(std::string const &query) const;
  Either<std::string, Empty> validQuery(QueryDialect d, bool variables, std::string const &query) const;

  RecursiveCorpusReaderPrivate *d_private;
};

}

#endif // RECURSIVE_CORPUSREADER_HH
