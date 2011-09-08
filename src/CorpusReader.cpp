#include <string>

#include <tr1/memory>

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/DbCorpusReader.hh>
#include <AlpinoCorpus/DirectoryCorpusReader.hh>
#include <AlpinoCorpus/Error.hh>
#include <AlpinoCorpus/IndexedCorpusReader.hh>
#include <AlpinoCorpus/RecursiveCorpusReader.hh>
#include <util/markqueries.hh>

#include <typeinfo>

#include <xercesc/framework/MemBufInputSource.hpp>
#include <xqilla/functions/FunctionString.hpp>
#include <xqilla/utils/XQillaPlatformUtils.hpp>
#include <xqilla/xqilla-simple.hpp>

namespace {
    struct Globals {
        Globals();
        virtual ~Globals();
    };

    static Globals s_globals;
    
    Globals::Globals() {
        XQillaPlatformUtils::initialize();
    }
    
    Globals::~Globals() {
        XQillaPlatformUtils::terminate();
    }

    static XQilla s_xqilla;

}

namespace alpinocorpus {    

    bool CorpusReader::EntryIterator::operator!=(EntryIterator const &other) const
    {
        return !operator==(other);
    }
    
    
    CorpusReader::EntryIterator CorpusReader::begin() const
    {
        return getBegin();
    }
    
    std::string CorpusReader::EntryIterator::contents(CorpusReader const &rdr) const
    {
        return impl->contents(rdr);
    }
    
    CorpusReader::EntryIterator CorpusReader::end() const
    {
        return getEnd();
    }
    
    std::string CorpusReader::name() const
    {
        return getName();
    }
        
    CorpusReader *CorpusReader::open(std::string const &corpusPath)
    {
        try {
            return new DirectoryCorpusReader(corpusPath);
        } catch (OpenError const &e) {
        }

        try {
            return new IndexedCorpusReader(corpusPath);
        } catch (OpenError const &e) {
        }

        return new DbCorpusReader(corpusPath);
    }

    CorpusReader *CorpusReader::openRecursive(std::string const &path)
    {
      return new RecursiveCorpusReader(path);
    }

    std::string CorpusReader::read(std::string const &entry) const
    {
        return readEntry(entry);
    }
    
    std::string CorpusReader::readMarkQueries(std::string const &entry,
        std::list<MarkerQuery> const &queries) const
    {
        return readEntryMarkQueries(entry, queries);
    }
        
    std::string CorpusReader::readEntryMarkQueries(std::string const &entry,
        std::list<MarkerQuery> const &queries) const
    {
        std::string xmlData = readEntry(entry);

        return markQueries(xmlData, queries);
    }
    
    size_t CorpusReader::size() const
    {
        return getSize();
    }
    
    bool CorpusReader::isValidQuery(QueryDialect d, bool variables, std::string const &query)
    {
        // XXX - strip/trim
        if (query.empty())
            return true;

        DynamicContext *ctx = s_xqilla.createContext(XQilla::XPATH2);
        ctx->setXPath1CompatibilityMode(true);

        std::tr1::shared_ptr<XQQuery> queryPtr;
        try {
            queryPtr.reset(s_xqilla.parse(X(query.c_str()), ctx));
        } catch (XQException &e) {
          return false;
        }

        return true;
    }
    
    CorpusReader::EntryIterator::value_type CorpusReader::EntryIterator::operator*() const
    {
        return impl->current();
    }
    
    bool CorpusReader::EntryIterator::operator==(EntryIterator const &other) const
    {
        if (!impl)
            return !other.impl;
        else if (!other.impl)
            return !impl;
        else
            return impl->equals(*other.impl.get());
    }
    
    CorpusReader::EntryIterator &CorpusReader::EntryIterator::operator++()
    {
        impl->next();
        return *this;
    }

    
    CorpusReader::EntryIterator CorpusReader::EntryIterator::operator++(int)
    {
        EntryIterator r(*this);
        operator++();
        return r;
    }


    CorpusReader::EntryIterator CorpusReader::query(QueryDialect d,
        std::string const &q) const
    {
        switch (d) {
          case XPATH:  return runXPath(q);
          case XQUERY: return runXQuery(q);
          default:     throw NotImplemented("unknown query language");
        }
    }

    CorpusReader::EntryIterator CorpusReader::runXPath(std::string const &query) const
    {
        //throw NotImplemented(typeid(*this).name(), "XQuery functionality");
        return EntryIterator(new FilterIter(*this, getBegin(), getEnd(), query));
    }

    CorpusReader::EntryIterator CorpusReader::runXQuery(std::string const &) const
    {
        throw NotImplemented(typeid(*this).name(), "XQuery functionality");
    }
    
    CorpusReader::FilterIter::FilterIter(CorpusReader const &corpus,
        EntryIterator itr, EntryIterator end, std::string const &query)
    :
        d_corpus(corpus),
        d_itr(itr),
        d_end(end)
    {
        DynamicContext *ctx = s_xqilla.createContext(XQilla::XPATH2);
        ctx->setXPath1CompatibilityMode(true);

        try {
            d_query.reset(s_xqilla.parse(X(query.c_str()), ctx));
        } catch (XQException &e) {
            throw Error("CorpusReader::FilterIter::FilterIter: could not evaluate XPath expression.");
        }

        next();
    }
    
    std::string CorpusReader::FilterIter::current() const
    {
        return d_file;
    }
    
    bool CorpusReader::FilterIter::equals(IterImpl const &itr) const
    {
        try {
            // TODO fix me to be more like isEqual instead of hasNext.
            return d_itr == d_end
                && d_buffer.size() == 0;
        } catch (std::bad_cast const &e) {
            return false;
        }
    }
    
    void CorpusReader::FilterIter::next()
    {
        if (!d_buffer.empty())
            d_buffer.pop();
        
        while (d_buffer.empty() && d_itr != d_end)
        {
            d_file = *d_itr;
            parseFile(d_file);
            
            ++d_itr;
        }
    }
    
    std::string CorpusReader::FilterIter::contents(CorpusReader const &rdr) const
    {
        return d_buffer.empty()
        ?   std::string() // XXX - should be a null string???
            : d_buffer.front();
    }
    
    void CorpusReader::FilterIter::parseFile(std::string const &file)
    {
        std::string xml(d_corpus.read(file));

        std::tr1::shared_ptr<DynamicContext> ctx(d_query->createDynamicContext());
        XERCES_CPP_NAMESPACE::MemBufInputSource xmlInput(
            reinterpret_cast<XMLByte const *>(xml.c_str()),
            xml.size(), "input");

        try {
            Sequence seq(ctx->parseDocument(xmlInput));
        
            if (!seq.isEmpty() && seq.first()->isNode()) {
                ctx->setContextItem(seq.first());
                ctx->setContextPosition(1);
                ctx->setContextSize(1);
            }
        } catch (XQException &e) {
            // XXX - warning???
            return;
        }

        Result result = d_query->execute(ctx.get());
        
        Item::Ptr item;
        while ((item = result->next(ctx.get()))) {
            std::string value(UTF8(FunctionString::string(item, ctx.get())));
           
            // XXX - trim value!
            d_buffer.push(value);
        }
    }
    
    std::string CorpusReader::IterImpl::contents(CorpusReader const &rdr) const
    {
        //return rdr.read(current());
        return std::string(); // XXX - should be a null string
    }
    
}
