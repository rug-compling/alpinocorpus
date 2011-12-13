#include <AlpinoCorpus/Error.hh>
#include "RemoteCorpusReaderPrivate.hh"
#include "util/GetUrl.hh"
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <cctype>

// TODO: remove next line and all lines with std::cerr
#include <iostream>

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

    // done? TODO: alleen naam van corpus of complete url? (nu: complete url)
    std::string RemoteCorpusReaderPrivate::getName() const
    {
        // return d_name;
        return d_url;
    }

    // done
    size_t RemoteCorpusReaderPrivate::getSize() const
    {
        return d_entries.size();
    }

    // done
    std::string RemoteCorpusReaderPrivate::readEntry(std::string const &filename) const
    {
        util::GetUrl p(d_url + "/entry/" + escape(filename));
        return p.body();
    }

    // TODO: multiple queries (now: only the first is used)
    std::string RemoteCorpusReaderPrivate::readEntryMarkQueries(std::string const &entry,
                                                                std::list<MarkerQuery> const &queries) const
    {
        std::list<MarkerQuery>::const_iterator iter = queries.begin();

        if (iter == queries.end())
            return readEntry(entry);

        std::cerr << "RemoteCorpusReaderPrivate::readEntryMarkQueries(" << entry << ", (" << iter->query << ", " << iter->attr << ", " << iter->value << "))" << std::endl;

        util::GetUrl p(d_url + "/entry/" + escape(entry) +
                       "?markerQuery=" + escape(iter->query) +
                       "&markerAttr=" + escape(iter->attr) +
                       "&markerValue=" + escape(iter->value));

        ++iter;
        if (iter != queries.end())
            throw Error("RemoteCorpusReaderPrivate: Multiple queries not implemented");

        return p.body();
    }

    // done
    CorpusReader::EntryIterator RemoteCorpusReaderPrivate::runXPath(std::string const &query) const
    {
        util::GetUrl p(d_url + "/entries?query=" + escape(query) + "&unique=1");
        std::vector<std::string> *data;
        data = new std::vector<std::string>;
        data->clear();
        boost::algorithm::split(*data, p.body(), boost::algorithm::is_any_of("\n"), boost::algorithm::token_compress_on);
        size_t i = data->size() - 1;
        if (i >= 0 && (*data)[i] == "")
            data->resize(i);
        return EntryIterator(new RemoteIter(data, 0, true, query));
    }

    // done? TODO: klopt dit? (blijkbaar wel)
    CorpusReader::EntryIterator RemoteCorpusReaderPrivate::runXQuery(std::string const &query) const
    {
        return runXPath(query);
    }

    // done
    RemoteCorpusReaderPrivate::RemoteIter::RemoteIter(std::vector<std::string> const * i,
                                                      size_t n,
                                                      bool const ownsdata,
                                                      std::string const & query,
                                                      size_t * refcount) :
        d_items(i), d_idx(n), d_size(i->size()), d_ownsdata(ownsdata), d_query(query), d_refcount(refcount)
    {
        if (d_ownsdata) {
            if (d_refcount == 0) {
                d_refcount = new size_t;
                *d_refcount = 1;
                std::cerr << "RemoteCorpusReaderPrivate::RemoteIter::RemoteIter query=" << query << std::endl;
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
            return new RemoteIter(this->d_items, this->d_idx, true, this->d_query, this->d_refcount);
        else
            return new RemoteIter(this->d_items, this->d_idx);
    }

    // done
    void RemoteCorpusReaderPrivate::RemoteIter::interrupt()
    {
        d_idx = d_size;
    }

    // BUSY
    std::string RemoteCorpusReaderPrivate::RemoteIter::contents(CorpusReader const &rdr) const
    {
        if (d_idx < 0 || d_idx >= d_size)
            return std::string("");

        if (! d_ownsdata)
            return std::string("");

        size_t i = d_query.rfind("/@");
        std::string q1 = d_query.substr(0, i);
        std::string q2 = " " + d_query.substr(i + 2) + "=\"";

        std::list<CorpusReader::MarkerQuery> queries;
        queries.push_back(CorpusReader::MarkerQuery(q1, "active", "1"));

        std::string p = rdr.readMarkQueries((*d_items)[d_idx], queries);

        std::vector<std::string> lines;
        boost::algorithm::split(lines, p, boost::algorithm::is_any_of("\n"));

        for (std::vector<std::string>::iterator it = lines.begin(); it != lines.end(); it++) {
            size_t i = it->find(" active=\"1\"");
            if (i < it->size()) {
                i = it->find(q2);
                if (i < it->size()) {
                    size_t i1 = i + q2.size();
                    size_t i2 = it->find("\"", i1);
                    return it->substr(i1, i2 - i1);
                }
            }
        }

        return std::string("");
    }

    // done
    std::string RemoteCorpusReaderPrivate::escape(std::string const &s) const
    {
        char buf[4];
        std::string s2 = "";
        for (std::string::const_iterator it = s.begin(); it < s.end(); it++) {
            if (isalnum(*it) || *it == '.' || *it == '-' || *it == '_' || *it == '[' || *it == ']')
                s2 += *it;
            else {
                sprintf(buf, "%02x", (int) *it);
                s2 += "%";
                s2 += buf;
            }
        }
        return s2;
    }


}   // namespace alpinocorpus
