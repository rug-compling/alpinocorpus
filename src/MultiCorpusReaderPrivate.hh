#include <list>
#include <map>
#include <string>
#include <utility>

#include <boost/filesystem.hpp>
#include <boost/tr1/memory.hpp>
#include <boost/tr1/unordered_map.hpp>

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/Entry.hh>
#include <AlpinoCorpus/IterImpl.hh>

#include "util/NameCompare.hh"

namespace alpinocorpus {

class MultiCorpusReaderPrivate : public CorpusReader
{
public:
struct ReaderIter
{
  ReaderIter(std::string newName, std::string newFilename, bool newRecursive) :
    name(newName), filename(newFilename), recursive(newRecursive) {}

  std::string name;
  std::string filename;
  bool recursive;
};
private:
  typedef std::map<std::string, std::pair<std::string, bool>, NameCompare> Corpora;

  class MultiIter : public IterImpl
  {
  public:
    MultiIter(Corpora const &corpora);
    MultiIter(Corpora const &corpora,
      std::string const &query);
    ~MultiIter();
    IterImpl *copy() const;
    void nextIterator();
    bool hasNext();
    bool hasProgress();
    void interrupt();
    Entry next(CorpusReader const &rdr);
    double progress();
  private:
    void openTip();

    size_t d_totalIters;
    std::list<ReaderIter> d_iters;
    std::tr1::shared_ptr<CorpusReader> d_currentReader;
    std::tr1::shared_ptr<CorpusReader::EntryIterator> d_currentIter;
    std::string d_currentName;
    bool d_hasQuery;
    std::string d_query;
    bool d_interrupted;
  };

public:
  MultiCorpusReaderPrivate();
  virtual ~MultiCorpusReaderPrivate();

  EntryIterator getEntries() const;
  std::string getName() const;
  size_t getSize() const;
  void push_back(std::string const &name, std::string const &filename,
      bool recursive = false);
  std::string readEntry(std::string const &) const;
  std::string readEntryMarkQueries(std::string const &entry, std::list<MarkerQuery> const &queries) const;
  EntryIterator runXPath(std::string const &query) const;

private:
  std::pair<std::string, bool> corpusFromPath(std::string const &path) const;
  std::string entryFromPath(std::string const &path) const;

  boost::filesystem::path d_directory;
  std::list<std::pair<std::string, bool> > d_corpora;
  Corpora d_corporaMap;
};

}
