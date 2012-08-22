#include <list>
#include <map>
#include <stdexcept>
#include <string>
#include <utility>

#include <boost/tr1/memory.hpp>
#include <boost/tr1/unordered_map.hpp>

#include <boost/filesystem.hpp>

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/CorpusReaderFactory.hh>
#include <AlpinoCorpus/Entry.hh>
#include <AlpinoCorpus/Error.hh>
#include <AlpinoCorpus/IterImpl.hh>
#include <AlpinoCorpus/RecursiveCorpusReader.hh>

#include "util/NameCompare.hh"

namespace bf = boost::filesystem;

namespace alpinocorpus {

struct ReaderIter
{
  ReaderIter(std::string newName, std::string newFilename) :
    name(newName), filename(newFilename) {}

  std::string name;
  std::string filename;
};

/*
bool operator==(ReaderIter const &left, ReaderIter const &right)
{
  return left.name == right.name && left.reader == right.reader &&
    left.iter == right.iter;
}
*/

class RecursiveCorpusReaderPrivate : public CorpusReader
{
  typedef std::map<std::string, std::string, NameCompare> Corpora;
  class RecursiveIter : public IterImpl
  {
  public:
    RecursiveIter(Corpora const &readers);
    RecursiveIter(Corpora const &readers,
      std::string const &query);
    ~RecursiveIter();
    IterImpl *copy() const;
    void nextIterator();
    bool hasNext();
    Entry next(CorpusReader const &rdr);
  private:
      void openTip();

      std::list<ReaderIter> d_iters;
      std::tr1::shared_ptr<CorpusReader> d_currentReader;
      std::tr1::shared_ptr<CorpusReader::EntryIterator> d_currentIter;
      std::string d_currentName;
      bool d_hasQuery;
      std::string d_query;
  };
public:
  RecursiveCorpusReaderPrivate(std::string const &directory);
  virtual ~RecursiveCorpusReaderPrivate();

  EntryIterator getEntries() const;
  std::string getName() const;
  size_t getSize() const;
  void push_back(std::string const &name, std::string const &filename);
  std::string readEntry(std::string const &) const;
  std::string readEntryMarkQueries(std::string const &entry, std::list<MarkerQuery> const &queries) const;
  EntryIterator runXPath(std::string const &query) const;
  bool validQuery(QueryDialect d, bool variables, std::string const &query) const;

private:
  std::string corpusFromPath(std::string const &path) const;
  std::string entryFromPath(std::string const &path) const;

  bf::path d_directory;
  std::list<std::string> d_corpora;
  Corpora d_corporaMap;
};


// Implementation of the public interface.
RecursiveCorpusReader::RecursiveCorpusReader(std::string const &directory) :
  d_private(new RecursiveCorpusReaderPrivate(directory))
{
}

RecursiveCorpusReader::~RecursiveCorpusReader()
{
  delete d_private;
}

CorpusReader::EntryIterator RecursiveCorpusReader::getEntries() const
{
  return d_private->getEntries();
}

std::string RecursiveCorpusReader::getName() const
{
  return d_private->getName();
}

size_t RecursiveCorpusReader::getSize() const
{
  return d_private->getSize();
}
    
bool RecursiveCorpusReader::validQuery(QueryDialect d, bool variables, std::string const &query) const
{
  return d_private->isValidQuery(d, variables, query);
}

std::string RecursiveCorpusReader::readEntry(std::string const &entry) const
{
  return d_private->readEntry(entry);
}
    
std::string RecursiveCorpusReader::readEntryMarkQueries(std::string const &entry, 
    std::list<MarkerQuery> const &queries) const
{
  return d_private->readEntryMarkQueries(entry, queries);
}

CorpusReader::EntryIterator RecursiveCorpusReader::runXPath(std::string const &query) const
{
  return d_private->query(XPATH, query);
}

// Implementation of the private interface

RecursiveCorpusReaderPrivate::RecursiveCorpusReaderPrivate(std::string const &directory)
{
  if (directory[directory.size() - 1] == '/')
    d_directory = bf::path(directory).parent_path();
  else
    d_directory = bf::path(directory);
  
  if (!bf::exists(d_directory) ||
    !bf::is_directory(d_directory))
    throw OpenError(directory, "non-existent or not a directory");

  for (bf::recursive_directory_iterator iter(d_directory, bf::symlink_option::recurse);
       iter != bf::recursive_directory_iterator();
       ++iter)
  {
    if (iter->path().extension() != ".dact" &&
        iter->path().extension() != ".index")
      continue;

    bf::path namePath = iter->path();
    namePath.replace_extension("");
    std::string name = namePath.string();

    name.erase(0, d_directory.string().size() + 1);
    
    push_back(name, iter->path().string());
  }
}

RecursiveCorpusReaderPrivate::~RecursiveCorpusReaderPrivate()
{
}

CorpusReader::EntryIterator RecursiveCorpusReaderPrivate::getEntries() const
{
  return EntryIterator(new RecursiveIter(d_corporaMap));
}

std::string RecursiveCorpusReaderPrivate::getName() const
{
  return "<multi>";
}

size_t RecursiveCorpusReaderPrivate::getSize() const
{
  size_t size = 0;

  for (std::list<std::string>::const_iterator iter =
      d_corpora.begin(); iter != d_corpora.end(); ++iter)
  {
      CorpusReader *reader;
      try {
          reader = CorpusReaderFactory::open(*iter);
      } catch (OpenError const &)
      {
        // XXX - Print a warning?
        continue;
      }
      size += reader->size();
      delete reader;
  }

  return size;
}

void RecursiveCorpusReaderPrivate::push_back(std::string const &name,
    std::string const &filename)
{
  // Ignore empty corpus readers, simplifies assumptions.
  /*if (reader->size() == 0) {
    delete reader;
    return;
  }
  */

  d_corpora.push_back(filename);
  d_corporaMap[name] = filename; // XXX - exists check?
}

std::string RecursiveCorpusReaderPrivate::corpusFromPath(
    std::string const &path) const
{
  for (Corpora::const_iterator iter =
      d_corporaMap.begin(); iter != d_corporaMap.end(); ++iter)
    if (path.find(iter->first) == 0)
      return iter->second;
  
  throw std::runtime_error(std::string("Unknown corpus: " + path));
}

std::string RecursiveCorpusReaderPrivate::entryFromPath(
    std::string const &path) const
{
  for (Corpora::const_iterator iter =
      d_corporaMap.begin(); iter != d_corporaMap.end(); ++iter)
    if (path.find(iter->first) == 0)
      return path.substr(iter->first.size() + 1);

  throw std::runtime_error(std::string("Could not find entry: " + path));
}

std::string RecursiveCorpusReaderPrivate::readEntry(std::string const &path) const
{
  std::string fn = corpusFromPath(path);
  CorpusReader *reader = CorpusReaderFactory::open(fn);
  std::string data = reader->read(entryFromPath(path));
  delete reader;
  return data;
}

std::string RecursiveCorpusReaderPrivate::readEntryMarkQueries(
    std::string const &path, std::list<MarkerQuery> const &queries) const
{
  std::string fn = corpusFromPath(path);
  CorpusReader *reader = CorpusReaderFactory::open(fn);
  std::string data = reader->read(entryFromPath(path), queries);
  delete reader;
  return data;
}

CorpusReader::EntryIterator RecursiveCorpusReaderPrivate::runXPath(
    std::string const &query) const
{
  return EntryIterator(new RecursiveIter(d_corporaMap, query));
}

bool RecursiveCorpusReaderPrivate::validQuery(QueryDialect d, bool variables, std::string const &query) const
{
  if (d_corpora.size() == 0)
    return false;

  // Only check using the first reader, otherwise, this is too expensive.
  std::string fn = d_corpora.front();
  CorpusReader *reader = CorpusReaderFactory::open(fn);
  bool valid = reader->isValidQuery(d, variables, query);
  delete reader;

  return valid;
}

// Iteration over RecursiveCorpusReaders

RecursiveCorpusReaderPrivate::RecursiveIter::RecursiveIter(
  Corpora const &corpora) : d_hasQuery(false)
{
  for (Corpora::const_iterator
      iter = corpora.begin();
      iter != corpora.end(); ++iter)
    d_iters.push_back(ReaderIter(iter->first, iter->second));
}

RecursiveCorpusReaderPrivate::RecursiveIter::RecursiveIter(
  Corpora const &readers,
  std::string const &query) : d_hasQuery(true)
{
  for (Corpora::const_iterator
      iter = readers.begin();
      iter != readers.end(); ++iter)
    d_iters.push_back(ReaderIter(iter->first, iter->second));

    d_query = query;
}

RecursiveCorpusReaderPrivate::RecursiveIter::~RecursiveIter() {}

IterImpl *RecursiveCorpusReaderPrivate::RecursiveIter::copy() const
{
  // No pointer members, pointer member of ReaderIter is not managed by
  // ReaderIter.
  return new RecursiveIter(*this);
}

bool RecursiveCorpusReaderPrivate::RecursiveIter::hasNext()
{
    nextIterator();
    return d_currentIter && d_currentIter->hasNext();
}

Entry RecursiveCorpusReaderPrivate::RecursiveIter::next(CorpusReader const &rdr)
{
    Entry e = d_currentIter->next(rdr);
    e.name = d_currentName + "/" + e.name;

    return e;
}

void RecursiveCorpusReaderPrivate::RecursiveIter::nextIterator()
{
  while (d_iters.size() != 0 &&
    (!d_currentIter || !d_currentIter->hasNext()))
  {
    d_currentIter.reset();
    d_currentReader.reset();
    openTip();
    d_iters.pop_front();
  }
}

void RecursiveCorpusReaderPrivate::RecursiveIter::openTip()
{
    CorpusReader *reader;
    try {
        reader = CorpusReaderFactory::open(d_iters.front().filename);
    } catch (OpenError const &e)
    {
        // XXX - print warning?
        return;
    }

    try {
      if (d_hasQuery)
        d_currentIter.reset(new EntryIterator(reader->query(CorpusReader::XPATH, d_query)));
      else
        d_currentIter.reset(new EntryIterator(reader->entries()));
    } catch (std::runtime_error &e)
    {
      delete reader;
      return;
    }

    d_currentReader.reset(reader);
    d_currentName = d_iters.front().name;
}

}

