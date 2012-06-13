#ifndef QUEUED_MESSAGE_LISTENER_HH
#define QUEUED_MESSAGE_LISTENER_HH

#include <queue>
#include <string>

#include <boost/optional.hpp>
#include <boost/thread.hpp>

#include "MessageListener.hh"
#include "../util/JSONObject.hh"

namespace alpinocorpus {

class QueuedMessageListener : public MessageListener
{
public:
  QueuedMessageListener(std::string const &newIdentifier) :
      d_identifier(newIdentifier), d_interrupted(false) {}
  ~QueuedMessageListener();

  boost::optional<JSONObjectPtr> operator()();
  bool operator==(QueuedMessageListener const &other);

  void close();
  std::string const &identifier();
  void interrupt();
  void process(JSONObjectPtr payload);

private:
  std::string d_identifier;
  std::queue<boost::optional<JSONObjectPtr> > d_payloads;
  mutable boost::mutex d_payloadsMutex;
  boost::condition_variable d_payloadReady;
  bool d_interrupted;
};

}

#endif // QUEUED_MESSAGE_LISTENER_HH
