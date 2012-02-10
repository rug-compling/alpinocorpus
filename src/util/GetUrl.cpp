// #define GETURL_DEBUG

#include <config.hh>

#include <istream>
#include <ostream>
#ifdef GETURL_DEBUG
#include <iostream>
#endif

#include <string>
#include <boost/asio.hpp>
#ifdef ALPINOCORPUS_WITH_SSL
#include <boost/asio/ssl.hpp>
#endif // defined(ALPINOCORPUS_WITH_SSL)
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/erase.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>

#include "GetUrl.hh"

using boost::asio::ip::tcp;

#ifdef ALPINOCORPUS_WITH_SSL
namespace ssl = boost::asio::ssl;
#endif // defined(ALPINOCORPUS_WITH_SSL)

namespace alpinocorpus { namespace util {

        GetUrl::GetUrl(std::string const& url, std::string const& body) :
            d_redirect(false),
            d_content_type(""),
            d_charset(""),
            d_requested_body(false),
            d_requested_line(false),
            d_nlines(0),
            d_prevline(-1),
            d_nullstring(""),
            d_eof(false),
            d_eoflast(false),
            d_interrupted(false)
        {
            download(url, 6, body);
        }

        GetUrl::~GetUrl()
        {
            clean_up();
        }

        void GetUrl::interrupt() {
            d_interrupted = true;
        }

        void GetUrl::clean_up() {
            d_io_service.stop();

            delete d_response_stream;

#ifdef ALPINOCORPUS_WITH_SSL
            if (d_ssl) {
                d_ssl_socket->lowest_layer().close();
                delete d_ssl_socket;
            } else
#endif // defined(ALPINOCORPUS_WITH_SSL)
                {
                    d_socket->close();
                    delete d_socket;
                }
        }

        std::string const& GetUrl::line(long signed int lineno)
        {
            if (d_requested_body)
                throw std::runtime_error("GetUrl: requesting line after requesting body");

            d_requested_line = true;

            d_eoflast = false;

            if (lineno < 0)
                lineno = d_prevline + 1;

            d_prevline = lineno;

            if (d_eof)
                if (lineno < d_nlines)
                    return d_lines[lineno];
                else {
                    d_eoflast = true;
                    return d_nullstring;
                }

            boost::system::error_code error;
            size_t i;

            while (d_nlines <= lineno) {

                if (! d_response.size()) {
                    std::size_t a;
#ifdef ALPINOCORPUS_WITH_SSL
                    if (d_ssl)
                        a = d_ssl_socket->lowest_layer().available();
                    else
#endif
                        a = d_socket->available();
                    if (! a) {
                        boost::asio::deadline_timer t(d_io_service);
                        t.expires_from_now(boost::posix_time::millisec(100));
                        t.wait();
                        if (d_interrupted) {
                            d_eof = true;
                            d_eoflast = true;
                            return d_nullstring;
                        }
                        continue;
                    }
                }

#ifdef ALPINOCORPUS_WITH_SSL
                if (d_ssl)
                    i = boost::asio::read_until(*d_ssl_socket, d_response, '\n', error);
                else
#endif
                    i = boost::asio::read_until(*d_socket, d_response, '\n', error);

                /*

                // problem: reading the last line if final newline is missing

                // solution seems to be: just skip this test

                if (! i && error == boost::asio::error::eof) {
                    std::cerr << error << std::endl;
                    d_eof = true;
                    d_eoflast = true;
                    return d_nullstring;
                }

                */

                std::string line;
                if (! std::getline(*d_response_stream, line)) {
                    d_eof = true;
                    d_eoflast = true;
                    return d_nullstring;
                } else {
                    if (line[0] == '\004') {
                        d_eof = true;
                        d_eoflast = true;
                        return d_nullstring;
                    }
                    d_lines.push_back(line);
                    d_nlines++;
                }
            }

            return d_lines[lineno];
        }

        std::string const& GetUrl::body()
        {
            if (d_requested_body)
                return d_result;
            if (d_requested_line)
                throw std::runtime_error("GetUrl: requesting body after requesting line");

            d_requested_body = true;

            boost::system::error_code error;
            size_t i;
#ifdef ALPINOCORPUS_WITH_SSL
            if (d_ssl)
                i = boost::asio::read(*d_ssl_socket, d_response, boost::asio::transfer_all(), error);
            else
#endif
                i = boost::asio::read(*d_socket, d_response, boost::asio::transfer_all(), error);

            std::string enc(d_headers["content-encoding"]);
            boost::algorithm::to_lower(enc);
            boost::iostreams::filtering_stream<boost::iostreams::input> in;
            if (enc == "gzip") { // not used anymore because you can't detect newlines in gzip'ed data
                in.push(boost::iostreams::gzip_decompressor());
            }
            in.push(*d_response_stream);

            char delim = '\0';
            std::getline(in, d_result, delim);
            while (!in.eof()) {
                d_result += delim;
                std::string s;
                std::getline(in, s, delim);
                d_result += s;
            }

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

        void GetUrl::download(std::string const& url, int maxhop, std::string const &body) {

#ifdef GETURL_DEBUG
            std::cerr << "[GetUrl] " << (body.size() ? "POST " : "GET ") << url << std::endl;
#endif

            d_url = url;
            d_result.clear();
            d_headers.clear();
            d_redirect = false;

            if (maxhop == 0)
                throw std::runtime_error("GetUrl: too many redirects");

            URLComponents urlc = parseUrl();

            if (urlc.scheme != "http"
#ifdef ALPINOCORPUS_WITH_SSL
                && urlc.scheme != "https"
#endif // defined(ALPINOCORPUS_WITH_SSL)
                )
                throw std::invalid_argument("GetUrl: unsupported scheme '" + urlc.scheme + "' in url " + d_url);

            if (urlc.domain == "")
                throw std::invalid_argument("GetUrl: missing domain in url " + d_url);

            // Form the request. We specify the "Connection: close" header so that the
            // server will close the socket after transmitting the response. This will
            // allow us to treat all data up until the EOF as the content.
            boost::asio::streambuf request;
            std::ostream request_stream(&request);
            if (body.size() == 0)
                request_stream << "GET " << urlc.path << " HTTP/1.0\r\n";
            else {
                request_stream << "POST " << urlc.path << " HTTP/1.0\r\n";
                request_stream << "Content-Length: " << body.size() << "\r\n";
            }
            request_stream << "Host: " << urlc.domain << "\r\n";
            request_stream << "Accept: */*\r\n";
            // request_stream << "Accept-Encoding: gzip\r\n";  // can't do this, can't detect newlines in gzip'ed data
            request_stream << "User-Agent: Alpino Corpus\r\n";
            request_stream << "Connection: close\r\n";
            request_stream << "\r\n";
            if (body.size()) {
                request_stream << body;
            }

            boost::system::error_code error;
            tcp::resolver resolver(d_io_service);
            tcp::resolver::query query(urlc.domain, urlc.port.size() ? urlc.port : urlc.scheme);
            tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

            size_t i;
#ifdef ALPINOCORPUS_WITH_SSL
            if (urlc.scheme == "https") {

                d_ssl = true;

                ssl::context ctx(ssl::context::sslv23);
                ctx.set_default_verify_paths();

                // Open a socket and connect it to the remote host.
                d_ssl_socket = new ssl_socket(d_io_service, ctx);
                boost::asio::connect(d_ssl_socket->lowest_layer(), endpoint_iterator);
                d_ssl_socket->lowest_layer().set_option(tcp::no_delay(true));

                // Perform SSL handshake and verify the remote host's certificate.
#ifdef ALPINOCORPUS_WITH_SSL_STRICT
                d_ssl_socket->socket.set_verify_mode(ssl::verify_peer);
                d_ssl_socket->socket.set_verify_callback(ssl::rfc2818_verification(urlc.domain));
#else
                d_ssl_socket->set_verify_mode(ssl::verify_none);
#endif // defined(ALPINOCORPUS_WITH_SSL_STRICT)
                d_ssl_socket->handshake(ssl_socket::client);

                // Send the request.
                boost::asio::write(*d_ssl_socket, request);

                // Get response.
                i = boost::asio::read_until(*d_ssl_socket, d_response, "\r\n", error);
            } else
#endif // defined(ALPINOCORPUS_WITH_SSL)
                { // scheme == "http"
                    d_ssl = false;
                    // Try each endpoint until we successfully establish a connection.
                    d_socket = new tcp::socket(d_io_service);
                    boost::asio::connect(*d_socket, endpoint_iterator);

                    // Send the request.
                    boost::asio::write(*d_socket, request);

                    // Get response.
                    i = boost::asio::read_until(*d_socket, d_response, "\r\n", error);
                }
            if (!i)
                throw std::runtime_error("GetUrl: unable to connect to " + urlc.domain + " [ " + url + " ]");

            d_response_stream = new std::istream(&d_response);

            parseResponse(d_response_stream);

#ifdef ALPINOCORPUS_WITH_SSL
            if (d_ssl)
                i = boost::asio::read_until(*d_ssl_socket, d_response, "\r\n\r\n", error);
            else
#endif
                i = boost::asio::read_until(*d_socket, d_response, "\r\n\r\n", error);

            if (!i)
                throw std::runtime_error("GetUrl: reading headers for " + urlc.domain);
            parseHeaders(d_response_stream);
            if (d_redirect && d_headers["location"].size() == 0)
                throw std::runtime_error("GetUrl: redirect without location");

            if (d_headers["location"].size() == 0) {
                parseContentType();
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

            d_response.consume(d_response.size());
            clean_up();
            d_io_service.reset();
            download(u, maxhop - 1, body);

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

    }
} // namespace alpinocorpus::util
