#ifndef ALPINOCORPUS_FILTERITER_HH
#define ALPINOCORPUS_FILTERITER_HH

#include <queue>
#include <string>

#include <memory>

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/Entry.hh>
#include <AlpinoCorpus/IterImpl.hh>

class XQQuery;

namespace alpinocorpus {
    class FilterIter : public IterImpl {
      public:
        FilterIter(CorpusReader const &corpus,
            CorpusReader::EntryIterator i,
            std::string const &query);
        IterImpl *copy() const;
        bool hasNext();
        bool hasProgress();
        Entry next(CorpusReader const &rdr);
        double progress();

      protected:
        void interrupt();
      
      private:
        void parseFile(std::string const &);
        
        CorpusReader const &d_corpus;
        CorpusReader::EntryIterator d_itr;
        std::string d_file;
        std::shared_ptr<XQQuery> d_query;
        std::queue<std::string> d_buffer;
        bool d_interrupted;
    };
}


#endif // ALPINOCORPUS_FILTERITER_HH
