#ifndef ALPINOCORPUS_FILTERITER_HH
#define ALPINOCORPUS_FILTERITER_HH

#include <queue>
#include <string>

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/Entry.hh>
#include <AlpinoCorpus/IterImpl.hh>

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
        std::string d_query;
        std::queue<std::string> d_buffer;
        mutable bool d_initialState;
        bool d_interrupted;
    };
}


#endif // ALPINOCORPUS_FILTERITER_HH
