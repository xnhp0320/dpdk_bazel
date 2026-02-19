#include <iostream>
#include <memory>
#include <string>
#include <array>
#include <thread>
#include <functional>

#include <boost/asio.hpp>
#include <boost/asio/local/stream_protocol.hpp>

#include "messages.pb.h"
#include <google/protobuf/util/json_util.h>

using boost::asio::local::stream_protocol;
namespace json_util = google::protobuf::util;

// Session class to handle a single client connection
class Session : public std::enable_shared_from_this<Session> {
 public:
  explicit Session(stream_protocol::socket socket)
      : socket_(std::move(socket)) {}

  void Start() {
    DoRead();
  }

 private:
  void DoRead() {
    auto self = shared_from_this();
    // Read until newline (assuming newline-delimited JSON messages)
    boost::asio::async_read_until(
        socket_,
        boost::asio::dynamic_buffer(buffer_),
        '\n',
        [this, self](boost::system::error_code ec, std::size_t length) {
          if (!ec) {
            // Extract the message (excluding the newline)
            std::string json_str = buffer_.substr(0, length - 1);
            buffer_.erase(0, length);

            HandleMessage(json_str);
          } else if (ec != boost::asio::error::eof) {
            std::cerr << "Read error: " << ec.message() << std::endl;
          }
        });
  }


  void HandleMessage(const std::string& json_str) {
    unix_socket::Request request;
    unix_socket::Response response;

    // Parse JSON from protobuf
    auto parse_status = json_util::JsonStringToMessage(json_str, &request);
    if (!parse_status.ok()) {
      response.set_success(false);
      response.set_message("Invalid JSON: " + parse_status.ToString());
    } else {
      // Process the request
      ProcessRequest(request, response);
    }

    // Serialize response to JSON
    std::string response_json;
    auto serialize_status =
        json_util::MessageToJsonString(response, &response_json);
    if (!serialize_status.ok()) {
      response_json = R"({"success": false, "message": "Failed to serialize response"})";
    }

    DoWrite(response_json + '\n');
  }

  void ProcessRequest(const unix_socket::Request& request,
                      unix_socket::Response& response) {
    const std::string& cmd = request.command();

    if (cmd == "echo") {
      response.set_success(true);
      response.set_message("Echo: " + GetArg(request, "text", "empty"));
      (*response.mutable_data())["original"] = GetArg(request, "text", "empty");

    } else if (cmd == "ping") {
      response.set_success(true);
      response.set_message("pong");
      (*response.mutable_data())["timestamp"] = std::to_string(std::time(nullptr));

    } else if (cmd == "add") {
      try {
        double a = std::stod(GetArg(request, "a", "0"));
        double b = std::stod(GetArg(request, "b", "0"));
        double result = a + b;
        response.set_success(true);
        response.set_message("Addition completed");
        (*response.mutable_data())["result"] = std::to_string(result);

      } catch (const std::exception& e) {
        response.set_success(false);
        response.set_message(std::string("Invalid numbers: ") + e.what());
      }

    } else if (cmd == "info") {
      response.set_success(true);
      response.set_message("Server info");
      (*response.mutable_data())["version"] = "1.0.0";
      (*response.mutable_data())["type"] = "unix-socket-server";
      (*response.mutable_data())["protocol"] = "json-protobuf";

    } else {
      response.set_success(false);
      response.set_message("Unknown command: " + cmd);
      (*response.mutable_data())["available_commands"] = "echo, ping, add, info";
    }
  }

  static std::string GetArg(const unix_socket::Request& request,
                            const std::string& key,
                            const std::string& default_val) {
    auto it = request.args().find(key);
    return (it != request.args().end()) ? it->second : default_val;
  }

  void DoWrite(const std::string& json_str) {
    auto self = shared_from_this();
    boost::asio::async_write(
        socket_, boost::asio::buffer(json_str),
        [this, self](boost::system::error_code ec, std::size_t /*length*/) {
          if (!ec) {
            // Continue reading next message
            DoRead();
          } else {
            std::cerr << "Write error: " << ec.message() << std::endl;
          }
        });
  }

  stream_protocol::socket socket_;
  std::string buffer_;
};


// Unix socket server
class UnixSocketServer {
 public:
  UnixSocketServer(boost::asio::io_context& io_context,
                   const std::string& socket_path)
      : acceptor_(io_context, stream_protocol::endpoint(socket_path)),
        socket_path_(socket_path) {
    // Remove existing socket file if it exists
    Start();
  }

  ~UnixSocketServer() {
    // Clean up socket file on shutdown
    std::remove(socket_path_.c_str());
  }

  void Start() {
    DoAccept();
  }

 private:
  void DoAccept() {
    acceptor_.async_accept(
        [this](boost::system::error_code ec, stream_protocol::socket socket) {
          if (!ec) {
            std::cout << "Client connected" << std::endl;
            std::make_shared<Session>(std::move(socket))->Start();
          } else {
            std::cerr << "Accept error: " << ec.message() << std::endl;
          }

          // Accept next connection
          DoAccept();
        });
  }

  stream_protocol::acceptor acceptor_;
  std::string socket_path_;
};

int main(int argc, char* argv[]) {
  const std::string socket_path = "/tmp/unix_socket_server.sock";

  try {
    boost::asio::io_context io_context;
    UnixSocketServer server(io_context, socket_path);

    std::cout << "Unix socket server listening on: " << socket_path << std::endl;
    std::cout << "Supported commands: echo, ping, add, info" << std::endl;

    // Handle Ctrl+C gracefully
    boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
    signals.async_wait([&](const boost::system::error_code&, int) {
      std::cout << "\nShutting down server..." << std::endl;
      io_context.stop();
    });

    io_context.run();

  } catch (const std::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
