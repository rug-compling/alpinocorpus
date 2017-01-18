#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include <boost/filesystem.hpp>

#include "InfInt.h"
#include "NameCompare.hh"
#include "parseString.hh"

namespace {

    std::vector<std::string> nameIndex(std::string const &name)
    {
        std::vector<std::string> index;

        if (name.size() == 0)
            return index;

        std::ostringstream buf;

        bool prevIsDigit = std::isdigit(name[0]);
        for (std::string::const_iterator iter = name.begin(); iter != name.end();
            ++iter)
        {
            bool curIsDigit = std::isdigit(*iter);

            if (curIsDigit && prevIsDigit)
                buf << *iter;
            else
            {
                index.push_back(buf.str());
                buf.str("");
                buf << *iter;
            }

            prevIsDigit = curIsDigit;
        }

        // Leftover
        index.push_back(buf.str());

        return index;
    }

}

namespace alpinocorpus {

    bool NameCompare::operator()(std::string const &s1, std::string const &s2) const
    {
        std::vector<std::string> const &i1 = nameIndex(s1);
        std::vector<std::string> const &i2 = nameIndex(s2);

        for (size_t i = 0; i < i1.size() && i < i2.size(); ++i)
        {
                // Both digits? Sort on digits!
            if (std::isdigit(i1[i][0]) && std::isdigit(i2[i][0]))
            {
                InfInt d1 = util::parseString<InfInt>(i1[i]);
                InfInt d2 = util::parseString<InfInt>(i2[i]);

                if (d1 != d2)
                    return d1 < d2;
            }
            else
                if (i1[i] != i2[i])
                    return i1[i] < i2[i];
            }

        if (i1.size() != i2.size())
          return i1.size() < i2.size();

        // The pairs are equal...
        return false;
    }

    bool PathCompare::operator()(boost::filesystem::path const &p1,
        boost::filesystem::path const &p2) const
    {
        return d_nameCompare(p1.string(), p2.string());
    }
}
