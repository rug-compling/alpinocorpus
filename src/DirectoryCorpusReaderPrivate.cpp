#include <algorithm>
#include <fstream>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <string>
#include <typeinfo>

#include <boost/filesystem.hpp>

#include <AlpinoCorpus/Error.hh>
#include <AlpinoCorpus/Entry.hh>
#include <AlpinoCorpus/IterImpl.hh>

#include "DirectoryCorpusReaderPrivate.hh"
#include "util/textfile.hh"

namespace bf = boost::filesystem;

namespace alpinocorpus {

DirectoryCorpusReaderPrivate::DirectoryCorpusReaderPrivate(
    std::string const &directory) :
    d_nEntries(std::numeric_limits<size_t>::max())
{
    if (directory[directory.size() - 1] == '/')
        d_directory = bf::path(directory).parent_path();
    else
        d_directory = bf::path(directory);
    
    if (!bf::exists(d_directory) ||
        !bf::is_directory(d_directory))
        throw OpenError(directory, "non-existent or not a directory");
}

DirectoryCorpusReaderPrivate::~DirectoryCorpusReaderPrivate()
{}

CorpusReader::EntryIterator DirectoryCorpusReaderPrivate::getEntries() const
{
    return EntryIterator(new DirIter(d_directory,
        bf::recursive_directory_iterator(d_directory, bf::symlink_option::recurse)));
}

std::string DirectoryCorpusReaderPrivate::getName() const
{
  return d_directory.string();
}

size_t DirectoryCorpusReaderPrivate::getSize() const
{
    if (d_nEntries == std::numeric_limits<size_t>::max())
    {
      size_t nEntries = 0;

      EntryIterator i = getEntries();
      while (i.hasNext())
      {
        ++nEntries;
        i.next(*this);
      }

      d_nEntries = nEntries;
    }
    
    return d_nEntries;
}


DirectoryCorpusReaderPrivate::DirIter::DirIter(
    bf::path const &path, bf::recursive_directory_iterator i) :
    d_directory(path), iter(i)
{
    //if (!isValid())
    //    next();
}

IterImpl *DirectoryCorpusReaderPrivate::DirIter::copy() const
{
    // No pointer members
    return new DirIter(*this);
}

bool DirectoryCorpusReaderPrivate::DirIter::isValid()
{
    // End is a correct iterator state.
    if (iter == bf::recursive_directory_iterator())
        return true;

    return iter->path().extension() == ".xml";
}

bool DirectoryCorpusReaderPrivate::DirIter::hasNext()
{
    // Position iterator at the next valid entry
    while (!isValid()) {
      ++iter;
    }

    return iter != bf::recursive_directory_iterator();
}

Entry DirectoryCorpusReaderPrivate::DirIter::next(CorpusReader const &rdr)
{
    // We assume the iterator is valid, since hasNext() should be called
    // before next().

    std::string entryPathStr = iter->path().string();
    entryPathStr.erase(0, d_directory.string().size());

    if (entryPathStr[0] == '/')
        entryPathStr.erase(0, 1);

    bf::path entryPath(entryPathStr);

    // Move the iterator.
    ++iter;

    Entry entry(entryPath.string(), "");

    return entry;
}

std::string DirectoryCorpusReaderPrivate::readEntry(std::string const &entry) const
{
    bf::path p(d_directory);
    p /= entry;
    return util::readFile(p.string());
}

bf::path DirectoryCorpusReaderPrivate::cachePath() const
{
    return d_directory.parent_path() / d_directory.filename().replace_extension(".dir_index");
}

}   // namespace alpinocorpus
