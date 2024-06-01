#include "../headers/Server.h"
#include <iostream>
#include <thread>

void Server::broadcast_message(const std::string& message)
{
    for (const auto& [fst, snd] : sessions_)
    {
        snd->send_response(message);
        std::cout << "\nSent\n\n";
    }
}

void Server::clear_screen()
{
#ifdef _WIN32
    std::system("cls");
#else
    std::system("clear");
#endif
}

void Server::print_message(const std::string& message, const int& interval)
{
    std::cout << message << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(interval));
    clear_screen();
}

void Server::dummy_status_bar(const int interval)
{
    clear_screen();
    print_message("Loading your cargo...\n", interval);
    print_message("Getting things ready...\n", interval);
    std::string final_message = "****** Chiral Network welcomes you ******\n";
    for(const char& c : final_message)
    {
        std::cout << c << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(40));
    }
}

void Server::do_accept()
{
    m_acceptor.async_accept(
        [this](const boost::system::error_code& ec, tcp::socket socket)
        {
            if (!ec)
            {
                const auto session = std::make_shared<Session>(std::move(socket), *this);
                sessions_.emplace_back(id_count++, session);
                session->run();
            }
            do_accept();
        }
    );
}
