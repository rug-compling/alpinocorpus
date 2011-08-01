#ifndef ALPINO_ERROR_HH
#define ALPINO_ERROR_HH

#include <string>
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
      public:
        explicit DuplicateKey(std::string const &key)
         : Error(construct(key)) {}
        ~DuplicateKey() throw() {}

      private:
        static std::string construct(std::string const &);
    };

    /**
     * Functionality not implemented (yet)
     */
    class NotImplemented : public Error
    {
      public:
        explicit NotImplemented(char const *func) : Error(func) {}
        explicit NotImplemented(char const *type, char const *func)
         : Error(construct(type, func)) {}
        ~NotImplemented() throw() {}

      private:
        std::string construct(char const *, char const *);
    };

    /**
     * Construction/open error from corpus readers and writers
     */
    class OpenError : public Error
    {
      public:
        explicit OpenError(std::string const &path)
         : Error(construct(path)) {}
        explicit OpenError(std::string const &path, std::string const &extra)
         : Error(construct(path, extra)) {}
        ~OpenError() throw() {}

      private:
        static std::string construct(std::string const &path);
        static std::string construct(std::string const &path, std::string const &extra);
        static std::string construct(std::string const &path, char const *extra);
    };
}

#endif // ALPINO_ERROR_HH
