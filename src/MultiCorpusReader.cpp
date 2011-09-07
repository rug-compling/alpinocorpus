#include <list>
#include <stdexcept>
#include <string>
#include <utility>

#include <tr1/unordered_map>

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/MultiCorpusReader.hh>

namespace alpinocorpus {


class MultiCorpusReaderPrivate : public CorpusReader
{
  class MultiIter : public CorpusReader::IterImpl
  {
  public:
    MultiIter(std::list<CorpusReader *> const &readers);
    ~MultiIter();
    std::string current() const;
    bool equals(IterImpl const &other) const;
    void next();
  private:
    typedef std::pair<CorpusReader *, CorpusReader::EntryIterator> ReaderIterPair;
    std::list<ReaderIterPair> d_iters;
  };
public:
  MultiCorpusReaderPrivate() {};
  virtual ~MultiCorpusReaderPrivate();

  EntryIterator getBegin() const;
  EntryIterator getEnd() const;
  std::string getName() const;
  size_t getSize() const;
  void push_back(CorpusReader *reader);
  std::string readEntry(std::string const &) const;
  std::string readEntryMarkQueries(std::string const &entry, std::list<MarkerQuery> const &queries) const;
  bool validQuery(QueryDialect d, bool variables, std::string const &query) const;

private:
  CorpusReader const *corpusReaderFromPath(std::string const &path) const;
  std::string entryFromPath(std::string const &path) const;

  std::list<CorpusReader *> d_corpusReaders;
  std::tr1::unordered_map<std::string, CorpusReader *> d_corpusReaderMap;
};


// Implementation of the public interface.
MultiCorpusReader::MultiCorpusReader() :
  d_private(new MultiCorpusReaderPrivate)
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
    
bool MultiCorpusReader::validQuery(QueryDialect d, bool variables, std::string const &query) const
{
    return d_private->isValidQuery(d, variables, query);
}

void MultiCorpusReader::push_back(CorpusReader *reader)
{
  d_private->push_back(reader);
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

// Implementation of the private interface

MultiCorpusReaderPrivate::~MultiCorpusReaderPrivate()
{
  for (std::list<CorpusReader *>::iterator iter = d_corpusReaders.begin();
      iter != d_corpusReaders.end(); ++iter)
    delete *iter;
}

CorpusReader::EntryIterator MultiCorpusReaderPrivate::getBegin() const
{
  return EntryIterator(new MultiIter(d_corpusReaders));
}

CorpusReader::EntryIterator MultiCorpusReaderPrivate::getEnd() const
{
  return EntryIterator(new MultiIter(std::list<CorpusReader *>()));
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

void MultiCorpusReaderPrivate::push_back(CorpusReader *reader)
{
  // Ignore empty corpus readers, simplifies assumptions.
  if (reader->size() == 0) {
    delete reader;
    return;
  }

  d_corpusReaders.push_back(reader);
  d_corpusReaderMap[reader->name()] = reader; // XXX - exists check?
}

CorpusReader const *MultiCorpusReaderPrivate::corpusReaderFromPath(
    std::string const &path) const
{
  int slashPos = path.find("/", 0);
  std::string readerName = path.substr(0, slashPos);

  std::tr1::unordered_map<std::string, CorpusReader *>::const_iterator iter =
    d_corpusReaderMap.find(readerName);
  if (iter == d_corpusReaderMap.end())
    throw std::runtime_error(std::string("Unknown corpus: " + readerName));

  return iter->second;
}

std::string MultiCorpusReaderPrivate::entryFromPath(
    std::string const &path) const
{
  int slashPos = path.find("/", 0);
  return path.substr(slashPos + 1);
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

bool MultiCorpusReaderPrivate::validQuery(QueryDialect d, bool variables, std::string const &query) const
{
  if (d_corpusReaders.size() > 0)
    return d_corpusReaders.front()->isValidQuery(d, variables, query);

  return false;
}

// Iteration over MultiCorpusReaders

MultiCorpusReaderPrivate::MultiIter::MultiIter(
  std::list<CorpusReader *> const &readers)
{
  for (std::list<CorpusReader *>::const_iterator iter = readers.begin();
      iter != readers.end(); ++iter)
    d_iters.push_back(make_pair(*iter, (*iter)->begin()));

  // TODO: Make sure that we are positioned correctly.
}

MultiCorpusReaderPrivate::MultiIter::~MultiIter() {}

std::string MultiCorpusReaderPrivate::MultiIter::current() const
{
  if (d_iters.size() == 0)
    throw std::runtime_error("Cannot dereference an end iterator!");

  return d_iters.front().first->name() + "/" + *d_iters.front().second;
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
  if (++d_iters.front().second != d_iters.front().first->end())
    return;

  // Ok, we are at the end of the current corpus, so we'll remove the
  // corpus from the iteration list, and get started on the next corpus.
  d_iters.pop_front();
}

}

void bla() {
  alpinocorpus::MultiCorpusReader reader;
}
