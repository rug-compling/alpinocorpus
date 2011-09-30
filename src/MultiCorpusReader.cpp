#include <list>
#include <stdexcept>
#include <string>
#include <utility>

#include <tr1/unordered_map>

#include <boost/filesystem.hpp>

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/Error.hh>
#include <AlpinoCorpus/MultiCorpusReader.hh>

#include <iostream>

namespace bf = boost::filesystem;

namespace {

struct ReaderIter
{
  ReaderIter(std::string newName, alpinocorpus::CorpusReader *newReader,
      alpinocorpus::CorpusReader::EntryIterator newIter) :
    name(newName), reader(newReader), iter(newIter) {}

  std::string name;
  alpinocorpus::CorpusReader *reader;
  alpinocorpus::CorpusReader::EntryIterator iter;
};

bool operator==(ReaderIter const &left, ReaderIter const &right)
{
  return left.name == right.name && left.reader == right.reader &&
    left.iter == right.iter;
}

}

namespace alpinocorpus {

class MultiCorpusReaderPrivate : public CorpusReader
{
  class MultiIter : public CorpusReader::IterImpl
  {
  public:
    MultiIter(std::tr1::unordered_map<std::string, CorpusReader *> const &readers);
    MultiIter(std::tr1::unordered_map<std::string, CorpusReader *> const &readers,
      std::string const &query);
    ~MultiIter();
    std::string contents(CorpusReader const &) const;
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

  bf::path d_directory;
  std::list<CorpusReader *> d_corpusReaders;
  std::tr1::unordered_map<std::string, CorpusReader *> d_corpusReaderMap;
};


// Implementation of the public interface.
MultiCorpusReader::MultiCorpusReader() :
  d_private(new MultiCorpusReaderPrivate())
{
}

MultiCorpusReader::~MultiCorpusReader()
{
}

CorpusReader::EntryIterator MultiCorpusReader::getBegin() const
{
  return d_private->getBegin();
}

CorpusReader::EntryIterator MultiCorpusReader::getEnd() const
{
  return d_private->getEnd();
}

std::string MultiCorpusReader::getName() const
{
  return d_private->getName();
}

size_t MultiCorpusReader::getSize() const
{
  return d_private->getSize();
}

void MultiCorpusReader::push_back(std::string const &name, CorpusReader *reader)
{
  d_private->push_back(name, reader);
}
    
bool MultiCorpusReader::validQuery(QueryDialect d, bool variables, std::string const &query) const
{
  return d_private->isValidQuery(d, variables, query);
}

std::string MultiCorpusReader::readEntry(std::string const &entry) const
{
  return d_private->readEntry(entry);
}
    
std::string MultiCorpusReader::readEntryMarkQueries(std::string const &entry, 
    std::list<MarkerQuery> const &queries) const
{
  return d_private->readEntryMarkQueries(entry, queries);
}

CorpusReader::EntryIterator MultiCorpusReader::runXPath(std::string const &query) const
{
  return d_private->query(XPATH, query);
}

// Implementation of the private interface

MultiCorpusReaderPrivate::MultiCorpusReaderPrivate()
{
}

MultiCorpusReaderPrivate::~MultiCorpusReaderPrivate()
{
  for (std::list<CorpusReader *>::iterator iter = d_corpusReaders.begin();
      iter != d_corpusReaders.end(); ++iter)
    delete *iter;
}

CorpusReader::EntryIterator MultiCorpusReaderPrivate::getBegin() const
{
  return EntryIterator(new MultiIter(d_corpusReaderMap));
}

CorpusReader::EntryIterator MultiCorpusReaderPrivate::getEnd() const
{
  return EntryIterator(new MultiIter(
    std::tr1::unordered_map<std::string, CorpusReader *>()));
}

std::string MultiCorpusReaderPrivate::getName() const
{
  return "<multi>";
}

size_t MultiCorpusReaderPrivate::getSize() const
{
  size_t size = 0;

  for (std::list<CorpusReader *>::const_iterator iter =
      d_corpusReaders.begin(); iter != d_corpusReaders.end(); ++iter)
    size += (*iter)->size();

  return size;
}

void MultiCorpusReaderPrivate::push_back(std::string const &name,
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

CorpusReader const *MultiCorpusReaderPrivate::corpusReaderFromPath(
    std::string const &path) const
{
  for (std::tr1::unordered_map<std::string, CorpusReader *>::const_iterator iter =
      d_corpusReaderMap.begin(); iter != d_corpusReaderMap.end(); ++iter)
    if (path.find(iter->first) == 0)
      return iter->second;
  
  throw std::runtime_error(std::string("Could not find corpus for: " + path));
}

std::string MultiCorpusReaderPrivate::entryFromPath(
    std::string const &path) const
{
  for (std::tr1::unordered_map<std::string, CorpusReader *>::const_iterator iter =
      d_corpusReaderMap.begin(); iter != d_corpusReaderMap.end(); ++iter)
    if (path.find(iter->first) == 0)
      return path.substr(iter->first.size() + 1);

  throw std::runtime_error(std::string("Could not find corpus for: " + path));
}

std::string MultiCorpusReaderPrivate::readEntry(std::string const &path) const
{
  CorpusReader const *reader = corpusReaderFromPath(path);
  return reader->read(entryFromPath(path));
}

std::string MultiCorpusReaderPrivate::readEntryMarkQueries(
    std::string const &path, std::list<MarkerQuery> const &queries) const
{
  CorpusReader const *reader = corpusReaderFromPath(path);
  return reader->readMarkQueries(entryFromPath(path), queries);
}

CorpusReader::EntryIterator MultiCorpusReaderPrivate::runXPath(
    std::string const &query) const
{
  return EntryIterator(new MultiIter(d_corpusReaderMap, query));
}

bool MultiCorpusReaderPrivate::validQuery(QueryDialect d, bool variables, std::string const &query) const
{
  if (d_corpusReaders.size() == 0)
    return false;

  for (std::list<CorpusReader *>::const_iterator iter = d_corpusReaders.begin();
      iter != d_corpusReaders.end(); ++iter)
    if (!(*iter)->isValidQuery(d, variables, query))
      return false;

  return true;
}

// Iteration over MultiCorpusReaders

MultiCorpusReaderPrivate::MultiIter::MultiIter(
  std::tr1::unordered_map<std::string, CorpusReader *> const &readers)
{
  for (std::tr1::unordered_map<std::string, CorpusReader *>::const_iterator
      iter = readers.begin();
      iter != readers.end(); ++iter)
    d_iters.push_back(ReaderIter(iter->first, iter->second,
          (iter->second->begin())));
}

MultiCorpusReaderPrivate::MultiIter::MultiIter(
  std::tr1::unordered_map<std::string, CorpusReader *> const &readers,
  std::string const &query)
{
  for (std::tr1::unordered_map<std::string, CorpusReader *>::const_iterator
      iter = readers.begin();
      iter != readers.end(); ++iter)
    d_iters.push_back(ReaderIter(iter->first, iter->second,
          (iter->second->query(XPATH, query))));
}

MultiCorpusReaderPrivate::MultiIter::~MultiIter() {}

std::string MultiCorpusReaderPrivate::MultiIter::current() const
{
  if (d_iters.size() == 0)
    throw std::runtime_error("Cannot dereference an end iterator!");

  return d_iters.front().name + "/" + *d_iters.front().iter;
}

std::string MultiCorpusReaderPrivate::MultiIter::contents(
  CorpusReader const &reader) const
{
  if (d_iters.size() == 0)
    throw std::runtime_error("Cannot dereference an end iterator!");

  return d_iters.front().iter.contents(reader);
}

bool MultiCorpusReaderPrivate::MultiIter::equals(IterImpl const &other) const
{
  try {
    MultiIter &that = const_cast<MultiIter &>(dynamic_cast<MultiIter const&>(other));
    return that.d_iters == d_iters;
  } catch (std::bad_cast const &e) {
    return false;
  }
}

void MultiCorpusReaderPrivate::MultiIter::next() {
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
