#include <AlpinoCorpus/Error.hh>
#include "RemoteCorpusReaderPrivate.hh"
#include "util/GetUrl.hh"
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <cctype>

#include <iostream>  // voor debug, TO DO: weghalen

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
        return EntryIterator(new RemoteIter(&d_entries, 0));
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

    // working...
    CorpusReader::EntryIterator RemoteCorpusReaderPrivate::runXPath(std::string const &query) const
    {
        char buf[4];
        std::string qu = "";
        for (std::string::const_iterator it = query.begin(); it < query.end(); it++) {
            if (isalnum(*it) || *it == '-' || *it == '_' || *it == '[' || *it == ']')
                qu += *it;
            else {
                sprintf(buf, "%02x", (int) *it);
                qu += "%";
                qu += buf;
            }
        }
        std::cerr << qu << std::endl;
        util::GetUrl p(d_url + "/entries?query=" + qu);
        std::vector<std::string> *data;
        data = new std::vector<std::string>;
        data->clear();
        boost::algorithm::split(*data, p.body(), boost::algorithm::is_any_of("\n"), boost::algorithm::token_compress_on);
        size_t i = data->size() - 1;
        if (i >= 0 && (*data)[i] == "")
            data->resize(i);
        return EntryIterator(new RemoteIter(data, 0, true));
    }

    CorpusReader::EntryIterator RemoteCorpusReaderPrivate::runXQuery(std::string const &query) const
    {
        return CorpusReader::EntryIterator(0);
    }

    RemoteCorpusReaderPrivate::RemoteIter::RemoteIter(std::vector<std::string> const * i,
                                                      size_t n,
                                                      bool const ownsdata,
                                                      size_t * refcount) :
        d_items(i), d_idx(n), d_size(i->size()), d_ownsdata(ownsdata), d_refcount(refcount)
    {
        if (d_ownsdata) {
            if (d_refcount == 0) {
                d_refcount = new size_t;
                *d_refcount = 1;
            } else
                (*d_refcount)++;
        }
    }

    // done
    RemoteCorpusReaderPrivate::RemoteIter::~RemoteIter()
    {
        if (d_ownsdata) {
            (*d_refcount)--;
            if (*d_refcount == 0) {
                delete d_refcount;
                delete d_items;
            }
        }
    }

    // done
    std::string RemoteCorpusReaderPrivate::RemoteIter::current() const
    {
        if (d_idx >= 0 && d_idx < d_size)
            return (*d_items)[d_idx];
        else
            return "";
    }

    // done
    void RemoteCorpusReaderPrivate::RemoteIter::next()
    {
        d_idx++;
    }

    // done
    bool RemoteCorpusReaderPrivate::RemoteIter::equals(IterImpl const &other) const
    {
        RemoteIter const &that = (RemoteIter const &)other;
        if (d_idx >= d_size and that.d_idx >= that.d_size)
            return true;
        if (d_idx < 0 and that.d_idx < 0)
            return true;
        return d_idx == that.d_idx;
    }

    // done
    CorpusReader::IterImpl *RemoteCorpusReaderPrivate::RemoteIter::copy() const
    {
        if (this->d_ownsdata)
            return new RemoteIter(this->d_items, this->d_idx, true, this->d_refcount);
        else
            return new RemoteIter(this->d_items, this->d_idx);
    }

}   // namespace alpinocorpus
