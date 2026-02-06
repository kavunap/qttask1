#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <vector>

#pragma comment(lib, "ws2_32.lib")

int main() {
    WSADATA wsaData;
    SOCKET serverSocket;
    sockaddr_in serverAddr;

    WSAStartup(MAKEWORD(2, 2), &wsaData);

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    listen(serverSocket, SOMAXCONN);

    std::vector<SOCKET> clients;
    fd_set readfds;

    std::cout << "Event-driven server running on port 8080\n";

    while (true) {
        FD_ZERO(&readfds);
        FD_SET(serverSocket, &readfds);

        for (SOCKET c : clients)
            FD_SET(c, &readfds);

        select(0, &readfds, nullptr, nullptr, nullptr);

        // New connection
        if (FD_ISSET(serverSocket, &readfds)) {
            SOCKET client = accept(serverSocket, nullptr, nullptr);
            clients.push_back(client);
        }

        // Existing clients
        for (auto it = clients.begin(); it != clients.end();) {
            SOCKET c = *it;
            if (FD_ISSET(c, &readfds)) {
                char buffer[1024] = {0};
                recv(c, buffer, sizeof(buffer), 0);

                const char* response =
                    "HTTP/1.1 200 OK\r\n"
                    "Content-Type: text/html\r\n"
                    "Connection: close\r\n\r\n"
                    "<html><body><h2>Hello from event-driven server!</h2></body></html>";

                send(c, response, strlen(response), 0);
                closesocket(c);
                it = clients.erase(it);
            } else {
                ++it;
            }
        }
    }
}
