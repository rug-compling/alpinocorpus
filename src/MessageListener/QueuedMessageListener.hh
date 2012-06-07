#ifndef QUEUED_MESSAGE_LISTENER_HH
#define QUEUED_MESSAGE_LISTENER_HH

#include <queue>
#include <string>

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

  boost::shared_ptr<JSONObject> operator()();
  bool operator==(QueuedMessageListener const &other);

  std::string const &identifier();
  void interrupt();
  void process(boost::shared_ptr<JSONObject> payload);

private:
  std::string d_identifier;
  std::queue<boost::shared_ptr<JSONObject> > d_payloads;
  mutable boost::mutex d_payloadsMutex;
  boost::condition_variable d_payloadReady;
  bool d_interrupted;
};

}

#endif // QUEUED_MESSAGE_LISTENER_HH
