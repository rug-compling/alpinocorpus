#ifndef ALPINO_WEBSOCKET_READER_HH
#define ALPINO_WEBSOCKET_READER_HH

#include <string>

#include <AlpinoCorpus/CorpusReader.hh>

namespace alpinocorpus {

class WebSocketReaderPrivate;
    
class WebSocketReader : public CorpusReader
{
  public:
    WebSocketReader(std::string const &url);
    virtual ~WebSocketReader();

  private:
    EntryIterator getBegin() const;
    EntryIterator getEnd() const;
    std::string getName() const;
    std::string readEntry(std::string const &) const;
        EntryIterator runQueryWithStylesheet(QueryDialect d,
        std::string const &query, std::string const &stylesheet,
        std::list<MarkerQuery> const &markerQueries) const;
    EntryIterator runXPath(std::string const &) const;
    size_t getSize() const;

    WebSocketReaderPrivate *d_private;
};

}   // namespace alpinocorpus

#endif // ALPINO_WEBSOCKET_READER_HH
