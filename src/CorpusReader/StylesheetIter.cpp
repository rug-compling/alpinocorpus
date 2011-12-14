#include <list>
#include <string>
#include <typeinfo>

#include <AlpinoCorpus/CorpusReader.hh>

#include "../XSLTransformer.hh"

namespace alpinocorpus {

    CorpusReader::StylesheetIter::StylesheetIter(EntryIterator iter,
            EntryIterator end,
            std::string const &stylesheet,
            std::list<MarkerQuery> const &markerQueries) :
        d_iter(iter), d_end(end),
        d_markerQueries(markerQueries),
        d_stylesheet(stylesheet),
        d_transformer(new XSLTransformer(stylesheet))
    {
    }

    CorpusReader::StylesheetIter::~StylesheetIter()
    {
        delete d_transformer;
    }

    CorpusReader::IterImpl *CorpusReader::StylesheetIter::copy() const
    {
        // The only state are the wrapped iterators. We can safely reconstruct
        // the transformer.
        return new StylesheetIter(d_iter, d_end, d_stylesheet, d_markerQueries);
    }

    std::string CorpusReader::StylesheetIter::current() const
    {
        return *d_iter;
    }

    bool CorpusReader::StylesheetIter::equals(IterImpl const &other)
        const
    {
        try {
            StylesheetIter const &otherSI =
                dynamic_cast<StylesheetIter const &>(other);
            
            return (d_iter == otherSI.d_iter &&
                d_markerQueries == otherSI.d_markerQueries &&
                d_stylesheet == otherSI.d_stylesheet);
        } catch (std::bad_cast const &) {
            // The other class is not a StylesheetIter, but the user may
            // be comparing this iterator with the wrapped end iterator.
            return d_iter == d_end;
        } 
    }
    
    std::string CorpusReader::StylesheetIter::contents(CorpusReader const &rdr) const
    {
        return d_transformer->transform(rdr.read(*d_iter, d_markerQueries));
    }

    void CorpusReader::StylesheetIter::interrupt()
    {
        d_iter.interrupt();
    }

    void CorpusReader::StylesheetIter::next()
    {
        ++d_iter;
    }

}
