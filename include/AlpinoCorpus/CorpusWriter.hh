#ifndef ALPINO_CORPUSWRITER_HH
#define ALPINO_CORPUSWRITER_HH

#include <QString>

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/util/NonCopyable.hh>

namespace alpinocorpus {
    class CorpusWriter : public util::NonCopyable
    {
    public:
        virtual ~CorpusWriter() {}
        
        /**
         * Will write name as a portable (Unix, UTF-8) pathname.
         */
        virtual void write(QString const &name, QString const &content) = 0;
        
        /**
         * Write the contents of an entire CorpusReader.
         * If the second argument is false, fails with an alpinocorpus::Error
         * on the first error. Else, keeps writing till completion but may throw
         * an alpinocorpus::BatchError.
         *
         * @bug Weakly exception-safe: does not clean up in fail-first mode.
         */
        virtual void write(CorpusReader const &corpus, bool failsafe = false) = 0;
  
    };
}

#endif // ALPINO_CORPUSWRITER_HH