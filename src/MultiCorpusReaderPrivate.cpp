#include <list>
#include <stdexcept>
#include <string>
#include <utility>

#include <boost/tr1/unordered_map.hpp>

#include <boost/filesystem.hpp>

#include <boost/config.hpp>

#if defined(BOOST_HAS_THREADS)
#include <boost/thread/mutex.hpp>
#endif

#include <AlpinoCorpus/tr1wrap/memory.hh>

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/CorpusReaderFactory.hh>
#include <AlpinoCorpus/Error.hh>
#include <AlpinoCorpus/IterImpl.hh>
#include <AlpinoCorpus/util/Either.hh>

#include <config.hh>

#ifdef USE_DBXML
#include <dbxml/DbXml.hpp>
#endif

#include "MultiCorpusReaderPrivate.hh"

namespace bf = boost::filesystem;

namespace alpinocorpus {

MultiCorpusReaderPrivate::MultiCorpusReaderPrivate()
{
    DbXml::XmlContainerConfig config;
    config.setReadOnly(false);

    // Create an in-memory container.
    d_container = d_mgr.createContainer("", config,
        DbXml::XmlContainer::NodeContainer);

    // Default container name.
    d_container.addAlias("corpus"); 
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
    bf::path corpusPath(filename);
    if (!bf::exists(corpusPath))
      throw OpenError(filename);

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
  return EntryIterator(new MultiIter(d_corporaMap, query, CorpusReader::XPATH));
}

CorpusReader::EntryIterator MultiCorpusReaderPrivate::runXQuery(
    std::string const &query) const
{
  return EntryIterator(new MultiIter(d_corporaMap, query, CorpusReader::XQUERY));
}

// Iteration over MultiCorpusReaders

MultiCorpusReaderPrivate::MultiIter::MultiIter(
  Corpora const &corpora) : d_hasQuery(false), d_interrupted(false)
{
#if defined(BOOST_HAS_THREADS)
    d_currentIterMutex.reset(new boost::mutex);
#endif

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
  std::string const &query,
  CorpusReader::QueryDialect dialect) :
  d_hasQuery(true), d_query(query), d_dialect(dialect), d_interrupted(false)
{
#if defined(BOOST_HAS_THREADS)
    d_currentIterMutex.reset(new boost::mutex);
#endif

  for (Corpora::const_iterator
      iter = corpora.begin();
      iter != corpora.end(); ++iter)
    d_iters.push_back(ReaderIter(iter->first, iter->second.first,
          iter->second.second));

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
    if (d_interrupted)
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

  // d_currentIter could be resetted in the iteration thread after the
  // null-pointer check.
#if defined(BOOST_HAS_THREADS)
  boost::mutex::scoped_lock lock(*d_currentIterMutex);
#endif
  if (d_currentIter)
    d_currentIter->interrupt();
}

Entry MultiCorpusReaderPrivate::MultiIter::next(CorpusReader const &rdr)
{
    if (d_interrupted)
        throw IterationInterrupted();

    Entry e = d_currentIter->next(rdr);
    e.name = d_currentName + "/" + e.name;

    return e;
}

void MultiCorpusReaderPrivate::MultiIter::nextIterator()
{
  while (d_iters.size() != 0 && !d_interrupted &&
    (!d_currentIter || !d_currentIter->hasNext()))
  {
#if defined(BOOST_HAS_THREADS)
    boost::mutex::scoped_lock lock(*d_currentIterMutex);
#endif
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
        d_currentIter.reset(new EntryIterator(reader->query(d_dialect, d_query)));
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

#ifdef USE_DBXML
Either<std::string, Empty> MultiCorpusReaderPrivate::validQuery(QueryDialect d, bool variables,
    std::string const &query) const
{
        // Prefer to validate the query on an actual corpus.
        if (d_corporaMap.begin() != d_corporaMap.end())
        {
            try {
                std::pair<std::string, bool> p = d_corporaMap.begin()->second;
                std::tr1::shared_ptr<CorpusReader> reader;
                if (p.second)
                    reader.reset(CorpusReaderFactory::openRecursive(p.first));
                else
                    reader.reset(CorpusReaderFactory::open(p.first));

                Either<std::string, Empty> result = reader->isValidQuery(d, variables, query);

                return result;
            } catch (OpenError const &)
            {
            }
        }
        else {
            try {
                DbXml::XmlQueryContext ctx = d_mgr.createQueryContext();
                d_mgr.prepare(query, ctx);
            } catch (DbXml::XmlException const &e) {
                return Either<std::string, Empty>::left(e.what());
            }
        }

        return Either<std::string, Empty>::right(Empty());
#endif
}

}
