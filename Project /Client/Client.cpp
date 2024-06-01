#include "Client.hpp"
#include <iostream>
#include <thread>

void Client::connect()
{
    ip::tcp::resolver resolver(io_context_);
    const auto endpoints = resolver.resolve("127.0.0.1", "8080");
    while(true)
    {
        boost::system::error_code ec;
        boost::asio::connect(socket_, endpoints, ec);

        if (!ec)
        {
            std::cout << node << " connected to Chiral Network..." << std::endl;
            break;
        }
        std::cerr << "Trying to connect to Chiral Network operator..." << std::endl;
        std::cout << "Reconnecting in 5 seconds...\n" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(5));

    }
}

void Client::send_cargo()
{
    std::cout << "\nRecepient -> ";
    std::this_thread::sleep_for(std::chrono::seconds(5));
    send_request_to_server("Sending to operator...");
}

bool Client::catch_impostor(const std::string& _node)
{
    if (const auto it = std::find(cities_.begin(), cities_.end(), _node); it == cities_.end())
    {
        cities_.push_back(_node);
        return false;
    }

    return true;
}

void Client::send_request_to_server(const std::string& recipient_node)
{
    std::string input;
    const nlohmann::json message = {
        {"name", "Sam"},
        {"surname", "Porter Bridges"},
        {"stamina", 100},
        {"carried_materials", {
                {"resins", {
                    {"small", 1},
                    {"medium", 2},
                    {"large", 1}
                }},
                {"metals", {
                    {"small", 2},
                    {"medium", 2}
                }}
        }},
        {"load_weight", 50},
        {"id", 2}
    };


    std::cout << message["name"] << " sends -> " << message.dump('\n') << std::endl;
    async_write(socket_, buffer(message.dump()),
    [this](const boost::system::error_code ec, std::size_t)
    {
        if(ec)
        {
            std::cout << "Error: " << ec.message() << std::endl;
        }
    });
}

std::string Client::receiveMessage() {
    read_until(socket_, m_buffer, "\0");
    std::istream input(&m_buffer                                                                                                                                                                                                                );
    std::string msg;
    std::cout << "\nGet\n";
    std::getline(input, msg, '\0');
    return msg;
}