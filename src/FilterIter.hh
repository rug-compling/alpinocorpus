#ifndef ALPINOCORPUS_FILTERITER_HH
#define ALPINOCORPUS_FILTERITER_HH

#include <queue>
#include <string>

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/IterImpl.hh>

namespace alpinocorpus {
    class FilterIter : public IterImpl {
      public:
        FilterIter(CorpusReader const &, CorpusReader::EntryIterator,
            CorpusReader::EntryIterator, std::string const &);
        IterImpl *copy() const;
        std::string current() const;
        bool equals(IterImpl const &) const;
        void next();
        std::string contents(CorpusReader const &) const;

      protected:
        void interrupt();
      
      private:
        void parseFile(std::string const &);
        
        CorpusReader const &d_corpus;
        CorpusReader::EntryIterator d_itr;
        CorpusReader::EntryIterator d_end;
        std::string d_file;
        std::string d_query;
        std::queue<std::string> d_buffer;
        mutable bool d_initialState;
        bool d_interrupted;
    };
}


#endif ALPINOCORPUS_FILTERITER_HH
