#include <string>
#include <typeinfo>

#include <AlpinoCorpus/tr1wrap/memory.hh>

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/Entry.hh>
#include <AlpinoCorpus/Error.hh>

#include "FilterIter.hh"

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

    FilterIter::FilterIter(CorpusReader const &corpus,
        CorpusReader::EntryIterator itr,
        std::string const &query)
    :
        d_corpus(corpus),
        d_itr(itr),
        d_initialState(true)
    {
        DynamicContext *ctx = s_xqilla.createContext(XQilla::XPATH2);
        ctx->setXPath1CompatibilityMode(true);

        try {
            d_query.reset(s_xqilla.parse(X(query.c_str()), ctx));
        } catch (XQException &e) {
            throw Error("CorpusReader::FilterIter::FilterIter: could not evaluate XPath expression.");
        }
    }
    
    IterImpl *FilterIter::copy() const
    {
        // FilterIter is no bare pointer members.
        return new FilterIter(*this);
    }

    bool FilterIter::hasNext()
    {
        d_interrupted = false;

        while (d_buffer.empty() && d_itr.hasNext())
        {
            Entry e = d_itr.next(d_corpus);

            if (d_interrupted)
              throw IterationInterrupted();

            d_file = e.name;
            parseFile(d_file);
        }

        return !d_buffer.empty();
    }

    bool FilterIter::hasProgress()
    {
      return d_itr.hasProgress();
    }

    void FilterIter::interrupt()
    {
      d_interrupted = true;
    }

    
    Entry FilterIter::next(CorpusReader const &rdr)
    {
        if (d_buffer.empty())
            throw Error("Calling next() on an iterator that is exhausted.");

        Entry e = {d_file, d_buffer.empty() ? std::string() : d_buffer.front()};

        d_buffer.pop();

        return e;
    }
    
    void FilterIter::parseFile(std::string const &file)
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

    double FilterIter::progress()
    {
        return d_itr.progress();
    }

}
