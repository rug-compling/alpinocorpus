#ifndef QUERY_RESULT_HANDLER
#define QUERY_RESULT_HANDLER

#include <string>

struct Entry
{
  std::string name;
  std::string value;
};

class QueryResultHandler
{
  public:
    virtual void handleEntry(Entry const &entry) = 0;
};

#endif //QUERY_RESULT_HANDLER
