#pragma once

#include <vector>
#include "Session.h"

using boost::asio::ip::tcp;

class Session;
class Server final
{
public:
    explicit Server(boost::asio::io_context& io_context, const short port)
    : m_acceptor(io_context, tcp::endpoint(tcp::v4(), port)), id_count(1) {
        do_accept();
    }

    void broadcast_message(const std::string& message);

    static void clear_screen();

    static void print_message(const std::string& message, const int& interval);

    static void dummy_status_bar(int interval);

    void do_accept();

    tcp::acceptor m_acceptor;
    std::vector<std::pair<int, std::shared_ptr<Session>>> sessions_;
    int id_count{};

public:
    [[nodiscard]] int get_id_count() const
    {
        return id_count;
    }
};
