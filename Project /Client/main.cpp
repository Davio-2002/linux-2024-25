#include <iostream>
#include <fstream>
#include <boost/asio.hpp>
#include "Client.hpp"

using ip::tcp;

int read_count(const std::string& filename) {
    std::ifstream infile(filename);
    int count = 0;
    if (infile.is_open()) {
        infile >> count;
    }
    return count;
}

void write_count(const std::string& filename, const int count) {
    if (std::ofstream outfile(filename); outfile.is_open()) {
        outfile << count;
    }
}

int main(const int argc, char** argv)
{
    if (argc != 2)
    {
        std::cerr << "Usage: client <node>\n";
        return 1;
    }

    const std::string count_filename = "client_count.txt";
    int count = read_count(count_filename);

    ++count;

    write_count(count_filename, count);

    std::cout << "Client instance count: " << count << std::endl;

    io_context io;

    Client c(io, argv[1]);

    std::thread t([&io] { io.run(); });

    while (true)
    {
        c.send_cargo();

        std::cout << c.receiveMessage();
    }

    t.join();
    return 0;
}
