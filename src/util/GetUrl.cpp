#include <config.hh>

#include <istream>
#include <ostream>
#include <string>
#include <boost/asio.hpp>
#ifdef WITH_SSL
#include <boost/asio/ssl.hpp>
#endif // defined(WITH_SSL)
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/erase.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>

#include "GetUrl.hh"

using boost::asio::ip::tcp;

#ifdef WITH_SSL
namespace ssl = boost::asio::ssl;
#endif // defined(WITH_SSL)

namespace alpinocorpus { namespace util {

GetUrl::GetUrl(std::string const& url) :
    d_redirect(false),
    d_content_type(""),
    d_charset("")
{
    download(url, 6);
}

GetUrl::~GetUrl()
{
}

std::string const& GetUrl::body() const
{
    return d_result;
}

std::string const& GetUrl::header(std::string const& field) const
{
    static const std::string null("");

    std::string s(field);
    boost::algorithm::to_lower(s);
    Headers::const_iterator it = d_headers.find(s);
    if (it == d_headers.end()) {
        return null;
    }
    return it->second;
}

std::string const & GetUrl::content_type() const
{
    return d_content_type;
}

std::string const & GetUrl::charset() const
{
    return d_charset;
}

GetUrl::Headers const &GetUrl::headers() const
{
    return d_headers;
}

void GetUrl::download(std::string const& url, int maxhop) {

    d_url = url;
    d_result.clear();
    d_headers.clear();
    d_redirect = false;

    if (maxhop == 0)
        throw std::runtime_error("GetUrl: too many redirects");

    URLComponents urlc = parseUrl();

    if (urlc.scheme != "http"
#ifdef WITH_SSL
         && urlc.scheme != "https"
#endif // defined(WITH_SSL)
        )
        throw std::invalid_argument("GetUrl: unsupported scheme '" + urlc.scheme + "' in url " + d_url);

    if (urlc.domain == "")
        throw std::invalid_argument("GetUrl: missing domain in url " + d_url);


    // Form the request. We specify the "Connection: close" header so that the
    // server will close the socket after transmitting the response. This will
    // allow us to treat all data up until the EOF as the content.
    boost::asio::streambuf request;
    std::ostream request_stream(&request);
    request_stream << "GET " << urlc.path << " HTTP/1.0\r\n";
    request_stream << "Host: " << urlc.domain << "\r\n";
    request_stream << "Accept: */*\r\n";
    request_stream <<" Accept-Encoding: gzip\r\n";
    request_stream << "User-Agent: Alpino Corpus\r\n";
    request_stream << "Connection: close\r\n";
    request_stream << "\r\n";

    boost::asio::io_service io_service;
    boost::asio::streambuf response;
    boost::system::error_code error;
    tcp::resolver resolver(io_service);
    tcp::resolver::query query(urlc.domain, urlc.port.size() ? urlc.port : urlc.scheme);
    tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

    size_t i;
#ifdef WITH_SSL
    if (urlc.scheme == "https") {

        typedef ssl::stream<tcp::socket> ssl_socket;

        ssl::context ctx(ssl::context::sslv23);
        ctx.set_default_verify_paths();

        // Open a socket and connect it to the remote host.
        ssl_socket socket(io_service, ctx);
        boost::asio::connect(socket.lowest_layer(), endpoint_iterator);
        socket.lowest_layer().set_option(tcp::no_delay(true));

        // Perform SSL handshake and verify the remote host's certificate.
#ifdef WITH_SSL_STRICT
        socket.set_verify_mode(ssl::verify_peer);
        socket.set_verify_callback(ssl::rfc2818_verification(urlc.domain));
#else
        socket.set_verify_mode(ssl::verify_none);
#endif // defined(WITH_SSL_STRICT)
        socket.handshake(ssl_socket::client);

        // Send the request.
        boost::asio::write(socket, request);

        // Get response.
        i = boost::asio::read(socket, response, boost::asio::transfer_all(), error);
    } else
#endif // defined(WITH_SSL)
        { // scheme == "http"
            // Try each endpoint until we successfully establish a connection.
            tcp::socket socket(io_service);
            boost::asio::connect(socket, endpoint_iterator);

            // Send the request.
            boost::asio::write(socket, request);

            // Get response.
            i = boost::asio::read(socket, response, boost::asio::transfer_all(), error);
        }
    if (!i)
        throw boost::system::system_error(error);


    std::istream response_stream(&response);

    parseResponse(&response_stream);

    parseHeaders(&response_stream);
    if (d_redirect && d_headers["location"].size() == 0)
           throw "GetUrl: redirect without location";

    if (d_headers["location"].size() == 0) {
        parseContentType();
        getBody(&response_stream);
        return;
    }

    // redirect
    std::string u;
    if (d_headers["location"][0] == '/') {
        u = urlc.scheme + "://" + urlc.domain;
        if (urlc.port.size() > 0) {
            u += ":";
            u += urlc.port;
        }
        u += d_headers["location"];
    } else
        u = d_headers["location"];
    download(u, maxhop - 1);

}

void GetUrl::getBody(std::istream *response_stream)
{
    std::string enc(d_headers["content-encoding"]);
    boost::algorithm::to_lower(enc);
    boost::iostreams::filtering_stream<boost::iostreams::input> in;
    if (enc == "gzip") {
        in.push(boost::iostreams::gzip_decompressor());
    }
    in.push(*response_stream);

    char delim = '\0';
    std::getline(in, d_result, delim);
    while (!in.eof()) {
        d_result += delim;
        std::string s;
        std::getline(in, s, delim);
        d_result += s;
    }
}

void GetUrl::parseHeaders(std::istream *response_stream)
{
    std::string line;
    std::string previous ("");
    while (std::getline(*response_stream, line)) {

        boost::algorithm::trim_right(line);

        if (line.size() == 0)
            return;

        if (line[0] == ' ' || line[0] == '\t') { // continuation line
            boost::algorithm::trim_left(line);
            if (d_headers[previous].size() > 0)
                d_headers[previous] += " ";
            d_headers[previous] += line;
        } else { // normal header line
            std::string s1, s2;
            size_t i;
            i = line.find(":");
            if (i == std::string::npos) {
                throw std::runtime_error("GetUrl: invalid header line: " + line);
            } else {
                s1 = line.substr(0, i);
                boost::algorithm::to_lower(s1);
                boost::algorithm::trim_right(s1);
                s2 = line.substr(i + 1);
                boost::algorithm::trim_left(s2);
            }
            d_headers[s1] = s2;
            previous = s1;
        }

    }
}

void GetUrl::parseResponse(std::istream *response_stream)
{

    std::string line;
    std::string status;

    std::getline(*response_stream, line);
    if (line.substr(0, 4) != "HTTP")
        throw std::runtime_error("GetUrl: invalid response: " + line);

    // find reponse code
    typedef std::vector< std::string > split_vector_type;
    split_vector_type sv;
    boost::algorithm::split(sv, line, boost::algorithm::is_any_of(" \t"), boost::algorithm::token_compress_on);
    status = sv[1];

    if (status[0] == '2')
        ;
    else if (status[0] == '3')
        d_redirect = true;
    else
        throw std::runtime_error("geturl " + d_url +" : " + line);

}

GetUrl::URLComponents GetUrl::parseUrl()
{

    std::string u, scheme, domain, port, path;
    size_t i;
    i = d_url.find("://");
    if (i == std::string::npos) {
        scheme = "http";
        u = d_url;
    } else {
        scheme = d_url.substr(0, i);
        u = d_url.substr(i + 3);
    }

    i = u.find("/");
    if (i == std::string::npos) {
        path = "/";
    } else {
        path = u.substr(i);
        u = u.substr(0, i);
    }

    i = u.find(":");
    if (i == std::string::npos) {
        domain = u;
        port = "";
    } else {
        domain = u.substr(0, i);
        port = u.substr(i + 1);
    }

    return URLComponents(scheme, domain, port, path);
}

void GetUrl::parseContentType()
{
    typedef std::vector< std::string > split_vector_type;

    std::string ct(header("Content-Type"));
    boost::algorithm::to_lower(ct);
    boost::algorithm::erase_all(ct, "\"");
    boost::algorithm::erase_all(ct, "'");

    split_vector_type sv;
    boost::algorithm::split(sv, ct, boost::algorithm::is_any_of(";"), boost::algorithm::token_compress_on);

    if (sv.size() > 0) {
        d_content_type = sv[0];
        boost::algorithm::trim(d_content_type);
    }

    for (size_t i = 1; i < sv.size(); i++) {
        split_vector_type sv2;
        boost::algorithm::split(sv2, sv[i], boost::algorithm::is_any_of("="), boost::algorithm::token_compress_on);
        if (sv2.size() == 2) {
            boost::algorithm::trim(sv2[0]);
            if (sv2[0] == "charset") {
                d_charset = sv2[1];
                boost::algorithm::trim(d_charset);
            }
        }
    }
}

} } // namespace alpinocorpus::util

