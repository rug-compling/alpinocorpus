#include <sstream>
#include <string>
#include <queue>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread.hpp>

#include <json/json.h>

#include <roles/client.hpp>
#include <websocketpp.hpp>

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/Error.hh>

#include "WebSocketReaderPrivate.hh"
#include "MessageListener/GetMessageListener.hh"
#include "util/JSONObject.hh"

#include <iostream>

using namespace std;

namespace alpinocorpus
{

std::string getIdentifier()
{
    // XXX - accurate enough on Windows?
    boost::posix_time::ptime t =
      boost::posix_time::microsec_clock::universal_time();
    return boost::posix_time::to_iso_string(t);
}

WebSocketReaderPrivate::WebSocketReaderPrivate(std::string const &url)
  : d_handler(new AlpinoCorpusHandler()), d_endpoint(d_handler)
{
  if (url.find("ws://") == string::npos)
    throw OpenError("Could not open WebSocket corpus.");

  d_endpoint.alog().unset_level(websocketpp::log::alevel::ALL);
  d_endpoint.elog().unset_level(websocketpp::log::elevel::ALL);
  d_connection = d_endpoint.connect(url);

  boost::mutex::scoped_lock lock(d_handler->d_connectionReadyMutex);

  d_endpointThread.reset(new boost::thread(boost::bind(&websocketpp::client::run, &d_endpoint, false)));

  // Wait until the connection is ready.
  d_handler->d_connectionReady.wait(lock);
}

WebSocketReaderPrivate::~WebSocketReaderPrivate()
{
  d_endpoint.close_all();
  d_endpointThread->join();
}

CorpusReader::EntryIterator WebSocketReaderPrivate::getBegin() const
{
    std::string identifier = getIdentifier();
    WebSocketIter *iter = new WebSocketIter(d_handler, identifier);
    d_handler->send(std::string("{\"command\": \"list\", \"identifier\": \"") +
        identifier + "\"}");
    return EntryIterator(iter);
}

CorpusReader::EntryIterator WebSocketReaderPrivate::runQueryWithStylesheet(
    QueryDialect d, std::string const &query, std::string const &stylesheet,
    std::list<MarkerQuery> const &markerQueries) const
{
  std::string identifier = getIdentifier();
  WebSocketIter *iter = new WebSocketIter(d_handler, identifier);

  ostringstream queryStream;

  queryStream << "{\"command\": \"queryWithStylesheet\", \"identifier\": \""
      << identifier << "\","
      << "\"query\": " << JSONObject::toJSONString(query) << ", "
      << "\"stylesheet\": " << JSONObject::toJSONString(stylesheet)
      << ", \"markers\": [";

  for (std::list<MarkerQuery>::const_iterator markerIter = markerQueries.begin();
      markerIter != markerQueries.end(); ++markerIter)
  {
    queryStream << "{";
    queryStream << "\"query\": "
        << JSONObject::toJSONString(markerIter->query) << ",";
    queryStream << "\"attribute\": "
        << JSONObject::toJSONString(markerIter->attr) << ",";
    queryStream << "\"value\": "
        << JSONObject::toJSONString(markerIter->value);
    queryStream << "}";

    // Hmpf...
    std::list<MarkerQuery>::const_iterator nextIter = markerIter;
    ++nextIter;
    if (nextIter != markerQueries.end())
      queryStream << ",";
  }

  queryStream << "]}";

  // XXX - Use libjson?
  d_handler->send(queryStream.str());  

  return EntryIterator(iter);
}

CorpusReader::EntryIterator WebSocketReaderPrivate::runXPath(std::string const &query) const
{
    std::string identifier = getIdentifier();
    WebSocketIter *iter = new WebSocketIter(d_handler, identifier);
    d_handler->send(std::string("{\"command\": \"query\", \"identifier\": \"") +
      identifier + "\", \"query\": " + JSONObject::toJSONString(query) + "}");
    return EntryIterator(iter);
}

CorpusReader::EntryIterator WebSocketReaderPrivate::getEnd() const
{
    WebSocketIter *iter = new WebSocketIter();
    return EntryIterator(iter);
}

std::string WebSocketReaderPrivate::getName() const
{
  return "websockets";
}

size_t WebSocketReaderPrivate::getSize() const
{
  throw "not implemented";
}

std::string WebSocketReaderPrivate::readEntry(std::string const &entry) const
{
  std::string id = getIdentifier();
  GetMessageListener getListener(id);
  d_handler->addListener(&getListener);
  d_handler->send(std::string("{\"command\": \"get\", \"identifier\": \"") +
      id + "\", \"entry\": \"" + entry + "\"}");
  boost::shared_ptr<JSONObject> obj = getListener();
  d_handler->removeListener(&getListener);
 
  std::string data = obj->stringValue("data");

  return data;
}

WebSocketReaderPrivate::WebSocketIter::WebSocketIter(
        boost::shared_ptr<AlpinoCorpusHandler> handler,
        std::string const &identifier) :
    d_handler(handler), d_first(true),
    d_listener(new QueuedMessageListener(identifier))
{
    d_handler->addListener(d_listener.get());
}

WebSocketReaderPrivate::WebSocketIter::WebSocketIter() :
    d_first(false)
{
}


WebSocketReaderPrivate::WebSocketIter::~WebSocketIter()
{
    if (d_handler && d_listener) {
        d_handler->removeListener(d_listener.get());
    }

}

void WebSocketReaderPrivate::WebSocketIter::advanceToFirst() const
{
    if (d_first) {
        WebSocketIter &iter = const_cast<WebSocketIter &>(*this);
        iter.next();
        iter.d_first = false;
    }
}

void WebSocketReaderPrivate::WebSocketIter::interrupt()
{
    if (d_handler && d_listener) {
        d_handler->removeListener(d_listener.get());
        d_listener->interrupt();
    }
}

void WebSocketReaderPrivate::WebSocketIter::next()
{
    d_current = (*d_listener)();

    if (d_current == boost::shared_ptr<JSONObject>())
        throw IterationInterrupted();

    // If the server sends a message that it has enumerated
    // all entries, change this iterator in an end iterator...
    if (d_current->stringValue("result") == "end")
    {
        d_first = false; // We could've had zero results...
        d_handler->removeListener(d_listener.get());
	d_listener.reset();
        d_handler.reset();
        d_current.reset();
    }
}

std::string WebSocketReaderPrivate::WebSocketIter::contents(CorpusReader const &) const
{
    advanceToFirst();
    return d_current->stringValue("contents");
}


std::string WebSocketReaderPrivate::WebSocketIter::current() const
{
    advanceToFirst();
    return d_current->stringValue("entry");
}

IterImpl *WebSocketReaderPrivate::WebSocketIter::copy() const
{
    WebSocketIter *iter = new WebSocketIter;
    iter->d_handler = d_handler;
    if (d_listener) {
        iter->d_listener.reset(new QueuedMessageListener(d_listener->identifier()));
        d_handler->addListener(iter->d_listener.get());
    }

    iter->d_current = d_current;
    iter->d_first = d_first;

    return iter;
}

bool WebSocketReaderPrivate::WebSocketIter::equals(IterImpl const &otherBase) const
{
    advanceToFirst();

    try {
        WebSocketIter const &other = dynamic_cast<WebSocketIter const &>(otherBase);

        if (d_listener && other.d_listener)
            return other.d_handler == d_handler &&
                    *other.d_listener == *d_listener &&
                    other.d_current == d_current;
        else
            return other.d_handler == d_handler &&
                    other.d_current == d_current;

    } catch (std::bad_cast const &e) {
        return false;
    }
}

void AlpinoCorpusHandler::addListener(MessageListener *listener)
{
  boost::mutex::scoped_lock lock(d_listenersMutex);
  d_listeners.insert(listener);
}

void AlpinoCorpusHandler::removeListener(MessageListener *listener)
{
  boost::mutex::scoped_lock lock(d_listenersMutex);
  d_listeners.erase(listener);
}

void AlpinoCorpusHandler::on_message(connection_ptr conn, message_ptr msg)
{
  if (msg->get_payload() == "server 1.0")
    return;
 
  boost::shared_ptr<JSONObject> json(JSONObject::parse(msg->get_payload()));

  {
      boost::mutex::scoped_lock lock(d_listenersMutex);
      for (Listeners::const_iterator iter = d_listeners.begin();
          iter != d_listeners.end(); ++iter)
        (*iter)->process(json);
  }
}

void AlpinoCorpusHandler::on_open(connection_ptr connection)
{
  d_connection = connection;
  send("client 1.0");
  boost::mutex::scoped_lock lock(d_connectionReadyMutex);
  d_connectionReady.notify_one();
}

void AlpinoCorpusHandler::send(std::string const &msg)
{
  if (!d_connection) {
    return;
  }

  d_connection->send(msg);
}

}
