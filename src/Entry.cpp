#include <AlpinoCorpus/Entry.hh>

#include <algorithm>
#include <cassert>
#include <iterator>
#include <vector>

#include <boost/filesystem.hpp>

namespace alpinocorpus {

std::string Entry::shortName()
{

  boost::filesystem::path p(d_name);

  std::vector<boost::filesystem::path> pathComponents;
  std::copy(p.begin(), p.end(), back_inserter(pathComponents));

  assert(pathComponents.size() > 0);

  // Shouldn't really happen...
  if (pathComponents.size() == 1)
    return d_name;

  boost::filesystem::path dir(pathComponents[pathComponents.size() - 2]);
  if (dir.has_extension())
    dir.replace_extension("");

  return dir.string() + '/' + pathComponents[pathComponents.size() - 1].string();
}

}
