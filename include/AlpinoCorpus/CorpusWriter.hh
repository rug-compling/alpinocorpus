#ifndef ALPINO_CORPUSWRITER_HH
#define ALPINO_CORPUSWRITER_HH

#include <string>

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/util/NonCopyable.hh>

namespace alpinocorpus {
    class ALPINO_CORPUS_EXPORT CorpusWriter : public util::NonCopyable
    {
    public:
        enum WriterType { DBXML_CORPUS_WRITER } ;

        virtual ~CorpusWriter() {}

        /**
         * Open a corpus writer of the given type.
         */
        static CorpusWriter *open(std::string const &filename,
            bool overwrite, WriterType writerType);

        /**
         * Check whether a particular writer type is available.
         */
        static bool writerAvailable(WriterType writerType);
        
        /**
         * Will write name as a portable (Unix, UTF-8) pathname.
         */
        void write(std::string const &name, std::string const &content);
        
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
        virtual void writeEntry(std::string const &name, std::string const &content) = 0;
        virtual void writeEntry(CorpusReader const &corpus, bool failsafe = false) = 0;
    };
}

#endif // ALPINO_CORPUSWRITER_HH
