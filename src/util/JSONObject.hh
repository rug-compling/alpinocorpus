#ifndef JSON_OBJECT_HH
#define JSON_OBJECT_HH

#include <string>

extern "C"
{
    struct json_object;
}

namespace alpinocorpus {

class JSONObject
{
public:
    static std::string toJSONString(std::string const &str);
    static JSONObject *parse(std::string const &data);
    virtual ~JSONObject();

    std::string stringValue(std::string const &key);

private:
    JSONObject(JSONObject &other);
    JSONObject &operator=(JSONObject &other);
    JSONObject(json_object *object);
    json_object *d_jsonObject;
};

}

#endif //JSON_OBJECT_HH
