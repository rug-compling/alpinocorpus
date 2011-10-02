#include <stdexcept>
#include <string>
#include <vector>

#include <boost/filesystem.hpp>

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/MultiCorpusReader.hh>

namespace bf = boost::filesystem;

using alpinocorpus::CorpusReader;
using alpinocorpus::MultiCorpusReader;

CorpusReader* openCorpus(std::string const &path,
    bool recursive)
{
    if (recursive)
      return CorpusReader::openRecursive(path);
    else
      return CorpusReader::open(path);
}

CorpusReader *openCorpora(
    std::vector<std::string>::const_iterator const &pathBegin,    
    std::vector<std::string>::const_iterator const &pathEnd,
    bool recursive)
{
  MultiCorpusReader *readers = new MultiCorpusReader;

  for (std::vector<std::string>::const_iterator iter = pathBegin;
      iter != pathEnd; ++iter)
  {
    CorpusReader *reader = openCorpus(*iter, recursive);
    if (reader == 0) {
      delete readers;
      return 0;
    }

    // If we are dealing with a directory, and the path ends with a trailing
    // slash, we remove the slash.
    bf::path p = bf::path(*iter);
    if (bf::is_directory(p) && iter->rfind('/') == iter->size() - 1)
      p = bf::path(iter->substr(0, iter->size() - 1));

    // Kill the extension, if there is any.
    p.replace_extension("");

    // Use the last path component as the corpus name.
    std::string name = p.filename().generic_string();


    // Strip extensions

    readers->push_back(name, reader);
  }

  return readers;
}
