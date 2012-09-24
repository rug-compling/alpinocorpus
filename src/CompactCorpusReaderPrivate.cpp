#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <vector>

#include <AlpinoCorpus/tr1wrap/memory.hh>

#include <boost/config.hpp>
#include <boost/filesystem.hpp>

#if defined(BOOST_HAS_THREADS)
#include <boost/thread/mutex.hpp>
#endif

#include <AlpinoCorpus/Error.hh>

#include "DzIstream.hh"
#include "CompactCorpusReaderPrivate.hh"
#include "util/base64.hh"

namespace {
    char const * const DATA_EXT = ".data.dz";
    char const * const INDEX_EXT = ".index";
}

namespace bf = boost::filesystem;

namespace alpinocorpus {

CompactCorpusReaderPrivate::CompactCorpusReaderPrivate(std::string const &filename)
{
    std::string canonical(filename);
    canonicalize(canonical);
    construct(canonical);
}

    CompactCorpusReaderPrivate::CompactCorpusReaderPrivate(std::string const &dataPath,
    std::string const &indexPath)
{
    std::string canonical(dataPath);
    canonicalize(canonical);
    construct(canonical, dataPath, indexPath);
}

void CompactCorpusReaderPrivate::construct(std::string const &canonical)
{
    std::string dataPath  = canonical + DATA_EXT;
    std::string indexPath = canonical + INDEX_EXT;
    construct(canonical, dataPath, indexPath);
}

void CompactCorpusReaderPrivate::construct(std::string const &canonical,
    std::string const &dataPath,
    std::string const &indexPath)
{
    // XXX race condition up ahead
    bf::path dataP(dataPath);
    if (!bf::is_regular_file(dataP))
        throw OpenError(dataPath, "not a regular file");

    bf::path indexP(indexPath);
    if (!bf::is_regular_file(indexP))
        throw OpenError(indexPath, "not a regular file");

    open(dataPath, indexPath);

    d_name = canonical;
}

CorpusReader::EntryIterator CompactCorpusReaderPrivate::getEntries() const
{
    ItemVector::const_iterator begin(d_indices.begin());
    return EntryIterator(new IndexIter(begin, d_indices.end()));
}

std::string CompactCorpusReaderPrivate::getName() const
{
    return d_name;
}

size_t CompactCorpusReaderPrivate::getSize() const
{
  return d_indices.size();
}

bool endsWith(std::string const &str, std::string const &end)
{
    size_t pos = str.rfind(end);
    
    if (pos == std::string::npos)
        return false;
        
    return (pos + end.size() == str.size());
}
    
/*
 * Canonicalize file name. To be called from constructor.
 */
void CompactCorpusReaderPrivate::canonicalize(std::string &filename)
{
    if (endsWith(filename, DATA_EXT))
        filename = filename.substr(0, filename.size() - 8);
    else if (endsWith(filename, INDEX_EXT))
        filename = filename.substr(0, filename.size() - 6);
    else
        throw OpenError(filename, "not an indexed (.dz) corpus file");
}

IterImpl *CompactCorpusReaderPrivate::IndexIter::copy() const
{
    // No pointer members.
    return new IndexIter(*this);
}

bool CompactCorpusReaderPrivate::IndexIter::hasNext()
{
  return d_iter != d_end;
}

Entry CompactCorpusReaderPrivate::IndexIter::next(CorpusReader const &)
{
    Entry e = {(*d_iter)->name, ""};

    ++d_iter;

    return e;
}

void CompactCorpusReaderPrivate::open(std::string const &dataPath,
    std::string const &indexPath)
{
    std::ifstream indexStream(indexPath.c_str());
    if (!indexStream)
        throw OpenError(indexPath);

    d_dataStream = DzIstreamPtr(new DzIstream(dataPath.c_str()));
    if (!d_dataStream)
        throw OpenError(indexPath);

    // Read indices
    std::string line;
    while(std::getline(indexStream, line))
    {
        std::istringstream iss(line);
        
        std::string name;
        iss >> name;
        
        std::string offset64;
        iss >> offset64;
        size_t offset = util::b64_decode<size_t>(offset64);
        
        std::string size64;
        iss >> size64;
        size_t size = util::b64_decode<size_t>(size64);
        
        IndexItemPtr item(new IndexItem(name, offset, size));
        d_indices.push_back(item);
        d_namedIndices[name] = item;
    }}

std::string CompactCorpusReaderPrivate::readEntry(std::string const &filename) const
{
    IndexMap::const_iterator iter = d_namedIndices.find(filename);
    if (iter == d_namedIndices.end())
        throw Error("CompactCorpusReaderPrivate::read: requesting unknown data!");

#if defined(BOOST_HAS_THREADS)
    boost::mutex::scoped_lock lock(d_readMutex);
#endif
    
    std::vector<unsigned char> data(iter->second->size);
    d_dataStream->seekg(iter->second->offset, std::ios::beg);
    d_dataStream->read(reinterpret_cast<char *>(&data[0]), iter->second->size);


    return std::string(reinterpret_cast<char const *>(&data[0]), data.size());
}

}   // namespace alpinocorpus
