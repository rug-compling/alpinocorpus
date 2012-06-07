#ifndef GET_MESSAGE_LISTENER_HH
#define GET_MESSAGE_LISTENER_HH

#include <string>

#include <boost/thread.hpp>

#include "MessageListener.hh"
#include "../util/JSONObject.hh"

namespace alpinocorpus {

class GetMessageListener : public MessageListener
{
public:
  GetMessageListener(std::string const &id) : d_id(id) {}
  ~GetMessageListener() {}

  boost::shared_ptr<JSONObject> operator()();

  boost::shared_ptr<JSONObject> payload();
  void process(boost::shared_ptr<JSONObject> payload);

private:
  std::string d_id;
  boost::shared_ptr<JSONObject> d_payload;
  boost::condition_variable d_payloadReady;
};

}

#endif // GET_MESSAGE_LISTENER_HH
