#include <AlpinoCorpus/Error.hh>
#include "RemoteCorpusReaderPrivate.hh"

namespace alpinocorpus {

    RemoteCorpusReaderPrivate::RemoteCorpusReaderPrivate(std::string const &filename)
    {

    }

    CorpusReader::EntryIterator RemoteCorpusReaderPrivate::getBegin() const
    {
        return 0;
    }

    CorpusReader::EntryIterator RemoteCorpusReaderPrivate::getEnd() const
    {
        return 0;
    }

    std::string RemoteCorpusReaderPrivate::getName() const
    {
        return std::string("");
    }

    size_t RemoteCorpusReaderPrivate::getSize() const
    {
        return 0;
    }

    std::string RemoteCorpusReaderPrivate::readEntry(std::string const &filename) const
    {
        return std::string("");
    }

}   // namespace alpinocorpus
