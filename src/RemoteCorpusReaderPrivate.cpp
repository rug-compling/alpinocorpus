#include <AlpinoCorpus/Error.hh>
#include "RemoteCorpusReaderPrivate.hh"
#include "util/GetUrl.hh"
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

namespace alpinocorpus {

    // done

    //! url is actual url#name
    //  example: http://localhost:8123/cdb#cdb
    RemoteCorpusReaderPrivate::RemoteCorpusReaderPrivate(std::string const &url)
    {
        if (url.substr(0, 7) != "http://" && url.substr(0, 8) != "https://")
            throw OpenError(url, "Not a valid URL");

        size_t i = url.find_last_of('/');
        if (i == std::string::npos)
            throw std::invalid_argument("Invalid argument, must be http://host/name");
        d_name = url.substr(i + 1);
        d_url = url;

        util::GetUrl p(d_url + "/entries");
        d_entries.clear();
        boost::algorithm::split(d_entries, p.body(), boost::algorithm::is_any_of("\n"), boost::algorithm::token_compress_on);
        i = d_entries.size() - 1;
        if (i >= 0 && d_entries[i] == "")
            d_entries.resize(i);
        if (i < 1)
            throw std::runtime_error("No entries found at " + d_url + "/entries");
    }

    CorpusReader::EntryIterator RemoteCorpusReaderPrivate::getBegin() const
    {
        return 0;
    }

    CorpusReader::EntryIterator RemoteCorpusReaderPrivate::getEnd() const
    {
        return 0;
    }

    // done
    std::string RemoteCorpusReaderPrivate::getName() const
    {
        return d_name;
    }

    // done
    size_t RemoteCorpusReaderPrivate::getSize() const
    {
        return d_entries.size();
    }

    std::string RemoteCorpusReaderPrivate::readEntry(std::string const &filename) const
    {
        return std::string("");
    }

    bool RemoteCorpusReaderPrivate::validQuery(QueryDialect d, bool variables, std::string const &query) const
    {
        return true;
    }

    std::string RemoteCorpusReaderPrivate::readEntryMarkQueries(std::string const &entry,
                                                                std::list<MarkerQuery> const &queries) const
    {
        return std::string("");
    }

    CorpusReader::EntryIterator RemoteCorpusReaderPrivate::runXPath(std::string const &query) const
    {
        return CorpusReader::EntryIterator(0);
        // return runXQuery(std::string("collection('corpus')" + query));
    }

    CorpusReader::EntryIterator RemoteCorpusReaderPrivate::runXQuery(std::string const &query) const
    {
        return CorpusReader::EntryIterator(0);
    }


}   // namespace alpinocorpus
