#ifndef ALPINO_CORPUSWRITER_HH
#define ALPINO_CORPUSWRITER_HH

#include <string>

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
        void write(std::string const &name, QString const &content);
        
        /**
         * Write the contents of an entire CorpusReader.
         * If the second argument is false, fails with an alpinocorpus::Error
         * on the first error. Else, keeps writing till completion but may throw
         * an alpinocorpus::BatchError.
         *
         * @bug Weakly exception-safe: does not clean up in fail-first mode.
         */
        void write(CorpusReader const &corpus, bool failsafe = false);
    private:
        virtual void writeEntry(std::string const &name, QString const &content) = 0;
        virtual void writeEntry(CorpusReader const &corpus, bool failsafe = false) = 0;
    };
}

#endif // ALPINO_CORPUSWRITER_HH