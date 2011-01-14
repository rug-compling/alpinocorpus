#include <QString>
#include <stdexcept>
#include <string>

namespace alpinocorpus {
    /**
     * Base class for errors generated inside the alpinocorpus namespace
     */
    class Error : public std::runtime_error
    {
      public:
        explicit Error(std::string const &msg) : std::runtime_error(msg) {}
        virtual ~Error() throw() {}

      protected:
        explicit Error() : std::runtime_error("") {}
    };

    /**
     * Error in batch processing/conversion of large corpora.
     * Logically encapsulates a set of Errors.
     */
    class BatchError : public Error
    {
        // We can't use an ostringstream because the result of its str()
        // method would go out of scope at the end of what(), so we'd
        // return a dangling pointer.
        std::string msg;
        size_t n;

      public:
        explicit BatchError() : msg("Errors in batch conversion:\n"), n(0) { }
        ~BatchError() throw() {}

        /** Add an exception to the list */
        void append(Error const &e)
        {
            msg += "    ";
            msg += e.what();
            msg += '\n';
            n++;
        }

        /** True iff no sub-errors recorded */
        bool empty() const { return n==0; }

        /** Returns a (multi-line) report from the appended Errors. */
        virtual char const *what() const throw() { return msg.c_str(); }
    };

    /**
     * Corpus contains duplicate key
     */
    class DuplicateKey : public Error
    {
        explicit DuplicateKey(QString const &key)
         : Error(construct(key))
        {
        }
        ~DuplicateKey() throw() {}

      private:
        static std::string construct(QString const &);
    };

    /**
     * Construction/open error from corpus readers and writers
     */
    class OpenError : public Error
    {
      public:
        explicit OpenError(QString const &path)
         : Error(construct(path)) { }
        explicit OpenError(QString const &path, QString const &extra)
         : Error(construct(path, extra)) { }
        ~OpenError() throw() {}

      private:
        static std::string construct(QString const &path);
        static std::string construct(QString const &path, QString const &extra);
        static std::string construct(QString const &path, char const *extra);
    };

    /**
     * Thrown by EntryIterator if the size of a corpus (subset) cannot be
     * determined.
     */
    class SizeNotAvailable : public Error
    {
      public:
        explicit SizeNotAvailable()
         : Error("size of corpus subset not available") { }
        ~SizeNotAvailable() throw() { }
    };
}
