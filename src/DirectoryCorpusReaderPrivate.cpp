#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QtDebug>

#include <algorithm>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <typeinfo>

#include <boost/filesystem.hpp>

#include <AlpinoCorpus/Error.hh>
#include <util/textfile.hh>

#include "DirectoryCorpusReaderPrivate.hh"

namespace bf = boost::filesystem;

namespace alpinocorpus {

DirectoryCorpusReaderPrivate::DirectoryCorpusReaderPrivate(
    std::string const &directory, bool wantCache)
{
    if (directory[directory.size() - 1] == '/')
        d_directory = bf::path(directory).parent_path();
    else
        d_directory = bf::path(directory);
    
    if (!boost::filesystem::exists(d_directory) ||
        !boost::filesystem::is_directory(d_directory))
        throw OpenError(directory, "non-existent or not a directory");
    
    if (!wantCache || !readCache()) {
        for (bf::recursive_directory_iterator iter(d_directory, bf::symlink_option::recurse);
             iter != bf::recursive_directory_iterator();
             ++iter)
        {
            if (iter->path().extension() != ".xml")
                continue;
            
            std::string entryPathStr = iter->path().native();
            entryPathStr.erase(0, d_directory.native().size());
            
            if (entryPathStr[0] == '/')
                entryPathStr.erase(0, 1);
            
            bf::path entryPath(entryPathStr);
            
            d_entries.push_back(entryPath.native());
        }
    }

    if (wantCache)
        writeCache();

    setName(directory);
}

DirectoryCorpusReaderPrivate::~DirectoryCorpusReaderPrivate()
{}

CorpusReader::EntryIterator DirectoryCorpusReaderPrivate::getBegin() const
{
    return EntryIterator(new DirIter(d_entries.begin()));
}

CorpusReader::EntryIterator DirectoryCorpusReaderPrivate::getEnd() const
{
    return EntryIterator(new DirIter(d_entries.end()));
}

std::string DirectoryCorpusReaderPrivate::DirIter::current() const
{
    return *iter; // XXX - native separators
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

void DirectoryCorpusReaderPrivate::DirIter::next()
{
    ++iter;
}

std::string DirectoryCorpusReaderPrivate::readEntry(std::string const &entry) const
{
    bf::path p(d_directory);
    p /= entry;
    return util::readFile(p.native());
}

bf::path DirectoryCorpusReaderPrivate::cachePath() const
{
    // XXX: putting the index outside the directory
    // is a fundamental design flaw. --Lars
    return d_directory.parent_path() / d_directory.filename().replace_extension(".dir_index");

}

/**
 * Read directory cache file. Returns true on success.
 */
bool DirectoryCorpusReaderPrivate::readCache()
{
    bf::path cacheP(cachePath());
    
    if (!bf::exists(cacheP) ||
        bf::last_write_time(d_directory) > bf::last_write_time(cacheP))
        return false;
    
    std::ifstream cache(cacheP.native().c_str());
    if (!cache)
        return false;
    
    std::string line;
    while (std::getline(cache, line)) {
        d_entries.push_back(line);
    }
    

    if (cache.eof())
        return true;
    else {      // I/O error occurred
        d_entries.clear();
        return false;
    }
}

void DirectoryCorpusReaderPrivate::writeCache()
{
    std::ofstream cache(cachePath().native().c_str());

    if (!cache)
        return;

    std::copy(d_entries.begin(), d_entries.end(), std::ostream_iterator<std::string>(cache, "\n"));
}

}   // namespace alpinocorpus
