#ifndef ALPINOCORPUS_UTIL_GET_URL_HH
#define ALPINOCORPUS_UTIL_GET_URL_HH

#include <string>
#include <map>

namespace alpinocorpus { namespace util {

/*! \class GetUrl GetUrl.hh "AlpinoCorpus/util/GetUrl.hh"
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
 *  \todo
 *  - decoding of body based on charset
 *
 */

class GetUrl {

public:
    typedef std::map<std::string, std::string> Headers;

    //! Constructor with a url. The webpage is retrieved immediately.
    GetUrl(std::string const& url);

    ~GetUrl();

    //! Get the body of the retrieved webpage.
    std::string const& body() const;

    //! Get a header for the retrieved webpage. Field names are case-insensitive.
    Headers const &headers() const;

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

    void download(std::string const& url, int maxhop);
    void extractHeaders(std::istream *response_stream);
    URLComponents parseUrl(std::string const &url);

    std::string d_result;
    Headers d_headers;

};

} } // namespace alpinocorpus::util

#endif // ALPINOCORPUS_UTIL_GET_URL_HH
