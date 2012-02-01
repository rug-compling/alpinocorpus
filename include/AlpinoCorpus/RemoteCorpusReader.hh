#ifndef ALPINO_REMOTECORPUSREADER_HH
#define ALPINO_REMOTECORPUSREADER_HH

#include <list>
#include <string>

#include <AlpinoCorpus/CorpusReader.hh>

namespace alpinocorpus {

    class RemoteCorpusReaderPrivate;
    
    /**
     * Remote corpus reader.
     */
    class RemoteCorpusReader : public CorpusReader
    {
    public:
        RemoteCorpusReader(std::string const &);
        virtual ~RemoteCorpusReader();

    private:
        bool validQuery(QueryDialect d, bool variables, std::string const &query) const;
        EntryIterator getBegin() const;
        EntryIterator getEnd() const;
        std::string getName() const;
        std::string readEntry(std::string const &) const;
        std::string readEntryMarkQueries(std::string const &entry, std::list<MarkerQuery> const &queries) const;
        EntryIterator queryWithStylesheet(QueryDialect d, std::string const &q,
                                          std::string const &stylesheet,
                                          std::list<MarkerQuery> const &markerQueries) const;
        EntryIterator beginWithStylesheet(std::string const &stylesheet, std::list<MarkerQuery> const &markerQueries) const;
        EntryIterator runXPath(std::string const &) const;
        EntryIterator runXQuery(std::string const &) const;
        size_t getSize() const;
    
        RemoteCorpusReaderPrivate *d_private;
    };

}   // namespace alpinocorpus

#endif
