#ifndef ALPINOCORPUS_LEX_ITEM_HH
#define ALPINOCORPUS_LEX_ITEM_HH


#include <string>
#include <set>
#include <vector>

namespace alpinocorpus {

struct LexItem
{
    std::string word;
    size_t begin;
    std::set<size_t> matches;

    inline bool operator<(LexItem const &other) const
    {
        if (begin != other.begin)
          return begin < other.begin;
        else
          return word < other.word;
    }
};

}

#endif // ALPINOCORPUS_LEX_ITEM_HH
