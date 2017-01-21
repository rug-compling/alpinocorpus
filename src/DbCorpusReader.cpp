/*
 * Oracle Berkeley DB XML-based treebank reader.
 * Written by Lars Buitinck.
 *
 * The basic format is explained in the header. An additional notes:
 *  - We use DBML_WELL_FORMED_ONLY to prevent having to read the DTD.
 *    This only works because the DTD does not define entities.
 */

#include <list>
#include <string>

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/DbCorpusReader.hh>

#include "DbCorpusReaderPrivate.hh"

namespace alpinocorpus {
    
DbCorpusReader::DbCorpusReader(std::string const &name) :
    d_private(new DbCorpusReaderPrivate(name))
{
}

DbCorpusReader::~DbCorpusReader()
{
    delete d_private;
}
    
CorpusReader::EntryIterator DbCorpusReader::getEntries(SortOrder sortOrder) const
{
    return d_private->getEntries(sortOrder);
}

std::string DbCorpusReader::getName() const
{
  return d_private->getName();
}

size_t DbCorpusReader::getSize() const
{
    return d_private->getSize();
}
    
Either<std::string, Empty> DbCorpusReader::validQuery(QueryDialect d, bool variables, std::string const &query) const
{
    return d_private->isValidQuery(d, variables, query);
}


std::string DbCorpusReader::readEntry(std::string const &entry) const
{
    return d_private->readEntry(entry);
}

CorpusReader::EntryIterator DbCorpusReader::runXPath(std::string const &query) const
{
    return d_private->runXPath(query);
}

CorpusReader::EntryIterator DbCorpusReader::runXQuery(std::string const &query) const
{
    return d_private->runXQuery(query);
}

}   // namespace alpinocorpus
