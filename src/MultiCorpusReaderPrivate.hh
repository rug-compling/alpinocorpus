#include <list>
#include <string>

#include <boost/filesystem.hpp>

#include <boost/tr1/unordered_map.hpp>

#include <AlpinoCorpus/CorpusReader.hh>

namespace alpinocorpus {

class MultiCorpusReaderPrivate : public CorpusReader
{
public:
  struct ReaderIter
  {
    ReaderIter(std::string newName, alpinocorpus::CorpusReader *newReader,
        alpinocorpus::CorpusReader::EntryIterator newIter) :
      name(newName), reader(newReader), iter(newIter) {}

    std::string name;
    alpinocorpus::CorpusReader *reader;
    alpinocorpus::CorpusReader::EntryIterator iter;
  };
private:
  class MultiIter : public CorpusReader::IterImpl
  {
  public:
    MultiIter(std::tr1::unordered_map<std::string, CorpusReader *> const &readers);
    MultiIter(std::tr1::unordered_map<std::string, CorpusReader *> const &readers,
      std::string const &query);
    ~MultiIter();
    std::string contents(CorpusReader const &) const;
    IterImpl *copy() const;
    std::string current() const;
    bool equals(IterImpl const &other) const;
    void next();
  private:
    std::list<ReaderIter> d_iters;
  };
public:
  MultiCorpusReaderPrivate();
  virtual ~MultiCorpusReaderPrivate();

  EntryIterator getBegin() const;
  EntryIterator getEnd() const;
  std::string getName() const;
  size_t getSize() const;
  void push_back(std::string const &name, CorpusReader *reader);
  std::string readEntry(std::string const &) const;
  std::string readEntryMarkQueries(std::string const &entry, std::list<MarkerQuery> const &queries) const;
  EntryIterator runXPath(std::string const &query) const;
  bool validQuery(QueryDialect d, bool variables, std::string const &query) const;

private:
  CorpusReader const *corpusReaderFromPath(std::string const &path) const;
  std::string entryFromPath(std::string const &path) const;

  boost::filesystem::path d_directory;
  std::list<CorpusReader *> d_corpusReaders;
  std::tr1::unordered_map<std::string, CorpusReader *> d_corpusReaderMap;
};

inline bool operator==(MultiCorpusReaderPrivate::ReaderIter const &left,
  MultiCorpusReaderPrivate::ReaderIter const &right)
{
  return left.name == right.name && left.reader == right.reader &&
    left.iter == right.iter;
}

}
