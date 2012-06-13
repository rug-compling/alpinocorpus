#include <queue>
#include <string>

#include <boost/optional.hpp>
#include <boost/thread.hpp>

#include "QueuedMessageListener.hh"
#include "../util/JSONObject.hh"

namespace alpinocorpus {

QueuedMessageListener::~QueuedMessageListener()
{
    boost::mutex::scoped_lock lock(d_payloadsMutex);
    d_payloads = std::queue<boost::optional<JSONObjectPtr> >();
}

void QueuedMessageListener::close()
{
    bool wasEmpty;
    {
        boost::mutex::scoped_lock lock(d_payloadsMutex);

        wasEmpty = d_payloads.empty();
        d_payloads.push(boost::optional<JSONObjectPtr>());

        if (wasEmpty)
            d_payloadReady.notify_one();
    }
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

void QueuedMessageListener::process(JSONObjectPtr payload)
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

boost::optional<JSONObjectPtr> QueuedMessageListener::operator()()
{
  {
    boost::mutex::scoped_lock lock(d_payloadsMutex);

    if (d_payloads.empty())
      d_payloadReady.wait(lock);
  }

  if (d_interrupted)
      return boost::optional<JSONObjectPtr>();

  boost::mutex::scoped_lock lock(d_payloadsMutex);

  assert(!d_payloads.empty());
  boost::optional<JSONObjectPtr> next = d_payloads.front();
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
