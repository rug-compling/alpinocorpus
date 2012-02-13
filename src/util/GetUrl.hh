#ifndef ALPINOCORPUS_UTIL_GET_URL_HH
#define ALPINOCORPUS_UTIL_GET_URL_HH

#include <config.hh>

#include <string>
#include <map>

#include <boost/asio.hpp>
#include <boost/asio/streambuf.hpp>
#ifdef ALPINOCORPUS_WITH_SSL
#include <boost/asio/ssl.hpp>
#endif // defined(ALPINOCORPUS_WITH_SSL)

namespace alpinocorpus { namespace util {

        /*! \class GetUrl GetUrl.hh "util/GetUrl.hh"
         *  \brief GetUrl is a simple class for retrieving a webpage.
         *
         *  Supported:
         *  - http
         *  - https, if compiled with \c WITH_SSL
         *  - redirection
         *  - url with port number
         *
         *  \note
         *  Url with username/password is \b not supported
         *
         * \note
         * Content-encoding gzip no longer supported
         *
         *  \todo
         *  - decoding of body based on charset
         *
         */

        class GetUrl {

        public:
            typedef std::map<std::string, std::string> Headers;

            //! Constructor with a url. The headers for the webpage are retrieved immediately.
            GetUrl(std::string const& url, std::string const& body="");

            ~GetUrl();

            /*! \brief Retrieve and return the body of the webpage.

              \note Don't mix calls to body() and line().
            */
            std::string const& body();

            /*! \brief Return a single line of the body of the webpage. Retrieve it from the server if necessary.

              If lineno < 0, then next line, else line with that number.

              - Use eof() after line() to check for end-of-file.

              \note Don't mix calls to line() and body().
            */
            std::string const& line(long signed int = -1);

            //! Did last call to line() result in end-of-file?
            bool eof() const { return d_eoflast; }

            //! Get a header for the retrieved webpage. Field names are case-insensitive.
            std::string const& header(std::string const& field) const;

            //! All headers for the retrieved webpage. Field names are converted to lowercase.
            Headers const &headers() const;

            //! Get the bare Content-Type for the retrieved webpage, converted to lower case, without charset etc.
            std::string const & content_type() const;

            //! Get charset for the retrieved webpage, converted to lower case
            std::string const & charset() const;

            void interrupt ();
            bool interrupted () const { return d_interrupted; }
            bool completed() const { return d_completed; }

        private:
            struct URLComponents {
                URLComponents(std::string const &newScheme,
                              std::string const &newDomain,
                              std::string const &newPort,
                              std::string const &newPath) :
                    scheme(newScheme),
                    domain(newDomain),
                    port(newPort),
                    path(newPath) {}

                std::string scheme;
                std::string domain;
                std::string port;
                std::string path;
            };

            void clean_up();
            void download(std::string const& url, int maxhop, std::string const &body);
            void parseResponse(std::istream *response_stream);
            void parseHeaders(std::istream *response_stream);
            void parseContentType();
            URLComponents parseUrl();

            std::string d_charset;
            std::string d_content_type;
            std::string d_result;
            std::string d_nullstring;
            std::vector<std::string> d_lines;
            long unsigned int d_nlines;
            long unsigned int d_prevline;
            std::string d_url; // url of last redirect
            Headers d_headers;
            bool d_redirect;
            bool d_ssl;
            bool d_requested_body;
            bool d_requested_line;
            bool d_eof;
            bool d_eoflast;
            bool d_completed;
            bool d_interrupted;
            bool d_cleaned_up;
            boost::asio::io_service d_io_service;
            boost::asio::streambuf d_response;
            std::istream *d_response_stream;

            boost::asio::ip::tcp::socket *d_socket;
#ifdef ALPINOCORPUS_WITH_SSL
            typedef boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_socket;
            ssl_socket *d_ssl_socket;
#endif // defined(ALPINOCORPUS_WITH_SSL)

        };

    }

} // namespace alpinocorpus::util

#endif // ALPINOCORPUS_UTIL_GET_URL_HH
