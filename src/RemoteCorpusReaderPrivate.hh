#ifndef ALPINO_REMOTE_CORPUSREADER_PRIVATE_HH
#define ALPINO_REMOTE_CORPUSREADER_PRIVATE_HH

#include <AlpinoCorpus/CorpusReader.hh>
#include <string>

namespace alpinocorpus {

    class RemoteCorpusReaderPrivate : public CorpusReader
    {

    public:

        RemoteCorpusReaderPrivate(std::string const &path);

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
        virtual bool validQuery(QueryDialect d, bool variables, std::string const &q) const;

    private:

    };

}

#endif  // ALPINO_REMOTE_CORPUSREADER_PRIVATE_HH
