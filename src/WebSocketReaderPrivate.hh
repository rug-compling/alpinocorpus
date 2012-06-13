#ifndef WEBSOCKET_READER_PRIVATE_HH
#define WEBSOCKET_READER_PRIVATE_HH

#include <set>
#include <string>

#include <boost/smart_ptr/scoped_ptr.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/thread.hpp>

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/IterImpl.hh>

#include <roles/client.hpp>
#include <websocketpp.hpp>

#include "MessageListener/QueuedMessageListener.hh"
#include "util/JSONObject.hh"

namespace alpinocorpus {

class AlpinoCorpusHandler : public websocketpp::client::handler {
public:
  virtual ~AlpinoCorpusHandler() {}
  void addListener(MessageListener *listener);
  void on_message(connection_ptr conn, message_ptr msg);
  void on_close(connection_ptr conn);
  void on_fail(connection_ptr con);
  void on_open(connection_ptr connection);
  void removeListener(MessageListener *listener);
  void send(std::string const &msg);
  boost::condition_variable d_connectionReady;
  boost::mutex d_connectionReadyMutex;
private:
  typedef std::set<MessageListener *> Listeners;

  websocketpp::client::connection_ptr d_connection;
  boost::mutex d_listenersMutex;
  Listeners d_listeners;
};

class WebSocketReaderPrivate : public CorpusReader
{
public:
    WebSocketReaderPrivate(std::string const &url);
    virtual ~WebSocketReaderPrivate();

    EntryIterator getBegin() const;
    EntryIterator getEnd() const;
    size_t getSize() const;
    std::string getName() const;
    std::string readEntry(std::string const &entry) const;
    EntryIterator runQueryWithStylesheet(QueryDialect d,
        std::string const &query, std::string const &stylesheet,
        std::list<MarkerQuery> const &markerQueries) const;
    EntryIterator runXPath(std::string const &query) const;
private:
    class WebSocketIter : public IterImpl
    {
    public:
        WebSocketIter(boost::shared_ptr<AlpinoCorpusHandler> handler,
            std::string const &identifier);
        WebSocketIter();
        virtual ~WebSocketIter();

        std::string contents(CorpusReader const &) const;
        virtual IterImpl *copy() const;
        std::string current() const;
        bool equals(IterImpl const &) const;
        void interrupt();
        void next();

    private:
        void advanceToFirst() const;

        bool d_first;
        boost::shared_ptr<AlpinoCorpusHandler> d_handler;
        boost::shared_ptr<QueuedMessageListener> d_listener;
        boost::optional<JSONObjectPtr> d_current;
    };

    boost::shared_ptr<AlpinoCorpusHandler> d_handler;
    websocketpp::client::connection_ptr d_connection;
    boost::scoped_ptr<boost::thread> d_endpointThread;
    websocketpp::client d_endpoint;
};

}

#endif // WEBSOCKET_READER_PRIVATE_HH
