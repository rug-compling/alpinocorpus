#include <string>

#include <boost/optional.hpp>
#include <boost/thread.hpp>

#include "../util/JSONObject.hh"

#include "GetMessageListener.hh"

namespace alpinocorpus {

boost::optional<JSONObjectPtr> GetMessageListener::operator()()
{
    boost::mutex connection_mutex;
    boost::unique_lock<boost::mutex> connection_lock(connection_mutex);
    d_payloadReady.wait(connection_lock);

    return d_payload;
}

void GetMessageListener::close()
{
    d_payload = boost::optional<JSONObjectPtr>();
    d_payloadReady.notify_one();
}

boost::optional<JSONObjectPtr> GetMessageListener::payload()
{
    return d_payload;
}

void GetMessageListener::process(boost::shared_ptr<JSONObject> payload)
{
    std::string identifier = payload->stringValue("identifier");

    if (identifier != d_id)
        return;

    d_payload = payload;
    d_payloadReady.notify_one();
}


}
