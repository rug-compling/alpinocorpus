#include <list>
#include <map>
#include <stdexcept>
#include <string>
#include <utility>

#include <boost/tr1/unordered_map.hpp>

#include <boost/filesystem.hpp>

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/CorpusReaderFactory.hh>
#include <AlpinoCorpus/Error.hh>
#include <AlpinoCorpus/IterImpl.hh>
#include <AlpinoCorpus/RecursiveCorpusReader.hh>

#include "util/NameCompare.hh"

namespace bf = boost::filesystem;

namespace alpinocorpus {

struct ReaderIter
{
  ReaderIter(std::string newName, CorpusReader *newReader,
      CorpusReader::EntryIterator newIter) :
    name(newName), reader(newReader), iter(newIter) {}

  std::string name;
  CorpusReader *reader;
  CorpusReader::EntryIterator iter;
};

bool operator==(ReaderIter const &left, ReaderIter const &right)
{
  return left.name == right.name && left.reader == right.reader &&
    left.iter == right.iter;
}

class RecursiveCorpusReaderPrivate : public CorpusReader
{
  typedef std::map<std::string, CorpusReader *, NameCompare> CorpusReaders;
  class RecursiveIter : public IterImpl
  {
  public:
    RecursiveIter(CorpusReaders const &readers);
    RecursiveIter(CorpusReaders const &readers,
      std::string const &query);
    ~RecursiveIter();
    std::string contents(CorpusReader const &) const;
    IterImpl *copy() const;
    std::string current() const;
    bool equals(IterImpl const &other) const;
    void next();
  private:

    std::list<ReaderIter> d_iters;
  };
public:
  RecursiveCorpusReaderPrivate(std::string const &directory);
  virtual ~RecursiveCorpusReaderPrivate();

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

  bf::path d_directory;
  std::list<CorpusReader *> d_corpusReaders;
  CorpusReaders d_corpusReaderMap;
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

CorpusReader::EntryIterator RecursiveCorpusReader::getBegin() const
{
  return d_private->getBegin();
}

CorpusReader::EntryIterator RecursiveCorpusReader::getEnd() const
{
  return d_private->getEnd();
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

    CorpusReader *reader(CorpusReaderFactory::open(iter->path().string()));

    bf::path namePath = iter->path();
    namePath.replace_extension("");
    std::string name = namePath.string();

    name.erase(0, d_directory.string().size() + 1);
    
    push_back(name, reader);
  }
}

RecursiveCorpusReaderPrivate::~RecursiveCorpusReaderPrivate()
{
  for (std::list<CorpusReader *>::iterator iter = d_corpusReaders.begin();
      iter != d_corpusReaders.end(); ++iter)
    delete *iter;
}

CorpusReader::EntryIterator RecursiveCorpusReaderPrivate::getBegin() const
{
  return EntryIterator(new RecursiveIter(d_corpusReaderMap));
}

CorpusReader::EntryIterator RecursiveCorpusReaderPrivate::getEnd() const
{
  // XXX - Constructing an empty map in the argument of the constructor
  // breaks with Boost 1.48.0. See ticket 6167.
  CorpusReaders emptyMap;
  return EntryIterator(new RecursiveIter(emptyMap));
}

std::string RecursiveCorpusReaderPrivate::getName() const
{
  return "<multi>";
}

size_t RecursiveCorpusReaderPrivate::getSize() const
{
  size_t size = 0;

  for (std::list<CorpusReader *>::const_iterator iter =
      d_corpusReaders.begin(); iter != d_corpusReaders.end(); ++iter)
    size += (*iter)->size();

  return size;
}

void RecursiveCorpusReaderPrivate::push_back(std::string const &name,
    CorpusReader *reader)
{
  // Ignore empty corpus readers, simplifies assumptions.
  if (reader->size() == 0) {
    delete reader;
    return;
  }

  d_corpusReaders.push_back(reader);
  d_corpusReaderMap[name] = reader; // XXX - exists check?
}

CorpusReader const *RecursiveCorpusReaderPrivate::corpusReaderFromPath(
    std::string const &path) const
{
  for (CorpusReaders::const_iterator iter =
      d_corpusReaderMap.begin(); iter != d_corpusReaderMap.end(); ++iter)
    if (path.find(iter->first) == 0)
      return iter->second;
  
  throw std::runtime_error(std::string("Unknown corpus: " + path));
}

std::string RecursiveCorpusReaderPrivate::entryFromPath(
    std::string const &path) const
{
  for (CorpusReaders::const_iterator iter =
      d_corpusReaderMap.begin(); iter != d_corpusReaderMap.end(); ++iter)
    if (path.find(iter->first) == 0)
      return path.substr(iter->first.size() + 1);

  throw std::runtime_error(std::string("Could not find entry: " + path));
}

std::string RecursiveCorpusReaderPrivate::readEntry(std::string const &path) const
{
  CorpusReader const *reader = corpusReaderFromPath(path);
  return reader->read(entryFromPath(path));
}

std::string RecursiveCorpusReaderPrivate::readEntryMarkQueries(
    std::string const &path, std::list<MarkerQuery> const &queries) const
{
  CorpusReader const *reader = corpusReaderFromPath(path);
  return reader->read(entryFromPath(path), queries);
}

CorpusReader::EntryIterator RecursiveCorpusReaderPrivate::runXPath(
    std::string const &query) const
{
  return EntryIterator(new RecursiveIter(d_corpusReaderMap, query));
}

bool RecursiveCorpusReaderPrivate::validQuery(QueryDialect d, bool variables, std::string const &query) const
{
  if (d_corpusReaders.size() == 0)
    return false;

  for (std::list<CorpusReader *>::const_iterator iter = d_corpusReaders.begin();
      iter != d_corpusReaders.end(); ++iter)
    if (!(*iter)->isValidQuery(d, variables, query))
      return false;

  // Correct according to all corpus readers.
  return true;
}

// Iteration over RecursiveCorpusReaders

RecursiveCorpusReaderPrivate::RecursiveIter::RecursiveIter(
  CorpusReaders const &readers)
{
  for (CorpusReaders::const_iterator
      iter = readers.begin();
      iter != readers.end(); ++iter)
    d_iters.push_back(ReaderIter(iter->first, iter->second,
          (iter->second->begin())));
}

RecursiveCorpusReaderPrivate::RecursiveIter::RecursiveIter(
  CorpusReaders const &readers,
  std::string const &query)
{
  for (CorpusReaders::const_iterator
      iter = readers.begin();
      iter != readers.end(); ++iter)
    d_iters.push_back(ReaderIter(iter->first, iter->second,
          (iter->second->query(XPATH, query))));
}

RecursiveCorpusReaderPrivate::RecursiveIter::~RecursiveIter() {}

std::string RecursiveCorpusReaderPrivate::RecursiveIter::contents(
  CorpusReader const &reader) const
{
  if (d_iters.size() == 0)
    throw std::runtime_error("Cannot dereference an end iterator!");

  return d_iters.front().iter.contents(reader);
}

IterImpl *RecursiveCorpusReaderPrivate::RecursiveIter::copy() const
{
  // No pointer members, pointer member of ReaderIter is not managed by
  // ReaderIter.
  return new RecursiveIter(*this);
}

std::string RecursiveCorpusReaderPrivate::RecursiveIter::current() const
{
  if (d_iters.size() == 0)
    throw std::runtime_error("Cannot dereference an end iterator!");

  return d_iters.front().name + "/" + *d_iters.front().iter;
}

bool RecursiveCorpusReaderPrivate::RecursiveIter::equals(IterImpl const &other) const
{
  try {
    RecursiveIter &that = const_cast<RecursiveIter &>(dynamic_cast<RecursiveIter const&>(other));
    return that.d_iters == d_iters;
  } catch (std::bad_cast const &) {
    return false;
  }
}

void RecursiveCorpusReaderPrivate::RecursiveIter::next() {
  // Iteration is done. Yay.
  if (d_iters.size() == 0)
    return;

  // Move the iterator over the current corpus .
  if (++d_iters.front().iter != d_iters.front().reader->end())
    return;

  // Ok, we are at the end of the current corpus, so we'll remove the
  // corpus from the iteration list, and get started on the next corpus.
  d_iters.pop_front();
}

}

