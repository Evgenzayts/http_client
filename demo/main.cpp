#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <cstdlib>
#include <iostream>
#include <string>
#include <nlohmann/json.hpp>

namespace beast = boost::beast;     // from <boost/beast.hpp>
namespace http = beast::http;       // from <boost/beast/http.hpp>
namespace net = boost::asio;        // from <boost/asio.hpp>
using tcp = net::ip::tcp;           // from <boost/asio/ip/tcp.hpp>
using json = nlohmann::json;

std::string GetInputStr() {
  std::cout << "Enter anything: ";
  std::string input;
  std::cin >> input;

  json smth;
  smth["input"] = input;
  std::stringstream ss;
  ss << smth;

  return ss.str();
}

int main(int argc, char** argv)
{
  try {
    // Check command line arguments.
    if (argc != 4) {
      std::cerr <<
          "Usage: http-client-sync <host> <port> <target>\n" <<
          "Example:\n" <<
          "    http-client-sync www.example.com 80 /\n";
      return EXIT_FAILURE;
    }
    auto const host = argv[1];
    auto const port = argv[2];
    auto const target = argv[3];
    auto const version = 11;

    // The io_context is required for all I/O
    net::io_context ioc;

    // These objects perform our I/O
    tcp::resolver resolver(ioc);
    beast::tcp_stream stream(ioc);

    // Look up the domain name
    auto const results = resolver.resolve(host, port);

    // Make the connection on the IP address we get from a lookup
    stream.connect(results);

    // Set up an HTTP GET request message
    http::request<http::string_body> request{http::verb::post, target, version};
    request.set(http::field::host, host);
    request.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

    std::string input_str = GetInputStr();

    request.body() = input_str;
    request.content_length(request.body().size());

    // Send the HTTP request to the remote host
    http::write(stream, request);

    // This buffer is used for reading and must be persisted
    beast::flat_buffer buffer;
    // Declare a container to hold the response
    http::response<http::dynamic_body> response;
    // Receive the HTTP response
    http::read(stream, buffer, response);

    // Write the message to standard out
    std::cout << response << std::endl;

    // Gracefully close the socket
    beast::error_code ec;
    stream.socket().shutdown(tcp::socket::shutdown_both, ec);

    if (ec && ec != beast::errc::not_connected)
      throw beast::system_error{ec};

  } catch (std::exception const& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
