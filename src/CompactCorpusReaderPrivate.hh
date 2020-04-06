#ifndef ALPINO_COMPACT_CORPUSREADER_PRIVATE_HH
#define ALPINO_COMPACT_CORPUSREADER_PRIVATE_HH

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

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
    typedef std::shared_ptr<IndexItem> IndexItemPtr;
    typedef std::unordered_map<std::string, IndexItemPtr> IndexMap;
    typedef std::shared_ptr<DzIstream> DzIstreamPtr;
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

    virtual EntryIterator getEntries(SortOrder sortOrder) const;
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

    mutable std::mutex d_readMutex;
};

}

#endif  // ALPINO_COMPACT_CORPUSREADER_PRIVATE_HH
