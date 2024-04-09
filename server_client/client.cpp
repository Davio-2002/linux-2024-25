#include <iostream>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080

int main() {
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};
    int sock = 0;
    
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cout << "\n Socket creation error \n";
        return -1;
    }
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);   

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cout << "\nConnection Failed \n";
        return -1;
    }

    while(1) {
        std::cout << "Enter message: ";
        std::cin.getline(buffer, 1024);
        send(sock, buffer, strlen(buffer), 0);
        std::cout << "Message sent\n";
        
        memset(buffer, 0, 1024);
        int valread = read(sock, buffer, 1024);
        std::cout << "Server: " << buffer << std::endl;
    }
    return 0;
}

