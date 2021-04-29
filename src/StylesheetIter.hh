#ifndef ALPINOCORPUS_STYLESHEETITER_HH
#define ALPINOCORPUS_STYLESHEETITER_HH

#include <list>
#include <memory>
#include <string>

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/Entry.hh>
#include <AlpinoCorpus/IterImpl.hh>

namespace alpinocorpus {
    class Stylesheet;

    class StylesheetIter : public IterImpl {
    public:
        StylesheetIter(CorpusReader::EntryIterator iter,
                       std::shared_ptr<Stylesheet> const stylesheet,
                       std::list<CorpusReader::MarkerQuery> const &markerQueries) :
                d_iter(iter),
                d_markerQueries(markerQueries),
                d_stylesheet(stylesheet) {}

        virtual ~StylesheetIter() {}

        IterImpl *copy() const;

        bool hasNext();

        bool hasProgress();

        Entry next(CorpusReader const &rdr);

        double progress();

    protected:
        void interrupt();

    private:
        StylesheetIter(StylesheetIter const &other);

        StylesheetIter &operator=(StylesheetIter const &other);

        CorpusReader::EntryIterator d_iter;
        std::list<CorpusReader::MarkerQuery> d_markerQueries;
        std::shared_ptr<Stylesheet> const d_stylesheet;
    };
}

#endif // ALPINOCORPUS_STYLESHEETITER_HH
