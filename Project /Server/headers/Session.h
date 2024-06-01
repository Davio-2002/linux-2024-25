#pragma once

#include <boost/asio.hpp>
#include <nlohmann/json.hpp>
#include <memory>
#include "Server.h"

class Server;
using boost::asio::ip::tcp;

class Session final : public std::enable_shared_from_this<Session>
{
public:
    explicit Session(tcp::socket socket, Server& server)
        : m_socket(std::move(socket)), m_server(server)
    {}

    void run();

    void send_response(const std::string& message);

private:
    void read_message_from_buffer();

    void handle_request(const std::string& message);

    tcp::socket m_socket;
    boost::asio::streambuf m_buffer;
    Server& m_server;
    std::string m_city_name;
    nlohmann::json m_cargo;
};
