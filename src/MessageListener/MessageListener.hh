#ifndef MESSAGE_LISTENER_HH
#define MESSAGE_LISTENER_HH

#include <AlpinoCorpus/DLLDefines.hh>
#include <AlpinoCorpus/util/NonCopyable.hh>

#include "../util/JSONObject.hh"

namespace alpinocorpus {

class MessageListener : private util::NonCopyable
{
public:
  virtual ~MessageListener() {}
  virtual void process(boost::shared_ptr<JSONObject> payload) = 0;
};

}

#endif // MESSAGE_LISTENER_HH
