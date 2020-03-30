#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include <boost/filesystem.hpp>

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/CorpusReaderFactory.hh>
#include <AlpinoCorpus/MultiCorpusReader.hh>

namespace bf = boost::filesystem;

using alpinocorpus::CorpusReader;
using alpinocorpus::CorpusReaderFactory;
using alpinocorpus::MultiCorpusReader;

std::shared_ptr<CorpusReader> openCorpus(std::string const &path,
    bool recursive)
{
    if (recursive && bf::is_directory(bf::path(path)))
      return std::shared_ptr<CorpusReader>(CorpusReaderFactory::openRecursive(path, false));
    else
      return std::shared_ptr<CorpusReader>(CorpusReaderFactory::open(path));
}

std::shared_ptr<CorpusReader> openCorpora(
    std::vector<std::string>::const_iterator const &pathBegin,    
    std::vector<std::string>::const_iterator const &pathEnd,
    bool recursive)
{
  auto readers = std::make_shared<MultiCorpusReader>();

  for (std::vector<std::string>::const_iterator iter = pathBegin;
      iter != pathEnd; ++iter)
  {
    // If we are dealing with a directory, and the path ends with a trailing
    // slash, we remove the slash.
    bf::path p = bf::path(*iter);

    bool isDir = bf::is_directory(p);

    if (isDir && iter->rfind('/') == iter->size() - 1)
      p = bf::path(iter->substr(0, iter->size() - 1));

    // Kill the extension, if there is any.
    p.replace_extension("");

    // Use the last path component as the corpus name.
    
    std::string name = isDir ? p.generic_string() : p.filename().generic_string();

    readers->push_back(name, *iter, recursive && isDir);
  }

  return readers;
}
