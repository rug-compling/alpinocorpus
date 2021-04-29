#include <list>
#include <string>
#include <typeinfo>

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/IterImpl.hh>
#include <AlpinoCorpus/XSLTransformer.hh>

#include "StylesheetIter.hh"

namespace alpinocorpus {


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

    bool StylesheetIter::hasProgress()
    {
        return d_iter.hasProgress();
    }

    bool StylesheetIter::hasNext()
    {
        return d_iter.hasNext();
    }

    Entry StylesheetIter::next(CorpusReader const &rdr)
    {
        Entry e = d_iter.next(rdr);
        e.contents = d_stylesheet->transform(rdr.read(e.name, d_markerQueries));

        return e;
    }

    double StylesheetIter::progress()
    {
        return d_iter.progress();
    }

    /*
    void StylesheetIter::next()
    {
        d_iter.next();
    }
    */

}
