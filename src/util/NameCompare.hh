#ifndef ALPINOCORPUS_NAME_COMPARE
#define ALPINOCORPUS_NAME_COMPARE

#include <filesystem>
#include <string>

namespace alpinocorpus
{

struct NameCompare
{
    bool operator()(std::string const &s1, std::string const &s2) const;
};

struct PathCompare
{
    bool operator()(std::filesystem::path const &p1,
        std::filesystem::path const &p2) const;
private:
    NameCompare d_nameCompare;
};

}

#endif // ALPINOCORPUS_NAME_COMPARE
