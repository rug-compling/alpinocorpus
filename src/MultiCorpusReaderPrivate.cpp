#include <list>
#include <stdexcept>
#include <string>
#include <utility>

#include <boost/tr1/unordered_map.hpp>

#include <boost/filesystem.hpp>

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/CorpusReaderFactory.hh>
#include <AlpinoCorpus/Error.hh>
#include <AlpinoCorpus/IterImpl.hh>

#include "MultiCorpusReaderPrivate.hh"

namespace bf = boost::filesystem;

namespace alpinocorpus {

MultiCorpusReaderPrivate::MultiCorpusReaderPrivate()
{
}

MultiCorpusReaderPrivate::~MultiCorpusReaderPrivate()
{
}

CorpusReader::EntryIterator MultiCorpusReaderPrivate::getEntries() const
{
  return EntryIterator(new MultiIter(d_corporaMap));
}

std::string MultiCorpusReaderPrivate::getName() const
{
  return "<multi>";
}

size_t MultiCorpusReaderPrivate::getSize() const
{
  size_t size = 0;

  for (std::list<std::pair<std::string, bool> >::const_iterator iter =
      d_corpora.begin(); iter != d_corpora.end(); ++iter)
  {
      CorpusReader *reader;
      try {
          if (iter->second)
            reader = CorpusReaderFactory::openRecursive(iter->first);
          else
            reader = CorpusReaderFactory::open(iter->first);
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

void MultiCorpusReaderPrivate::push_back(std::string const &name,
    std::string const &filename, bool recursive)
{
  d_corpora.push_back(std::make_pair(filename, recursive));
  d_corporaMap[name] = std::make_pair(filename, recursive); // XXX - exists check?
}

std::pair<std::string, bool> MultiCorpusReaderPrivate::corpusFromPath(
    std::string const &path) const
{
  for (Corpora::const_iterator iter =
      d_corporaMap.begin(); iter != d_corporaMap.end(); ++iter)
    if (path.find(iter->first) == 0)
      return iter->second;
  
  throw std::runtime_error(std::string("Unknown corpus: " + path));
}

std::string MultiCorpusReaderPrivate::entryFromPath(
    std::string const &path) const
{
  for (Corpora::const_iterator iter =
      d_corporaMap.begin(); iter != d_corporaMap.end(); ++iter)
    if (path.find(iter->first) == 0)
      return path.substr(iter->first.size() + 1);

  throw std::runtime_error(std::string("Could not find entry: " + path));
}

std::string MultiCorpusReaderPrivate::readEntry(std::string const &path) const
{
  std::pair<std::string, bool> fnRec = corpusFromPath(path);
  CorpusReader *reader;
  if (fnRec.second)
    reader = CorpusReaderFactory::openRecursive(fnRec.first);
  else
    reader = CorpusReaderFactory::open(fnRec.first);

  std::string data = reader->read(entryFromPath(path));
  delete reader;
  return data;
}

std::string MultiCorpusReaderPrivate::readEntryMarkQueries(
    std::string const &path, std::list<MarkerQuery> const &queries) const
{
  std::pair<std::string, bool> fnRec = corpusFromPath(path);
  CorpusReader *reader;
  if (fnRec.second)
    reader = CorpusReaderFactory::openRecursive(fnRec.first);
  else
    reader = CorpusReaderFactory::open(fnRec.first);

  std::string data = reader->read(entryFromPath(path), queries);
  delete reader;
  return data;
}

CorpusReader::EntryIterator MultiCorpusReaderPrivate::runXPath(
    std::string const &query) const
{
  return EntryIterator(new MultiIter(d_corporaMap, query));
}

// Iteration over MultiCorpusReaders

MultiCorpusReaderPrivate::MultiIter::MultiIter(
  Corpora const &corpora) : d_hasQuery(false), d_interrupted(false)
{
  for (Corpora::const_iterator
      iter = corpora.begin();
      iter != corpora.end(); ++iter)
    d_iters.push_back(ReaderIter(iter->first, iter->second.first,
          iter->second.second));

    // Initial number of 'iterators'.
    d_totalIters = d_iters.size();
}

MultiCorpusReaderPrivate::MultiIter::MultiIter(
  Corpora const &corpora,
  std::string const &query) : d_hasQuery(true), d_interrupted(false)
{
  for (Corpora::const_iterator
      iter = corpora.begin();
      iter != corpora.end(); ++iter)
    d_iters.push_back(ReaderIter(iter->first, iter->second.first,
          iter->second.second));

    d_query = query;

    // Initial number of 'iterators'.
    d_totalIters = d_iters.size();
}

MultiCorpusReaderPrivate::MultiIter::~MultiIter() {}

IterImpl *MultiCorpusReaderPrivate::MultiIter::copy() const
{
  // No pointer members, and pointer member in ReaderIter is not managed
  // by ReaderIter.
  return new MultiIter(*this);
}

bool MultiCorpusReaderPrivate::MultiIter::hasNext()
{
    if (d_interrupted == true)
        throw IterationInterrupted();
    nextIterator();
    return d_currentIter && d_currentIter->hasNext();
}

bool MultiCorpusReaderPrivate::MultiIter::hasProgress()
{
    return true;
}

void MultiCorpusReaderPrivate::MultiIter::interrupt()
{
  d_interrupted = true;

  if (d_currentIter)
    d_currentIter->progress();
}

Entry MultiCorpusReaderPrivate::MultiIter::next(CorpusReader const &rdr)
{
    Entry e = d_currentIter->next(rdr);
    e.name = d_currentName + "/" + e.name;

    return e;
}

void MultiCorpusReaderPrivate::MultiIter::nextIterator()
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

void MultiCorpusReaderPrivate::MultiIter::openTip()
{
    CorpusReader *reader;
    try {
        if (d_iters.front().recursive)
          reader = CorpusReaderFactory::openRecursive(d_iters.front().filename);
        else
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

double MultiCorpusReaderPrivate::MultiIter::progress()
{
    if (d_currentIter)
      return static_cast<double>(d_totalIters - d_iters.size() - 1) /
          static_cast<double>(d_totalIters) * 100.0;
    else
      return static_cast<double>(d_totalIters - d_iters.size()) /
          static_cast<double>(d_totalIters) * 100.0;
}

}
