#pragma once

#include <iostream>
#include <boost/asio.hpp>
#include <nlohmann/json.hpp>

using namespace boost::asio;
using nl_json = nlohmann::json;

class Client final
{
public:
    explicit Client(io_context& io, const std::string& node_)
        : io_context_(io), socket_(io), node(node_)
    {
        if (const auto flag = catch_impostor(node_); flag == true)
        {
            std::cout << "Impostor, try with another city..." << std::endl;
            exit(0);
        }
        connect();
    }

    void connect();

    void send_request_to_server(const std::string& recipient_node);

    void send_cargo();

    bool catch_impostor(const std::string&);

    std::string receiveMessage();

private:
    io_context& io_context_;
    ip::tcp::socket socket_;
    streambuf m_buffer;
    std::string node;
    std::vector<std::string> cities_{};
};