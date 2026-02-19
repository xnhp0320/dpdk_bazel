#include <iostream>
#include <string>

#include <boost/asio.hpp>
#include <boost/asio/local/stream_protocol.hpp>

#include "messages.pb.h"
#include <google/protobuf/util/json_util.h>

using boost::asio::local::stream_protocol;
namespace json_util = google::protobuf::util;

class UnixSocketClient {
 public:
  explicit UnixSocketClient(const std::string& socket_path)
      : socket_(io_context_) {
    socket_.connect(stream_protocol::endpoint(socket_path));
  }

  std::string SendRequest(const unix_socket::Request& request) {
    // Serialize request to JSON
    std::string request_json;
    auto status = json_util::MessageToJsonString(request, &request_json);
    if (!status.ok()) {
      throw std::runtime_error("Failed to serialize request: " +
                               status.ToString());
    }

    // Send request with newline delimiter
    std::string message = request_json + '\n';
    boost::asio::write(socket_, boost::asio::buffer(message));

    // Read response
    std::string response_json;
    boost::asio::read_until(socket_, boost::asio::dynamic_buffer(response_json),
                            '\n');

    // Remove trailing newline
    if (!response_json.empty() && response_json.back() == '\n') {
      response_json.pop_back();
    }

    return response_json;
  }

 private:
  boost::asio::io_context io_context_;
  stream_protocol::socket socket_;
};

int main(int argc, char* argv[]) {
  const std::string socket_path = "/tmp/unix_socket_server.sock";

  try {
    UnixSocketClient client(socket_path);

    // Test various commands
    std::vector<std::function<unix_socket::Request()>> tests = {
      // Echo command
      []() {
        unix_socket::Request req;
        req.set_command("echo");
        (*req.mutable_args())["text"] = "Hello from client!";
        return req;
      },
      // Ping command
      []() {
        unix_socket::Request req;
        req.set_command("ping");
        return req;
      },
      // Add command
      []() {
        unix_socket::Request req;
        req.set_command("add");
        (*req.mutable_args())["a"] = "42";
        (*req.mutable_args())["b"] = "58";
        return req;
      },
      // Info command
      []() {
        unix_socket::Request req;
        req.set_command("info");
        return req;
      },
      // Unknown command
      []() {
        unix_socket::Request req;
        req.set_command("unknown");
        return req;
      },
    };

    for (const auto& test_fn : tests) {
      unix_socket::Request request = test_fn();
      std::string output;
      if (!json_util::MessageToJsonString(request, &output).ok()) {
          output = "<error>";
      }

      std::cout << "\nSending command: " << request.command() << std::endl;
      std::cout << "Request JSON: "
                << output
                << std::endl;

      std::string response_json = client.SendRequest(request);

      unix_socket::Response response;
      auto status = json_util::JsonStringToMessage(response_json, &response);

      if (status.ok()) {
        std::cout << "Response success: " << (response.success() ? "true" : "false")
                  << std::endl;
        std::cout << "Response message: " << response.message() << std::endl;
        if (!response.data().empty()) {
          std::cout << "Response data:" << std::endl;
          for (const auto& [key, value] : response.data()) {
            std::cout << "  " << key << ": " << value << std::endl;
          }
        }
      } else {
        std::cout << "Failed to parse response: " << status.ToString()
                  << std::endl;
        std::cout << "Raw response: " << response_json << std::endl;
      }
    }

  } catch (const std::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
    std::cerr << "Make sure the server is running on " << socket_path << std::endl;
    return 1;
  }

  return 0;
}
