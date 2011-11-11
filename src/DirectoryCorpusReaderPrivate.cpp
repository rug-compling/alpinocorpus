#include <algorithm>
#include <fstream>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <string>
#include <typeinfo>

#include <boost/filesystem.hpp>

#include <AlpinoCorpus/Error.hh>
#include <util/textfile.hh>

#include "DirectoryCorpusReaderPrivate.hh"

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

CorpusReader::EntryIterator DirectoryCorpusReaderPrivate::getBegin() const
{
    return EntryIterator(new DirIter(d_directory,
        bf::recursive_directory_iterator(d_directory, bf::symlink_option::recurse)));
}

CorpusReader::EntryIterator DirectoryCorpusReaderPrivate::getEnd() const
{
    return EntryIterator(new DirIter(d_directory,
        bf::recursive_directory_iterator()));
}

std::string DirectoryCorpusReaderPrivate::getName() const
{
  return d_directory.string();
}

size_t DirectoryCorpusReaderPrivate::getSize() const
{
    if (d_nEntries == std::numeric_limits<size_t>::max())
        d_nEntries = std::distance(getBegin(), getEnd());
    
    return d_nEntries;
}


DirectoryCorpusReaderPrivate::DirIter::DirIter(
    bf::path const &path, bf::recursive_directory_iterator i) :
    d_directory(path), iter(i)
{
    if (!isValid())
        next();
}


std::string DirectoryCorpusReaderPrivate::DirIter::current() const
{
    std::string entryPathStr = iter->path().string();
    entryPathStr.erase(0, d_directory.string().size());

    if (entryPathStr[0] == '/')
        entryPathStr.erase(0, 1);

    bf::path entryPath(entryPathStr);

    return entryPath.string();
}

bool DirectoryCorpusReaderPrivate::DirIter::equals(IterImpl const &other) const
{
    try {
        DirIter const &that = dynamic_cast<DirIter const &>(other);
        return iter == that.iter;
    } catch (std::bad_cast const &) {
        return false;
    }
}

bool DirectoryCorpusReaderPrivate::DirIter::isValid()
{
    // End is a correct iterator state.
    if (iter == bf::recursive_directory_iterator())
        return true;

    return iter->path().extension() == ".xml";
}

void DirectoryCorpusReaderPrivate::DirIter::next()
{
    // Don't recurse past end.
    //if (iter == bf::recursive_directory_iterator())
    //    return;

    do {
        ++iter;
    } while (!isValid());
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
