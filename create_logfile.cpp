#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <cstdlib>

#pragma comment(lib, "ws2_32.lib")

std::ofstream logFile;

/* ===================== DATE HELPERS ===================== */

// YYYY-MM-DD
std::string getCurrentDate() {
    std::time_t now = std::time(nullptr);
    std::tm localTime;
    localtime_s(&localTime, &now);

    std::ostringstream oss;
    oss << std::put_time(&localTime, "%Y-%m-%d");
    return oss.str();
}

// Timestamp for log files
std::string getTimestampForFilename() {
    std::time_t now = std::time(nullptr);
    std::tm localTime;
    localtime_s(&localTime, &now);

    std::ostringstream oss;
    oss << std::put_time(&localTime, "%Y-%m-%d_%H-%M-%S");
    return oss.str();
}

/* ===================== LOGGING ===================== */

void log(const std::string& level, const std::string& message) {
    std::time_t now = std::time(nullptr);
    std::tm localTime;
    localtime_s(&localTime, &now);

    char timeBuf[20];
    std::strftime(timeBuf, sizeof(timeBuf), "%H:%M:%S", &localTime);

    std::string line = "[" + std::string(timeBuf) + "] [" + level + "] " + message;
    std::cout << line << std::endl;

    if (logFile.is_open())
        logFile << line << std::endl;
}

/* ===================== LOG BACKUP ===================== */

void backupLogs() {
    std::string backupDir = "/home/backups";
    std::string backupName = getCurrentDate() + "_backup.tar.gz";
    std::string fullPath = backupDir + "/" + backupName;

    // Ensure backup directory exists
    std::string mkdirCmd =
        "powershell -Command \"New-Item -ItemType Directory -Force -Path " + backupDir + "\"";
    system(mkdirCmd.c_str());

    // Create tar.gz archive
    std::string tarCmd =
        "powershell -Command \"tar -czf " + fullPath + " server_*.log\"";

    log("INFO", "Creating compressed backup: " + backupName);
    system(tarCmd.c_str());
    log("INFO", "Backup stored at: " + fullPath);
}

/* ===================== MAIN SERVER ===================== */

int main() {
    std::string logFilename = "server_" + getTimestampForFilename() + ".log";
    logFile.open(logFilename, std::ios::out);

    if (!logFile.is_open()) {
        std::cerr << "[ERROR] Failed to create log file\n";
        return 1;
    }

    log("INFO", "Log file created: " + logFilename);

    WSADATA wsaData;
    SOCKET serverSocket;
    sockaddr_in serverAddr;

    log("INFO", "Starting Winsock...");
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        log("ERROR", "WSAStartup failed");
        return 1;
    }

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        log("ERROR", "Socket creation failed");
        WSACleanup();
        return 1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        log("ERROR", "Bind failed");
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        log("ERROR", "Listen failed");
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    log("INFO", "Listening on port 8080...");

    std::vector<SOCKET> clients;
    fd_set readfds;

    while (true) {
        FD_ZERO(&readfds);
        FD_SET(serverSocket, &readfds);

        for (SOCKET c : clients)
            FD_SET(c, &readfds);

        int activity = select(0, &readfds, nullptr, nullptr, nullptr);
        if (activity == SOCKET_ERROR) {
            log("ERROR", "select() failed");
            break;
        }

        if (FD_ISSET(serverSocket, &readfds)) {
            SOCKET client = accept(serverSocket, nullptr, nullptr);
            if (client != INVALID_SOCKET) {
                clients.push_back(client);
                log("INFO", "New client connected");
            }
        }

        for (auto it = clients.begin(); it != clients.end();) {
            SOCKET c = *it;

            if (FD_ISSET(c, &readfds)) {
                char buffer[1024];
                int bytesReceived = recv(c, buffer, sizeof(buffer) - 1, 0);

                if (bytesReceived <= 0) {
                    log("WARN", "Client disconnected");
                    closesocket(c);
                    it = clients.erase(it);
                    continue;
                }

                buffer[bytesReceived] = '\0';
                log("INFO", "Request received");

                const char* body =
                    "<html><body><h2>Hello from event-driven server!</h2></body></html>";

                std::string response =
                    "HTTP/1.1 200 OK\r\n"
                    "Content-Type: text/html\r\n"
                    "Content-Length: " + std::to_string(strlen(body)) + "\r\n"
                    "Connection: close\r\n\r\n" +
                    std::string(body);

                send(c, response.c_str(), (int)response.size(), 0);
                closesocket(c);
                it = clients.erase(it);
            } else {
                ++it;
            }
        }
    }

    closesocket(serverSocket);
    WSACleanup();

    log("INFO", "Server shutting down...");
    logFile.close();

    backupLogs();  // ✅ YYYY-MM-DD_backup.tar.gz in /home/backups

    return 0;
}
