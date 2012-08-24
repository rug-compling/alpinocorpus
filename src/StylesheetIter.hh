#ifndef ALPINOCORPUS_STYLESHEETITER_HH
#define ALPINOCORPUS_STYLESHEETITER_HH

#include <list>
#include <string>

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/Entry.hh>
#include <AlpinoCorpus/IterImpl.hh>

namespace alpinocorpus {
    class XSLTransformer;

    class StylesheetIter : public IterImpl {
    public:
      StylesheetIter(CorpusReader::EntryIterator iter,
        std::string const &stylesheet,
        std::list<CorpusReader::MarkerQuery> const &markerQueries);
      virtual ~StylesheetIter();
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
      std::string d_stylesheet; // To simplify equals...
      XSLTransformer *d_transformer;
    };

}

#endif // ALPINOCORPUS_STYLESHEETITER_HH
