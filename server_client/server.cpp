#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <cctype>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080

void toUpperCase(char *str) {
    for (int i = 0; str[i]; i++) {
        str[i] = toupper(str[i]);
    }
}

int main() {
    int server_fd = 0, new_socket = 0;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }
    
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 1) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while(1) {
        std::cout << "Waiting for connections...\n";
        
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        
        std::cout << "Connection established\n";

        while(1) {
            memset(buffer, 0, 1024);
            int valread = read(new_socket, buffer, 1024);
            if(valread == 0) {
                std::cout << "Client disconnected\n";
                close(new_socket);
                break;
            }

            toUpperCase(buffer);

            std::cout << "Server will send to client: " << buffer << std::endl;

            send(new_socket, buffer, strlen(buffer), 0);
        }
    }
    return 0;
}

