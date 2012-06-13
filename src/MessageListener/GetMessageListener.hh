#ifndef GET_MESSAGE_LISTENER_HH
#define GET_MESSAGE_LISTENER_HH

#include <string>

#include <boost/optional.hpp>
#include <boost/thread.hpp>

#include "MessageListener.hh"
#include "../util/JSONObject.hh"

namespace alpinocorpus {

class GetMessageListener : public MessageListener
{
public:
  GetMessageListener(std::string const &id) : d_id(id) {}
  ~GetMessageListener() {}

  boost::optional<JSONObjectPtr> operator()();

  void close();
  boost::optional<JSONObjectPtr> payload();
  void process(boost::shared_ptr<JSONObject> payload);

private:
  std::string d_id;
  boost::optional<JSONObjectPtr> d_payload;
  boost::condition_variable d_payloadReady;
};

}

#endif // GET_MESSAGE_LISTENER_HH
