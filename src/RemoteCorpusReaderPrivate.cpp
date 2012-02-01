#include <config.hh>
#ifdef USE_DBXML
#include <dbxml/DbXml.hpp>
#endif // USE_DBXML
#include <AlpinoCorpus/Error.hh>
#include <AlpinoCorpus/IterImpl.hh>
#include "RemoteCorpusReaderPrivate.hh"
#include "util/GetUrl.hh"
#include "util/url.hh"
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <cctype>
#include <cstdio>

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
        util::GetUrl p1(url.substr(0, i) + "/corpora?plain=1");
        std::vector<std::string> lines;
        std::vector<std::string> words;
        boost::algorithm::split(lines, p1.body(), boost::algorithm::is_any_of("\n"), boost::algorithm::token_compress_on);
        for (i = 0; i < lines.size(); i++) {
            boost::algorithm::split(words, lines[i], boost::algorithm::is_any_of("\t"));
            if (words.size() == 4 && words[0] == d_name) {
                OK = true;
                d_size = std::atoi(words[1].c_str());
            }
        }
        if (! OK)
            throw std::invalid_argument("URL is not a valid corpus: " + d_url);

        d_geturl = new util::GetUrl(d_url + "/entries?plain=1");
    }

    // done
    CorpusReader::EntryIterator RemoteCorpusReaderPrivate::getBegin() const
    {
        return EntryIterator(new RemoteIter(d_geturl, 0));
    }

    // done
    CorpusReader::EntryIterator RemoteCorpusReaderPrivate::getEnd() const
    {
        return EntryIterator(new RemoteIter(d_geturl, -1));
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
        if (d_size < 0)
            throw std::runtime_error("RemoteCorpusReader: size is unknown");

        return d_size;
    }

    bool RemoteCorpusReaderPrivate::validQuery(QueryDialect d, bool variables, std::string const &query) const
    {
#ifdef USE_DBXML
        try {
            DbXml::XmlQueryContext ctx = mgr.createQueryContext();
            mgr.prepare(query, ctx);
        } catch (DbXml::XmlException const &e) {
            return false;
        }
        return true;
#else
        util::GetUrl p(d_url + "/validQuery?query=" + util::toPercentEncoding(query));
        std::string result = p.body();
        return (result.substr(0, 4) == "true");
#endif
    }

    // done
    std::string RemoteCorpusReaderPrivate::readEntry(std::string const &filename) const
    {
        // util::GetUrl p(d_url + "/entry/" + escape(filename));
        util::GetUrl p(d_url + "/entry/" + filename);
        return p.body();
    }

    // TODO: multiple queries (now: only the first is used)
    std::string RemoteCorpusReaderPrivate::readEntryMarkQueries(std::string const &entry,
                                                                std::list<MarkerQuery> const &queries) const
    {
        std::list<MarkerQuery>::const_iterator iter = queries.begin();

        if (iter == queries.end())
            return readEntry(entry);

        util::GetUrl p(d_url + "/entry/" + entry +   // escape(entry) +
                       "?markerQuery=" + util::toPercentEncoding(iter->query) +
                       "&markerAttr=" + util::toPercentEncoding(iter->attr) +
                       "&markerValue=" + util::toPercentEncoding(iter->value));

        ++iter;
        if (iter != queries.end())
            throw Error("RemoteCorpusReaderPrivate: Multiple queries not implemented");

        return p.body();
    }

    // TODO: multiple queries (now: only the first is used)
    CorpusReader::EntryIterator RemoteCorpusReaderPrivate::runQueryWithStylesheet(QueryDialect d, std::string const &q,
                                                                               std::string const &stylesheet,
                                                                               std::list<MarkerQuery> const &markerQueries) const
    {
        std::list<MarkerQuery>::const_iterator iter = markerQueries.begin();

        if (iter == markerQueries.end())
            throw Error("RemoteCorpusReaderPrivate: Missing query");

        util::GetUrl *p = new util::GetUrl(d_url + "/entries?query=" +
                                           util::toPercentEncoding(q) +
                                           "&markerQuery=" + util::toPercentEncoding(iter->query) +
                                           "&markerAttr=" + util::toPercentEncoding(iter->attr) +
                                           "&markerValue=" + util::toPercentEncoding(iter->value) +
                                           "&plain=1&contents=1", stylesheet);

        ++iter;
        if (iter != markerQueries.end())
            throw Error("RemoteCorpusReaderPrivate: Multiple queries not implemented");

        return EntryIterator(new RemoteIter(p, 0, true));
    }


    // TODO: multiple queries (now: only the first is used)
    CorpusReader::EntryIterator RemoteCorpusReaderPrivate::beginWithStylesheet(std::string const &stylesheet,
                                                                               std::list<MarkerQuery> const &markerQueries) const
    {
        std::list<MarkerQuery>::const_iterator iter = markerQueries.begin();

        if (iter == markerQueries.end())
            throw Error("RemoteCorpusReaderPrivate: Missing query");

        util::GetUrl *p = new util::GetUrl(d_url + "/entries" +
                                           "?markerQuery=" + util::toPercentEncoding(iter->query) +
                                           "&markerAttr=" + util::toPercentEncoding(iter->attr) +
                                           "&markerValue=" + util::toPercentEncoding(iter->value) +
                                           "&plain=1&contents=1", stylesheet);

        ++iter;
        if (iter != markerQueries.end())
            throw Error("RemoteCorpusReaderPrivate: Multiple queries not implemented");

        return EntryIterator(new RemoteIter(p, 0, true));


    }

    // done
    CorpusReader::EntryIterator RemoteCorpusReaderPrivate::runXPath(std::string const &query) const
    {
        util::GetUrl *p = new util::GetUrl(d_url + "/entries?query=" +
            util::toPercentEncoding(query) + "&plain=1&contents=1");
        return EntryIterator(new RemoteIter(p, 0, true));
    }

    // done? TODO: klopt dit? (blijkbaar wel)
    CorpusReader::EntryIterator RemoteCorpusReaderPrivate::runXQuery(std::string const &query) const
    {
        return runXPath(query);
    }

    // done
    RemoteCorpusReaderPrivate::RemoteIter::RemoteIter(util::GetUrl * geturl,
                                                      long signed int n,
                                                      bool const ownsdata,
                                                      size_t * refcount) :
        d_idx(n), d_ownsdata(ownsdata), d_refcount(refcount), d_interrupted(false)
    {
        d_geturl = geturl;
        if (d_ownsdata) {
            if (d_refcount == 0) {
                if (n >= 0) {
                    geturl->line(n);
                    if (geturl->eof())
                        d_idx = -1;
                }
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
                delete d_geturl;
            }
        }
    }

    // done
    std::string RemoteCorpusReaderPrivate::RemoteIter::current() const
    {
        if (d_idx >= 0) {
            if (d_ownsdata) {
                std::string s = d_geturl->line(d_idx);
                size_t i = s.find('\t');
                return s.substr(0, i);
            } else {
                return d_geturl->line(d_idx);
            }
        } else
            return "";
    }

    // done
    void RemoteCorpusReaderPrivate::RemoteIter::next()
    {
        if (d_interrupted)
            throw alpinocorpus::IterationInterrupted();
        if (d_idx >= 0) {
            d_idx++;
            d_geturl->line(d_idx);
            if (d_geturl->eof())
                d_idx = -1;
        }
    }

    // done
    bool RemoteCorpusReaderPrivate::RemoteIter::equals(IterImpl const &other) const
    {
        RemoteIter const &that = (RemoteIter const &)other;
        return d_idx == that.d_idx;
    }

    // done
    IterImpl *RemoteCorpusReaderPrivate::RemoteIter::copy() const
    {
        IterImpl * other;
        if (this->d_ownsdata)
            other = new RemoteIter(this->d_geturl, this->d_idx, true, this->d_refcount);
        else
            other = new RemoteIter(this->d_geturl, this->d_idx);
        if (this->d_interrupted)
            other->interrupt();
        return other;

    }

    // done
    void RemoteCorpusReaderPrivate::RemoteIter::interrupt()
    {
        d_interrupted = true;
    }

    // TODO ????? parameter rdr not used, what is this for?
    std::string RemoteCorpusReaderPrivate::RemoteIter::contents(CorpusReader const &rdr) const
    {
        if (d_idx < 0)
            return std::string("");

        if (! d_ownsdata)
            return std::string("");

        std::string s = d_geturl->line(d_idx);
        size_t i = s.find('\t');
        return s.substr(i + 1);
    }

}   // namespace alpinocorpus
