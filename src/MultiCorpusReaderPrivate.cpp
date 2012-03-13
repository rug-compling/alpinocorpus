#include <list>
#include <stdexcept>
#include <string>
#include <utility>

#include <boost/tr1/unordered_map.hpp>

#include <boost/filesystem.hpp>

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/Error.hh>

#include "MultiCorpusReaderPrivate.hh"

namespace bf = boost::filesystem;

namespace alpinocorpus {

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
  // XXX - Constructing an empty map in the argument of the constructor
  // breaks with Boost 1.48.0. See ticket 6167.
  std::tr1::unordered_map<std::string, CorpusReader *> emptyMap;
  return EntryIterator(new MultiIter(emptyMap));
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

  // If we have a query for which none of the corpora has a matching result,
  // then the iterator is in fact an end-iterator. We just don't know it yet,
  // unless we attempt to move the iterator.
  nextIterator();
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

  // If we have a query for which none of the corpora has a matching result,
  // then the iterator is in fact an end-iterator. We just don't know it yet,
  // unless we attempt to move the iterator.
  nextIterator();
}

MultiCorpusReaderPrivate::MultiIter::~MultiIter() {}

CorpusReader::IterImpl *MultiCorpusReaderPrivate::MultiIter::copy() const
{
  // No pointer members, and pointer member in ReaderIter is not managed
  // by ReaderIter.
  return new MultiIter(*this);
}

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
  } catch (std::bad_cast const &) {
    return false;
  }
}

void MultiCorpusReaderPrivate::MultiIter::nextIterator()
{
  while (d_iters.size() != 0 &&
      d_iters.front().iter == d_iters.front().reader->end())
    d_iters.pop_front();
}

void MultiCorpusReaderPrivate::MultiIter::next()
{
  // If we are already in an end-state, attempt to find the next usable
  // iterator.
  if (d_iters.front().iter == d_iters.front().reader->end())
    nextIterator();
  // If we arrive at the end of the current iterator, find the next usable
  // iterator.
  else if (++d_iters.front().iter == d_iters.front().reader->end())
    nextIterator();
}

}
