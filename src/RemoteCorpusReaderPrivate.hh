#ifndef ALPINO_REMOTE_CORPUSREADER_PRIVATE_HH
#define ALPINO_REMOTE_CORPUSREADER_PRIVATE_HH

#include <AlpinoCorpus/CorpusReader.hh>
#include <string>
#include <vector>

namespace alpinocorpus {

    class RemoteCorpusReaderPrivate : public CorpusReader
    {

        class RemoteIter : public CorpusReader::IterImpl {
        public:
            RemoteIter(std::vector<std::string> const * i, size_t n) : items(i),  idx(n) { }
            ~RemoteIter(); // if missing I get: undefined reference to `vtable for...
            IterImpl *copy() const;
            std::string current() const;
            bool equals(IterImpl const &) const;
            void next();
        private:
            std::vector<std::string> const *items;
            size_t idx;
        };

    public:

        RemoteCorpusReaderPrivate(std::string const &url);

        virtual ~RemoteCorpusReaderPrivate() {}

        virtual EntryIterator getBegin() const;
        virtual EntryIterator getEnd() const;
        virtual std::string getName() const;
        virtual size_t getSize() const;
        virtual std::string readEntry(std::string const &filename) const;
        virtual std::string readEntryMarkQueries(std::string const &entry,
                                                 std::list<MarkerQuery> const &queries) const;
        virtual EntryIterator runXPath(std::string const &) const;
        virtual EntryIterator runXQuery(std::string const &) const;

    private:
        std::string d_name;
        std::string d_url;
        std::vector<std::string> d_entries;
    };

}

#endif  // ALPINO_REMOTE_CORPUSREADER_PRIVATE_HH
