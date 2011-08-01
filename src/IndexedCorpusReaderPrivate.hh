#ifndef ALPINO_INDEXED_CORPUSREADER_PRIVATE_HH
#define ALPINO_INDEXED_CORPUSREADER_PRIVATE_HH

#include <string>
#include <tr1/unordered_map>
#include <tr1/memory>
#include <vector>

#include <boost/config.hpp>

#if defined(BOOST_HAS_THREADS)
#include <boost/thread/mutex.hpp>
#endif

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/DzIstream.hh>

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

class IndexedCorpusReaderPrivate : public CorpusReader
{
    typedef std::tr1::shared_ptr<IndexItem> IndexItemPtr;
    typedef std::tr1::unordered_map<std::string, IndexItemPtr> IndexMap;
    typedef std::tr1::shared_ptr<DzIstream> DzIstreamPtr;
    typedef std::vector<IndexItemPtr> ItemVector;

    class IndexIter : public IterImpl
    {
        ItemVector::const_iterator iter;

      public:
        IndexIter(ItemVector::const_iterator const &i) : iter(i) { }
        std::string current() const;
        bool equals(IterImpl const &) const;
        void next();
    };

public:
    /**
     * Construct from a single file (data or index); the other file is sought
     * for in the same directory.
     */
    IndexedCorpusReaderPrivate(std::string const &path);
    /** Construct from data and index file. */
    IndexedCorpusReaderPrivate(std::string const &dataFilename, std::string const &indexFilename);
    virtual ~IndexedCorpusReaderPrivate() {}

    virtual EntryIterator getBegin() const;
    virtual EntryIterator getEnd() const;
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

#if defined(BOOST_HAS_THREADS)
    mutable boost::mutex d_readMutex;
#endif
};

}

#endif  // ALPINO_INDEXED_CORPUSREADER_PRIVATE_HH
