#include <queue>
#include <string>

#include <boost/thread.hpp>

#include "QueuedMessageListener.hh"
#include "../util/JSONObject.hh"

namespace alpinocorpus {

QueuedMessageListener::~QueuedMessageListener()
{
    boost::mutex::scoped_lock lock(d_payloadsMutex);
    d_payloads = std::queue<boost::shared_ptr<JSONObject> >();
}

std::string const &QueuedMessageListener::identifier()
{
  return d_identifier;
}

void QueuedMessageListener::interrupt()
{
  d_interrupted = true;
  d_payloadReady.notify_one();
}

void QueuedMessageListener::process(boost::shared_ptr<JSONObject> payload)
{
  std::string identifier = payload->stringValue("identifier");

  if (identifier != d_identifier)
    return;

  bool wasEmpty;
  {
      boost::mutex::scoped_lock lock(d_payloadsMutex);

      wasEmpty = d_payloads.empty();
      d_payloads.push(payload);

      if (wasEmpty)
        d_payloadReady.notify_one();
  }
  
  // If the queue was empty, notify listeners...
  //if (wasEmpty)
}

boost::shared_ptr<JSONObject> QueuedMessageListener::operator()()
{
  {
    boost::mutex::scoped_lock lock(d_payloadsMutex);

    if (d_payloads.empty())
      d_payloadReady.wait(lock);
  }

  if (d_interrupted)
      return boost::shared_ptr<JSONObject>();

  boost::mutex::scoped_lock lock(d_payloadsMutex);

  assert(!d_payloads.empty());
  boost::shared_ptr<JSONObject> next = d_payloads.front();
  d_payloads.pop();

  return next;
}

bool QueuedMessageListener::operator==(QueuedMessageListener const &other)
{
    boost::mutex::scoped_lock lock(d_payloadsMutex);
    boost::mutex::scoped_lock lockOther(other.d_payloadsMutex);

    return d_identifier == other.d_identifier &&
      d_payloads == other.d_payloads;
}

}
