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

namespace {
    class DirIter : public alpinocorpus::IterImpl
    {
        boost::filesystem::recursive_directory_iterator iter;
        boost::filesystem::path d_directory;

      public:
        DirIter(boost::filesystem::path const &path,
            boost::filesystem::recursive_directory_iterator i);
        alpinocorpus::IterImpl *copy() const;
        bool hasNext();
        alpinocorpus::Entry next(alpinocorpus::CorpusReader const &rdr);
      private:
        bool isValid();
    };

    DirIter::DirIter(
        bf::path const &path, bf::recursive_directory_iterator i) :
        d_directory(path), iter(i)
    {
    }

    alpinocorpus::IterImpl *DirIter::copy() const
    {
        // No pointer members
        return new DirIter(*this);
    }

    bool DirIter::isValid()
    {
        // End is a correct iterator state.
        if (iter == bf::recursive_directory_iterator())
            return true;

        return iter->path().extension() == ".xml";
    }

    bool DirIter::hasNext()
    {
        // Position iterator at the next valid entry
        while (!isValid()) {
          ++iter;
        }

        return iter != bf::recursive_directory_iterator();
    }

    alpinocorpus::Entry DirIter::next(alpinocorpus::CorpusReader const &rdr)
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

        alpinocorpus::Entry entry = {entryPath.string(), ""};

        return entry;
    }

    class SortedDirIter : public alpinocorpus::IterImpl
    {
        std::vector<bf::path> d_entries;
        std::vector<bf::path>::const_iterator d_iter;
        boost::filesystem::path d_directory;

      public:
        SortedDirIter(boost::filesystem::path const &path,
            boost::filesystem::recursive_directory_iterator i);
        alpinocorpus::IterImpl *copy() const;
        bool hasNext();
        alpinocorpus::Entry next(alpinocorpus::CorpusReader const &rdr);
      private:
        SortedDirIter();
        SortedDirIter &operator=(SortedDirIter &other);
        bool isValid();
    };

    SortedDirIter::SortedDirIter(
        bf::path const &path, bf::recursive_directory_iterator i) :
        d_directory(path)
    {
        std::copy(i, bf::recursive_directory_iterator(),
            std::back_inserter(d_entries));

        d_iter = d_entries.begin();
    }

    SortedDirIter::SortedDirIter() {
    }

    alpinocorpus::IterImpl *SortedDirIter::copy() const
    {
      SortedDirIter *dirIter = new SortedDirIter;
      dirIter->d_entries = d_entries;
      dirIter->d_directory = d_directory;
      dirIter->d_iter = dirIter->d_entries.begin() +
          std::distance(d_entries.begin(), d_iter);

      return dirIter;
    }

    bool SortedDirIter::isValid()
    {
        // End is a correct iterator state.
        if (d_iter == d_entries.end())
            return true;

        return d_iter->extension() == ".xml";
    }

    bool SortedDirIter::hasNext()
    {
        // Position iterator at the next valid entry
        while (!isValid()) {
          ++d_iter;
        }

        return d_iter != d_entries.end();
    }

    alpinocorpus::Entry SortedDirIter::next(alpinocorpus::CorpusReader const &rdr)
    {
        // We assume the iterator is valid, since hasNext() should be called
        // before next().

        std::string entryPathStr = d_iter->string();
        entryPathStr.erase(0, d_directory.string().size());

        if (entryPathStr[0] == '/')
            entryPathStr.erase(0, 1);

        bf::path entryPath(entryPathStr);

        // Move the iterator.
        ++d_iter;

        alpinocorpus::Entry entry = {entryPath.string(), ""};

        return entry;
    }

}

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

CorpusReader::EntryIterator DirectoryCorpusReaderPrivate::getEntries(SortOrder sortOrder) const
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

      EntryIterator i = getEntries(NaturalOrder);
      while (i.hasNext())
      {
        ++nEntries;
        i.next(*this);
      }

      d_nEntries = nEntries;
    }
    
    return d_nEntries;
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
