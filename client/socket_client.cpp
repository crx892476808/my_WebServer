#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cstring>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

std::string str = "GET / HTTP/1.1" \
"Host: 10.176.35.12:8000" \
"User-Agent: curl/7.58.0"
"Accept: *//*";

int main() {
    std::cout << "This is client" << std::endl;
    // socket
    int client = socket(AF_INET, SOCK_STREAM, 0);
    if (client == -1) {
        std::cout << "Error: socket" << std::endl;
        return 0;
    }
    // connect
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8000);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if (connect(client, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cout << "Error: connect" << std::endl;
        return 0;
    }
    std::cout << "...connect" << std::endl;
    char data[255];
    char buf[255];
    while (true) {
        std::cin >> data;
        if(send(client, str.c_str(), str.size(), 0) > 0)
            std::cout << "client send finish" << std::endl;
        if (strcmp(data, "exit") == 0) {
            std::cout << "...disconnect" << std::endl;
            break;
        }
        memset(buf, 0, sizeof(buf));
        int len = recv(client, buf, sizeof(buf), 0);
        buf[len] = '\0';
        std::cout << buf << std::endl;
    }
    close(client);
    return 0;
}