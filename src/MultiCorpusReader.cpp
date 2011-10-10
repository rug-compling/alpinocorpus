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

}

