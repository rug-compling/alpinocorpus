#ifndef ALPINO_COMPACT_CORPUSREADER_PRIVATE_HH
#define ALPINO_COMPACT_CORPUSREADER_PRIVATE_HH

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>

#include <boost/tr1/unordered_map.hpp>

#include <boost/config.hpp>

#if defined(BOOST_HAS_THREADS)
#include <boost/thread/mutex.hpp>
#endif

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/Entry.hh>
#include <AlpinoCorpus/IterImpl.hh>

#include "DzIstream.hh"

namespace alpinocorpus
{
    struct IndexItem
{
    IndexItem(std::string const &newName, size_t newOffset, size_t newSize)
     : name(newName), offset(newOffset), size(newSize) {}
    IndexItem() : name(""), offset(0), size(0) {}

    std::string name;
    size_t offset;
    size_t size;
};

class CompactCorpusReaderPrivate : public CorpusReader
{
    typedef boost::shared_ptr<IndexItem> IndexItemPtr;
    typedef std::tr1::unordered_map<std::string, IndexItemPtr> IndexMap;
    typedef boost::shared_ptr<DzIstream> DzIstreamPtr;
    typedef std::vector<IndexItemPtr> ItemVector;

    class IndexIter : public IterImpl
    {
        ItemVector::const_iterator d_iter;
        ItemVector::const_iterator d_end;

    public:
        IndexIter(ItemVector::const_iterator i,
            ItemVector::const_iterator const end) : d_iter(i), d_end(end) { }
        IterImpl *copy() const;
        bool hasNext();
        Entry next(CorpusReader const &rdr);
    };

public:
    /**
     * Construct from a single file (data or index); the other file is sought
     * for in the same directory.
     */
    CompactCorpusReaderPrivate(std::string const &path);
    /** Construct from data and index file. */
    CompactCorpusReaderPrivate(std::string const &dataFilename, std::string const &indexFilename);
    virtual ~CompactCorpusReaderPrivate() {}

    virtual EntryIterator getEntries() const;
    virtual std::string getName() const;
    virtual std::string readEntry(std::string const &filename) const;
    virtual size_t getSize() const;

private:
    static void canonicalize(std::string &);
    void construct(std::string const &);
    void construct(std::string const &, std::string const &, std::string const &);
    void open(std::string const &, std::string const &);
	
    DzIstreamPtr d_dataStream;
    std::vector<IndexItemPtr> d_indices;
	IndexMap d_namedIndices;
    std::string d_name;

#if defined(BOOST_HAS_THREADS)
    mutable boost::mutex d_readMutex;
#endif
};

}

#endif  // ALPINO_COMPACT_CORPUSREADER_PRIVATE_HH
