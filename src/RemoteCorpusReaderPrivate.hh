#ifndef ALPINO_REMOTE_CORPUSREADER_PRIVATE_HH
#define ALPINO_REMOTE_CORPUSREADER_PRIVATE_HH

#include <config.hh>
#ifdef USE_DBXML
#include <dbxml/DbXml.hpp>
#endif // USE_DBXML

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/IterImpl.hh>
#include <../src/util/GetUrl.hh>
#include <string>
#include <vector>

namespace alpinocorpus {

    class RemoteCorpusReaderPrivate : public CorpusReader
    {

#ifdef USE_DBXML
        DbXml::XmlManager   mutable mgr;
#endif

        class RemoteIter : public IterImpl {
        public:
            RemoteIter(util::GetUrl * geturl, long signed int n,
                       bool ownsdata = false, size_t * refcount = 0);
            ~RemoteIter();
            IterImpl *copy() const;
            std::string current() const;
            bool equals(IterImpl const &) const;
            void next();
            void interrupt();
            std::string contents(CorpusReader const &) const;
        private:
            util::GetUrl *d_geturl;
            size_t *d_refcount;
            size_t d_idx;
            bool const d_ownsdata;
            bool d_interrupted;
        };

    public:

        RemoteCorpusReaderPrivate(std::string const &url);

        virtual ~RemoteCorpusReaderPrivate() { delete d_geturl; }

        virtual EntryIterator getBegin() const;
        virtual EntryIterator getEnd() const;
        virtual std::string getName() const;
        virtual size_t getSize() const;
        bool validQuery(QueryDialect d, bool variables, std::string const &query) const;
        virtual std::string readEntry(std::string const &filename) const;
        virtual std::string readEntryMarkQueries(std::string const &entry,
                                                 std::list<MarkerQuery> const &queries) const;
        virtual EntryIterator runXPath(std::string const &) const;
        virtual EntryIterator runXQuery(std::string const &) const;

    private:

        std::string d_name;
        std::string d_url;
        long int d_size;
        std::vector<std::string> d_entries;
        std::vector<std::string> d_results;

        util::GetUrl *d_geturl;

    };

}

#endif  // ALPINO_REMOTE_CORPUSREADER_PRIVATE_HH
