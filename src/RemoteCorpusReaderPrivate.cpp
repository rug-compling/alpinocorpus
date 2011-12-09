#include <AlpinoCorpus/Error.hh>
#include "RemoteCorpusReaderPrivate.hh"
#include "util/GetUrl.hh"
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

namespace alpinocorpus {

    // done
    RemoteCorpusReaderPrivate::RemoteCorpusReaderPrivate(std::string const &url)
    {
        if (url.substr(0, 7) != "http://" && url.substr(0, 8) != "https://")
            throw OpenError(url, "Not a valid URL");

        size_t i = url.find_last_of('/');
        if (i == std::string::npos)
            throw std::invalid_argument("Invalid argument, must be http://host/name");
        d_name = url.substr(i + 1);
        d_url = url;

        bool OK = false;
        util::GetUrl p1(url.substr(0, i + 1)); // TODO: met of zonder '/' aan het eind?
        std::vector<std::string> lines;
        std::vector<std::string> words;
        boost::algorithm::split(lines, p1.body(), boost::algorithm::is_any_of("\n"), boost::algorithm::token_compress_on);
        for (i = 0; i < lines.size(); i++) {
            boost::algorithm::split(words, lines[i], boost::algorithm::is_any_of("\t"));
            if (words.size() == 4 && words[0] == d_name)
                OK = true;
        }
        if (! OK)
            throw std::invalid_argument("URL is not a valid corpus: " + d_url);

        util::GetUrl p(d_url + "/entries");
        d_entries.clear();
        boost::algorithm::split(d_entries, p.body(), boost::algorithm::is_any_of("\n"), boost::algorithm::token_compress_on);
        i = d_entries.size() - 1;
        if (i >= 0 && d_entries[i] == "")
            d_entries.resize(i);
        if (i < 1)
            throw std::runtime_error("No entries found at " + d_url + "/entries");
    }

    // done
    CorpusReader::EntryIterator RemoteCorpusReaderPrivate::getBegin() const
    {
        return EntryIterator(new RemoteIter(&d_entries, (size_t) 0));
    }

    // done
    CorpusReader::EntryIterator RemoteCorpusReaderPrivate::getEnd() const
    {
        return EntryIterator(new RemoteIter(&d_entries, d_entries.size()));
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

    // done
    std::string RemoteCorpusReaderPrivate::readEntry(std::string const &filename) const
    {
        util::GetUrl p(d_url + "/entry/" + filename);
        return p.body();
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

    // done
    RemoteCorpusReaderPrivate::RemoteIter::~RemoteIter()
    {
    }

    // done
    std::string RemoteCorpusReaderPrivate::RemoteIter::current() const
    {
        if (idx >= 0 && idx < size)
            return (*items)[idx];
        else
            return "";
    }

    // done
    void RemoteCorpusReaderPrivate::RemoteIter::next()
    {
        idx++;
    }

    // done
    bool RemoteCorpusReaderPrivate::RemoteIter::equals(IterImpl const &other) const
    {
        RemoteIter const &that = (RemoteIter const &)other;
        if (idx >= size and that.idx >= that.size)
            return true;
        if (idx < 0 and that.idx < 0)
            return true;
        return idx == that.idx;
    }

    // done
    CorpusReader::IterImpl *RemoteCorpusReaderPrivate::RemoteIter::copy() const
    {
        return new RemoteIter(this->items, this->idx);
    }

}   // namespace alpinocorpus
