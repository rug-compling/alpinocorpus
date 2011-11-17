#include <config.hh>

#include <istream>
#include <ostream>
#include <string>
#include <boost/asio.hpp>
#ifdef WITH_SSL
#include <boost/asio/ssl.hpp>
#endif // defined(WITH_SSL)
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>

#include "GetUrl.hh"

using boost::asio::ip::tcp;

#ifdef WITH_SSL
namespace ssl = boost::asio::ssl;
#endif // defined(WITH_SSL)

namespace alpinocorpus { namespace util {

GetUrl::GetUrl(std::string const& url)
{
    download(url, 6);
}

GetUrl::~GetUrl()
{
}

std::string const& GetUrl::body()
{
    return d_result;
}

std::string const& GetUrl::header(std::string const& field)
{
    std::string s(field);
    boost::algorithm::to_lower(s);
    return d_headers[s];
}

void GetUrl::download(std::string const& url, int maxhop) {

    d_result.clear();
    d_headers.clear();

    if (maxhop == 0)
	throw std::runtime_error("GetUrl: too many redirects");

    // split the url into components
    std::string u, scheme, domain, port, path;
    size_t i;
    i = url.find("://");
    if (i == std::string::npos) {
	scheme = "http";
	u = url;
    } else {
	scheme = url.substr(0, i);
	u = url.substr(i + 3);
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

    if (scheme != "http"
#ifdef WITH_SSL
	 && scheme != "https"
#endif // defined(WITH_SSL)
	)
	throw std::invalid_argument("GetUrl: unsupported scheme '" + scheme + "' in url " + url);

    if (domain == "")
	throw std::invalid_argument("GetUrl: missing domain in url " + url);


    // Form the request. We specify the "Connection: close" header so that the
    // server will close the socket after transmitting the response. This will
    // allow us to treat all data up until the EOF as the content.
    boost::asio::streambuf request;
    std::ostream request_stream(&request);
    request_stream << "GET " << path << " HTTP/1.0\r\n";
    request_stream << "Host: " << domain << "\r\n";
    request_stream << "Accept: */*\r\n";
    request_stream << "User-Agent: Alpino Corpus\r\n";
    request_stream << "Connection: close\r\n\r\n";

    boost::asio::io_service io_service;
    boost::asio::streambuf response;
    boost::system::error_code error;
    tcp::resolver resolver(io_service);
    tcp::resolver::query query(domain,  port.size() ? port : scheme);
    tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
#ifdef WITH_SSL
    if (scheme == "https") {

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
	socket.set_verify_callback(ssl::rfc2818_verification(domain));
#else
	socket.set_verify_mode(ssl::verify_none);
#endif // defined(WITH_SSL_STRICT)
	socket.handshake(ssl_socket::client);

	// Send the request.
	boost::asio::write(socket, request);

	// Get response.
	i = boost::asio::read(socket, response, boost::asio::transfer_all(), error);
	socket.lowest_layer().close();

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
	    socket.close();
	}
    io_service.stop();
    if (! i)
	throw boost::system::system_error(error);

    // Process the response.
    std::istream response_stream(&response);
    std::string line;
    std::string status;
    bool redirect = false;

    std::getline(response_stream, line);
    if (line.substr(0, 4) != "HTTP")
	throw std::runtime_error("GetUrl: invalid response: " + line);

    // find reponse code
    typedef std::vector< std::string > split_vector_type;
    split_vector_type sv;
    boost::algorithm::split(sv, line, boost::algorithm::is_any_of(" \t"), boost::algorithm::token_compress_on);
    status = sv[1];

    // std::cout << "Status: " << status << std::endl;

    if (status[0] == '2')
	;
    else if (status[0] == '3')
	redirect = true;
    else
	throw std::runtime_error("geturl " + url +" : " + line);

    // header lines
    std::string previous ("");
    while (std::getline(response_stream, line)) {

	boost::algorithm::trim_right(line);

	if (line.size() == 0) { // end of header lines

	    if (d_headers["location"].size() > 0)
		break;

	    char delim = '\0';
	    std::getline(response_stream, d_result, delim);
	    while (!response_stream.eof()) {
		d_result += delim;
		std::string s;
		std::getline(response_stream, s, delim);
		d_result += s;
	    }
	    return;
	}

	if (line[0] == ' ' || line[0] == '\t') { // continuation line
	    boost::algorithm::trim_left(line);
	    if (d_headers[previous].size() > 0)
		d_headers[previous] += " ";
	    d_headers[previous] += line;
	} else { // normal header line
	    std::string s1, s2;
	    size_t ii;
	    ii = line.find(":");
	    if (ii == std::string::npos) {
		throw std::runtime_error("GetUrl: invalid header line: " + line);
	    } else {
		s1 = line.substr(0, ii);
		boost::algorithm::to_lower(s1);
		boost::algorithm::trim_right(s1);
		s2 = line.substr(ii + 1);
		boost::algorithm::trim_left(s2);
	    }
	    d_headers[s1] = s2;
	    previous = s1;
	}

    }

    if (redirect && d_headers["location"].size() == 0)
	throw "GetUrl: redirect without location";

    if (d_headers["location"].size() > 0) {
	std::string u;
	if (d_headers["location"][0] == '/') {
	    u = scheme + "://" + domain;
	    if (port.size() > 0) {
		u += ":";
		u += port;
	    }
	    u += d_headers["location"];
	} else
	    u = d_headers["location"];
	download(u, maxhop - 1);
    }

}

} } // namespace alpinocorpus::util
