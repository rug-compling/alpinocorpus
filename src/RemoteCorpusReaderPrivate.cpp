//#define RemCorReaPri_DEBUG

#include <stdexcept>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#include <config.hh>

#ifdef USE_DBXML
#include <dbxml/DbXml.hpp>
#endif // USE_DBXML

#include <AlpinoCorpus/Error.hh>
#include <AlpinoCorpus/IterImpl.hh>
#include "RemoteCorpusReaderPrivate.hh"
#include "util/GetUrl.hh"
#include "util/parseString.hh"
#include "util/url.hh"

#ifdef RemCorReaPri_DEBUG
#include <iostream>
#endif

namespace alpinocorpus {

    // done
    RemoteCorpusReaderPrivate::RemoteCorpusReaderPrivate(std::string const &url)
        : d_validSize(true)
    {
        if (url.substr(0, 7) != "http://" && url.substr(0, 8) != "https://")
            throw OpenError(url, "Not a valid URL");

        size_t i = url.find_last_of('/');
        if (i == std::string::npos)
            throw std::invalid_argument("Invalid argument, must be http://host/name");
        d_name = url.substr(i + 1);
        d_url = url;

        bool OK = false;
        util::GetUrl p1(url.substr(0, i) + "/corpora");
        std::vector<std::string> lines;
        std::vector<std::string> words;
        boost::algorithm::split(lines, p1.body(), boost::algorithm::is_any_of("\n"), boost::algorithm::token_compress_on);
        for (std::vector<std::string>::const_iterator iter = lines.begin();
            iter != lines.end(); ++iter)
        {
            boost::algorithm::split(words, *iter, boost::algorithm::is_any_of("\t"));
            if (words.size() == 4 && words[0] == d_name) {
                OK = true;
                try {
                    d_size = util::parseString<size_t>(words[1]);
                } catch (std::invalid_argument &) {
                    d_size = 0;
                    d_validSize = false;
                }
            }
        }
        if (!OK)
            throw std::invalid_argument("URL is not a valid corpus: " + d_url);

        // why a reset here?
        d_geturl.reset(new util::GetUrl(d_url + "/entries"));
    }

    RemoteCorpusReaderPrivate::~RemoteCorpusReaderPrivate()
    {
    }

    // done
    CorpusReader::EntryIterator RemoteCorpusReaderPrivate::getEntries() const
    {
        if (d_geturl->interrupted() && ! d_geturl->completed())
            const_cast<RemoteCorpusReaderPrivate *>(this)->d_geturl->resume();

        return EntryIterator(new RemoteIter(d_geturl, 0));
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
        if (d_validSize)
            return d_size;
        else
            throw std::runtime_error("RemoteCorpusReader: size is unknown");
    }

    bool RemoteCorpusReaderPrivate::validQuery(QueryDialect d, bool variables,
        std::string const &query) const
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
        boost::to_lower(result);
        boost::trim(result);
        return (result == "true" || result == "yes" || result == "1");
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
    CorpusReader::EntryIterator RemoteCorpusReaderPrivate::runQueryWithStylesheet(
        QueryDialect d, std::string const &q, std::string const &stylesheet,
        std::list<MarkerQuery> const &markerQueries) const
    {
        std::list<MarkerQuery>::const_iterator iter = markerQueries.begin();

        std::string url(d_url + "/entries?query=" + util::toPercentEncoding(q) + "&contents=1");

        if (iter != markerQueries.end())
            url += "&markerQuery=" + util::toPercentEncoding(iter->query) +
                "&markerAttr=" + util::toPercentEncoding(iter->attr) +
                "&markerValue=" + util::toPercentEncoding(iter->value);

        ++iter;
        if (iter != markerQueries.end())
            throw Error("RemoteCorpusReaderPrivate: Multiple queries not implemented");

        std::tr1::shared_ptr<util::GetUrl> p(new util::GetUrl(url, stylesheet));

        return EntryIterator(new RemoteIter(p, 0, false, true));
    }


    // TODO: multiple queries (now: only the first is used)
    CorpusReader::EntryIterator RemoteCorpusReaderPrivate::beginWithStylesheet(
        std::string const &stylesheet,
        std::list<MarkerQuery> const &markerQueries) const
    {
        std::list<MarkerQuery>::const_iterator iter = markerQueries.begin();

        if (iter == markerQueries.end())
            throw Error("RemoteCorpusReaderPrivate: Missing query");

        std::tr1::shared_ptr<util::GetUrl> p(new util::GetUrl(d_url + "/entries" +
                                                              "?markerQuery=" + util::toPercentEncoding(iter->query) +
                                                              "&markerAttr=" + util::toPercentEncoding(iter->attr) +
                                                              "&markerValue=" + util::toPercentEncoding(iter->value) +
                                                              "&contents=1", stylesheet));

        ++iter;
        if (iter != markerQueries.end())
            throw Error("RemoteCorpusReaderPrivate: Multiple queries not implemented");

        return EntryIterator(new RemoteIter(p, 0, false, true));


    }

    // done
    CorpusReader::EntryIterator RemoteCorpusReaderPrivate::runXPath(
        std::string const &query) const
    {
        std::tr1::shared_ptr<util::GetUrl> p(new util::GetUrl(d_url +
            "/entries?query=" + util::toPercentEncoding(query) +
            "&contents=1"));
        return EntryIterator(new RemoteIter(p, 0, false, true));
    }

    // done? TODO: klopt dit? (blijkbaar wel)
    CorpusReader::EntryIterator RemoteCorpusReaderPrivate::runXQuery(
        std::string const &query) const
    {
        return runXPath(query);
    }

    // done
    RemoteCorpusReaderPrivate::RemoteIter::RemoteIter(std::tr1::shared_ptr<util::GetUrl> geturl,
                                                      size_t n,
                                                      bool end,
                                                      bool isquery)
        : d_geturl(geturl), d_idx(n), d_end(end), d_isquery(isquery), d_active(false)
    {
        d_interrupted = std::tr1::shared_ptr<bool>(new bool(false));
    }

    // done
    RemoteCorpusReaderPrivate::RemoteIter::~RemoteIter()
    {
    }

    void RemoteCorpusReaderPrivate::RemoteIter::activate() const
    {
        if (d_active)
            return;
        if (d_end) {
            d_active = true;
            return;
        }

        d_geturl->line(d_idx);
        if (d_geturl->eof())
            d_end = true;

        d_active = true;
    }

    bool RemoteCorpusReaderPrivate::RemoteIter::hasNext()
    {
        activate();

        if (*d_interrupted) {
#ifdef RemCorReaPri_DEBUG
            std::cerr << "[RemoteCorpusReaderPrivate] Calling next on interrupted RemoteIter" << std::endl;
#endif
            throw alpinocorpus::IterationInterrupted();
        }

        if (d_end)
            return false;

        d_geturl->line(d_idx);
        if (d_geturl->eof()) {
            d_end = true;
            return false;
        }

        return true;
    }

    Entry RemoteCorpusReaderPrivate::RemoteIter::next(CorpusReader const &)
    {
        activate();

        if (d_end)
            throw std::runtime_error("Called next() on an iterator that does not have more entries!");

        std::string line = d_geturl->line(d_idx);
        Entry e;

        if (d_isquery) {
            size_t i = line.find('\t');
            if (i == std::string::npos)
                e.name = line;
            else {
                // We also have contents...
                e.name = line.substr(0, i);
                e.contents = unescape(line.substr(i + 1));
            }
        } else
              e.name = line;

        ++d_idx;

        return e;
    }

    // done
    IterImpl *RemoteCorpusReaderPrivate::RemoteIter::copy() const
    {
        RemoteIter *other = new RemoteIter(d_geturl, d_idx, d_end, d_isquery);

        other->d_interrupted = d_interrupted;

        return other;
    }

    // done
    void RemoteCorpusReaderPrivate::RemoteIter::interrupt()
    {
#ifdef RemCorReaPri_DEBUG
        std::cerr << "[RemoteCorpusReaderPrivate] interrupting..." << std::endl;
#endif
        d_geturl->interrupt();
        *d_interrupted = true;
    }

    std::string RemoteCorpusReaderPrivate::RemoteIter::unescape(std::string text)
    {
        boost::algorithm::replace_all(text, "\\f", "\f");
        boost::algorithm::replace_all(text, "\\n", "\n");
        boost::algorithm::replace_all(text, "\\n", "\n");
        boost::algorithm::replace_all(text, "\\r", "\r");
        boost::algorithm::replace_all(text, "\\t", "\t");
        boost::algorithm::replace_all(text," \\\\", "\\");

        return text;
    }

}   // namespace alpinocorpus
