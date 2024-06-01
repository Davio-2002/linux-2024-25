#include <boost/asio.hpp>
#include <nlohmann/json.hpp>
#include <iostream>
#include <memory>

#include "../headers/Session.h"

void Session::run()
{
    read_message_from_buffer();
}

void Session::send_response(const std::string& message)
{
    auto self = shared_from_this();
    async_write(m_socket, boost::asio::buffer(message + '\0'),
                [this, self](const boost::system::error_code ec, std::size_t)
                {
                    if (ec)
                    {
                        std::cout << "Error sending message: " << ec.message() << std::endl;
                    }
                }
    );
}


void Session::read_message_from_buffer()
{
    auto self = shared_from_this();
    async_read_until(m_socket, m_buffer, "\0",
                     [this, self](const boost::system::error_code ec, std::size_t)
                     {
                         if (!ec)
                         {
                             std::istream is(&m_buffer);
                             std::string message;
                             std::getline(is, message);
                             handle_request(message);
                         }
                         else
                         {
                             std::cout << "Client disconnected, waiting for upcoming session calls..." << std::endl;
                         }
                         read_message_from_buffer();
                     }
    );
}

void Session::handle_request(const std::string& message)
{
    const auto cargo = nlohmann::json::parse(message);
    m_cargo = cargo;
    std::cout << m_cargo.dump('\n') << std::endl;

    m_cargo["connection"] = "successful";
    m_cargo["from"] = "Chiral Network";
    m_cargo["id"] = "1";

    m_server.broadcast_message(m_cargo.dump('\n'));
}
