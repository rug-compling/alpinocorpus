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

        d_geturl.reset(new util::GetUrl(d_url + "/entries"));
    }

    RemoteCorpusReaderPrivate::~RemoteCorpusReaderPrivate()
    {
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

        if (iter == markerQueries.end())
            throw Error("RemoteCorpusReaderPrivate: Missing query");

        std::tr1::shared_ptr<util::GetUrl> p(new util::GetUrl(d_url + "/entries?query=" +
                                                              util::toPercentEncoding(q) +
                                                              "&markerQuery=" + util::toPercentEncoding(iter->query) +
                                                              "&markerAttr=" + util::toPercentEncoding(iter->attr) +
                                                              "&markerValue=" + util::toPercentEncoding(iter->value) +
                                                              "&contents=1", stylesheet));

        ++iter;
        if (iter != markerQueries.end())
            throw Error("RemoteCorpusReaderPrivate: Multiple queries not implemented");

        return EntryIterator(new RemoteIter(p, 0, true));
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

        return EntryIterator(new RemoteIter(p, 0, true));


    }

    // done
    CorpusReader::EntryIterator RemoteCorpusReaderPrivate::runXPath(
        std::string const &query) const
    {
        std::tr1::shared_ptr<util::GetUrl> p(new util::GetUrl(d_url + "/entries?query=" +
                                                              util::toPercentEncoding(query) + "&contents=1"));
        return EntryIterator(new RemoteIter(p, 0, true));
    }

    // done? TODO: klopt dit? (blijkbaar wel)
    CorpusReader::EntryIterator RemoteCorpusReaderPrivate::runXQuery(
        std::string const &query) const
    {
        return runXPath(query);
    }

    // done
    RemoteCorpusReaderPrivate::RemoteIter::RemoteIter(
        std::tr1::shared_ptr<util::GetUrl> geturl, long signed int n,
        bool isquery)
        : d_geturl(geturl), d_idx(n), d_isquery(isquery), d_interrupted(false)
    {
        if (d_idx >= 0) {
            geturl->line(d_idx);
            if (geturl->eof())
                d_idx = -1;
        }
    }

    // done
    RemoteCorpusReaderPrivate::RemoteIter::~RemoteIter()
    {
    }

    // done
    std::string RemoteCorpusReaderPrivate::RemoteIter::current() const
    {
        if (d_idx >= 0) {
            std::string s = d_geturl->line(d_idx);
            if (d_isquery) {
                size_t i = s.find('\t');
                if (i == std::string::npos)
                    return s;
                else
                    return s.substr(0, i);
            } else
                return s;
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
        IterImpl *other = new RemoteIter(this->d_geturl, this->d_idx, this->d_isquery);

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
    std::string RemoteCorpusReaderPrivate::RemoteIter::contents(
        CorpusReader const &rdr) const
    {
        if (d_idx < 0)
            return std::string();

        if (!d_isquery)
            return std::string();

        std::string s = d_geturl->line(d_idx);
        size_t i = s.find('\t');
        s = s.substr(i + 1);

        std::string result;
        size_t e;
        for (;;) {
            e = s.find('\\');
            if (e > s.size() - 2) {
                result += s;
                break;
            }
            result += s.substr(0, e);
            switch (s[e + 1]) {
            case 'f':
                result += '\f';
                break;
            case 'n':
                result += '\n';
                break;
            case 'r':
                result += '\r';
                break;
            case 't':
                result += '\t';
                break;
            default:
                result += s[e + 1];
            }
            s = s.substr(e + 2);
        }

        return result;
    }

}   // namespace alpinocorpus
