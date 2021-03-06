#include <string>

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/MultiCorpusReader.hh>

#include "MultiCorpusReaderPrivate.hh"

namespace alpinocorpus {

MultiCorpusReader::MultiCorpusReader() :
  d_private(new MultiCorpusReaderPrivate())
{
}

MultiCorpusReader::~MultiCorpusReader()
{
  delete d_private;
}

CorpusReader::EntryIterator MultiCorpusReader::getEntries(SortOrder sortOrder) const
{
  return d_private->getEntries(sortOrder);
}

std::string MultiCorpusReader::getName() const
{
  return d_private->getName();
}

size_t MultiCorpusReader::getSize() const
{
  return d_private->getSize();
}

void MultiCorpusReader::push_back(std::string const &name, std::string const &reader,
    bool recursive)
{
  d_private->push_back(name, reader, recursive);
}

Either<std::string, Empty> MultiCorpusReader::validQuery(QueryDialect d, bool variables, std::string const &query) const
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

CorpusReader::EntryIterator MultiCorpusReader::runXPath(std::string const &query, SortOrder sortOrder) const
{
  return d_private->query(XPATH, query, sortOrder);
}

CorpusReader::EntryIterator MultiCorpusReader::runXQuery(std::string const &query, SortOrder sortOrder) const
{
  return d_private->query(XQUERY, query, sortOrder);
}

}
