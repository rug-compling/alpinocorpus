#include <cassert>
#include <string>

#include <json/json.h>

#include "JSONObject.hh"

namespace alpinocorpus {

std::string JSONObject::toJSONString(std::string const &str)
{
    json_object *strObj = json_object_new_string(str.c_str());
    assert(strObj); // Should never fail.
    char const *cStr = json_object_to_json_string(strObj);
    std::string result(cStr);
    json_object_put(strObj);
    return result;
}

JSONObject *JSONObject::parse(std::string const &data)
{
    json_object *new_obj = json_tokener_parse(data.c_str());
    return new JSONObject(new_obj);
}

JSONObject::JSONObject(json_object *object)
{
    d_jsonObject = object;
}


JSONObject::~JSONObject()
{
    json_object_put(d_jsonObject);
}

std::string JSONObject::stringValue(std::string const &key)
{
    json_object *idObj = json_object_object_get(d_jsonObject,
        key.c_str());

    assert(idObj != 0);
    assert(json_object_is_type(idObj, json_type_string));

    char const *cId = json_object_get_string(idObj);

    return std::string(cId);
}


}
