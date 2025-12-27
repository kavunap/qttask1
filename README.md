# System Developer – Practical Test(KAVUNA Paul)

Below is a simple, minimal C++ web server, using Winsock. It listens on a port and serves a basic HTTP response.

## Technical Tools

1. Editor: VS Code
2. Browser: Google chrome
3. Terminal: Windows terminal
4. OS: Windows 10
5. Compiler: MinGW / g++

## Step 1

Create a new file and name it `minimal_server.cpp` and put the following code.

``minimal_server.cpp``

```cpp

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>

#pragma comment(lib, "ws2_32.lib")

int main() {
    WSADATA wsaData;
    SOCKET serverSocket, clientSocket;
    sockaddr_in serverAddr;
    char buffer[1024];

    // 1. Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return 1;
    }

    // 2. Create socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed\n";
        WSACleanup();
        return 1;
    }

    // 3. Bind socket
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // 4. Listen
    listen(serverSocket, 5);
    std::cout << "Server running at http://localhost:8080\n";

    // 5. Accept client
    clientSocket = accept(serverSocket, nullptr, nullptr);
    recv(clientSocket, buffer, sizeof(buffer), 0);

    // 6. HTTP response
    const char* response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Connection: close\r\n\r\n"
        "<html><body><h2>Hello from C++ Minimal Web Server!</h2></body></html>";

    send(clientSocket, response, strlen(response), 0);

    // 7. Cleanup
    closesocket(clientSocket);
    closesocket(serverSocket);
    WSACleanup();

    return 0;
}
```

## Compile the Server

1. Open VS Code termninal by pressing **ctrl+`** and put the following command.

`
g++ minimal_server.cpp -o server -lws2_32
`

2. Open windows command prompt and run the following command

`
server
`
 Then go to the browser and navigate to the following url

 `
 http://localhost:8080
`
The output should be the text

# Hello from C++ Minimal Web Server!
