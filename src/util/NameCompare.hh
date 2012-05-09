#ifndef ALPINOCORPUS_NAME_COMPARE
#define ALPINOCORPUS_NAME_COMPARE

#include <string>

#include <boost/filesystem.hpp>

namespace alpinocorpus
{

struct NameCompare
{
    bool operator()(std::string const &s1, std::string const &s2) const;
};

struct PathCompare
{
    bool operator()(boost::filesystem::path const &p1,
        boost::filesystem::path const &p2) const;
private:
    NameCompare d_nameCompare;
};

}

#endif // ALPINOCORPUS_NAME_COMPARE
