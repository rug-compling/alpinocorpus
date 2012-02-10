#ifndef ALPINO_REMOTE_CORPUSREADER_PRIVATE_HH
#define ALPINO_REMOTE_CORPUSREADER_PRIVATE_HH

#include <string>
#include <vector>

#include <boost/tr1/memory.hpp>

#include <config.hh>
#ifdef USE_DBXML
#include <dbxml/DbXml.hpp>
#endif // USE_DBXML

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/IterImpl.hh>
#include <../src/util/GetUrl.hh>

namespace alpinocorpus {

    class RemoteCorpusReaderPrivate : public CorpusReader
    {

#ifdef USE_DBXML
        DbXml::XmlManager   mutable mgr;
#endif

        class RemoteIter : public IterImpl {
        public:
            RemoteIter(std::tr1::shared_ptr<util::GetUrl> geturl,
                long signed int n);
            ~RemoteIter();
            IterImpl *copy() const;
            std::string current() const;
            bool equals(IterImpl const &) const;
            void next();
            void interrupt();
            std::string contents(CorpusReader const &) const;
        private:
            std::tr1::shared_ptr<util::GetUrl> d_geturl;
            size_t d_idx;
            bool d_interrupted;
        };

    public:

        RemoteCorpusReaderPrivate(std::string const &url);
        virtual ~RemoteCorpusReaderPrivate();

        virtual EntryIterator getBegin() const;
        virtual EntryIterator getEnd() const;
        virtual std::string getName() const;
        virtual size_t getSize() const;
        bool validQuery(QueryDialect d, bool variables, std::string const &query) const;
        virtual std::string readEntry(std::string const &filename) const;
        virtual std::string readEntryMarkQueries(std::string const &entry,
                                                 std::list<MarkerQuery> const &queries) const;
        virtual EntryIterator beginWithStylesheet(std::string const &stylesheet,
                                                  std::list<MarkerQuery> const &markerQueries) const;
        virtual EntryIterator runXPath(std::string const &) const;
        virtual EntryIterator runXQuery(std::string const &) const;
        virtual EntryIterator runQueryWithStylesheet(QueryDialect d, std::string const &q,
                                                  std::string const &stylesheet,
                                                  std::list<MarkerQuery> const &markerQueries) const;

    private:

        std::string d_name;
        std::string d_url;
        long int d_size;
        std::vector<std::string> d_entries;
        std::vector<std::string> d_results;

        std::tr1::shared_ptr<util::GetUrl> d_geturl;

    };

}

#endif  // ALPINO_REMOTE_CORPUSREADER_PRIVATE_HH
