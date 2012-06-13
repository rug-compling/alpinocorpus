#ifndef MESSAGE_LISTENER_HH
#define MESSAGE_LISTENER_HH

#include <boost/tr1/memory.hpp>

#include <AlpinoCorpus/DLLDefines.hh>
#include <AlpinoCorpus/util/NonCopyable.hh>

#include "../util/JSONObject.hh"

namespace alpinocorpus {

typedef std::tr1::shared_ptr<JSONObject> JSONObjectPtr;

class MessageListener : private util::NonCopyable
{
public:
  virtual ~MessageListener() {}
  virtual void close() = 0;
  virtual void process(boost::shared_ptr<JSONObject> payload) = 0;
};

}

#endif // MESSAGE_LISTENER_HH
