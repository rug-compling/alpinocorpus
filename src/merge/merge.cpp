#include <iostream>
#include <string>

#include <boost/filesystem.hpp>
#include <boost/scoped_ptr.hpp>

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/DbCorpusWriter.hh>

namespace ac = alpinocorpus;
namespace bf = boost::filesystem;

int main(int argc, char *argv[])
{
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " dir corpus.dact" << std::endl;
    return 1;
  }

  ac::DbCorpusWriter writer(argv[2], true);

  bf::path searchPath(argv[1]);

  for (bf::recursive_directory_iterator iter(searchPath, bf::symlink_option::recurse);
      iter != bf::recursive_directory_iterator(); ++iter)
  {
    if (iter->path().extension() != ".index" &&
        iter->path().extension() != ".dact")
      continue;

    boost::scoped_ptr<ac::CorpusReader> reader(
      ac::CorpusReader::open(iter->path().native()));

    // Base path for entries 
    bf::path basePath = iter->path();
    basePath.replace_extension("");
    std::string basePathStr = basePath.native();

    basePathStr.erase(0, searchPath.native().size());

    if (basePathStr[0] == '/')
      basePathStr.erase(0, 1);

    basePath = basePathStr;

    for (ac::CorpusReader::EntryIterator entryIter = reader->begin();
        entryIter != reader->end(); ++entryIter) {
      bf::path entry = basePath / *entryIter;

      std::cerr << "Writing " << *entryIter << " as " << entry.native() << std::endl;
      writer.write(entry.native(), reader->read(*entryIter));
    }
  }

}
