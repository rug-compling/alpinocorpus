/*
 * Remote corpus reader.
 */

#include <list>
#include <string>

#include <AlpinoCorpus/RemoteCorpusReader.hh>
#include "RemoteCorpusReaderPrivate.hh"

namespace alpinocorpus {
    
    RemoteCorpusReader::RemoteCorpusReader(std::string const &url) :
        d_private(new RemoteCorpusReaderPrivate(url))
    {
    }

    RemoteCorpusReader::~RemoteCorpusReader()
    {
        delete d_private;
    }
    
    CorpusReader::EntryIterator RemoteCorpusReader::getBegin() const
    {
        return d_private->getBegin();
    }

    CorpusReader::EntryIterator RemoteCorpusReader::getEnd() const
    {
        return d_private->getEnd();
    }

    std::string RemoteCorpusReader::getName() const
    {
        return d_private->getName();
    }

    size_t RemoteCorpusReader::getSize() const
    {
        return d_private->getSize();
    }
    
    bool RemoteCorpusReader::validQuery(QueryDialect d, bool variables, std::string const &query) const
    {
        return d_private->isValidQuery(d, variables, query);
    }


    std::string RemoteCorpusReader::readEntry(std::string const &entry) const
    {
        return d_private->readEntry(entry);
    }

    std::string RemoteCorpusReader::readEntryMarkQueries(std::string const &entry, 
                                                         std::list<MarkerQuery> const &queries) const
    {
        return d_private->readEntryMarkQueries(entry, queries);
    }

    CorpusReader::EntryIterator RemoteCorpusReader::runXPath(std::string const &query) const
    {
        return d_private->runXPath(query);
    }

    CorpusReader::EntryIterator RemoteCorpusReader::runXQuery(std::string const &query) const
    {
        return d_private->runXQuery(query);
    }

}   // namespace alpinocorpus