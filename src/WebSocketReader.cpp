#include <string>

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/WebSocketReader.hh>

#include "WebSocketReaderPrivate.hh"

namespace alpinocorpus {
    
WebSocketReader::WebSocketReader(std::string const &url) :
    d_private(new WebSocketReaderPrivate(url))
{
}

WebSocketReader::~WebSocketReader()
{
    delete d_private;
}
    
CorpusReader::EntryIterator WebSocketReader::getBegin() const
{
    return d_private->getBegin();
}

CorpusReader::EntryIterator WebSocketReader::getEnd() const
{
    return d_private->getEnd();
}

std::string WebSocketReader::getName() const
{
  return d_private->getName();
}

size_t WebSocketReader::getSize() const
{
    return d_private->getSize();
}

std::string WebSocketReader::readEntry(std::string const &entry) const
{
    return d_private->readEntry(entry);
}

CorpusReader::EntryIterator WebSocketReader::runQueryWithStylesheet(
    QueryDialect d, std::string const &query, std::string const &stylesheet,
    std::list<MarkerQuery> const &markerQueries) const
{
    return d_private->runQueryWithStylesheet(d, query, stylesheet,
        markerQueries);
}


CorpusReader::EntryIterator WebSocketReader::runXPath(std::string const &query) const
{
    return d_private->runXPath(query);
}

}   // namespace alpinocorpus
