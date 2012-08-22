#include <list>
#include <string>
#include <typeinfo>

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/IterImpl.hh>

#include "StylesheetIter.hh"
#include "XSLTransformer.hh"

namespace alpinocorpus {

    StylesheetIter::StylesheetIter(CorpusReader::EntryIterator iter,
            std::string const &stylesheet,
            std::list<CorpusReader::MarkerQuery> const &markerQueries) :
        d_iter(iter),
        d_markerQueries(markerQueries),
        d_stylesheet(stylesheet),
        d_transformer(new XSLTransformer(stylesheet))
    {
    }

    StylesheetIter::~StylesheetIter()
    {
        delete d_transformer;
    }

    IterImpl *StylesheetIter::copy() const
    {
        // The only state are the wrapped iterators. We can safely reconstruct
        // the transformer.
        return new StylesheetIter(d_iter, d_stylesheet, d_markerQueries);
    }

    void StylesheetIter::interrupt()
    {
        d_iter.interrupt();
    }

    bool StylesheetIter::hasNext()
    {
        return d_iter.hasNext();
    }

    Entry StylesheetIter::next(CorpusReader const &rdr)
    {
        Entry e = d_iter.next(rdr);
        e.contents = d_transformer->transform(rdr.read(e.name, d_markerQueries));

        return e;
    }

    /*
    void StylesheetIter::next()
    {
        d_iter.next();
    }
    */

}
