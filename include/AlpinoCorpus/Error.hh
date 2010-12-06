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
        explicit Error(std::string const &msg)
         : std::runtime_error(msg)
        {
        }
        virtual ~Error() throw() {}
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
}
