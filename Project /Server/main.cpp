#include <boost/asio/io_context.hpp>
#include <iostream>
#include "headers/Server.h"

constexpr short port = 8080;

int main()
{
    boost::asio::io_context io_context;
    Server server(io_context, port);
    Server::dummy_status_bar(1);

    io_context.run();
    return 0;
}
