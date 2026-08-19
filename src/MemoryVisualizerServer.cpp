#include "MemoryVisualizerServer.hpp"
#include "MemoryTracker.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
typedef int socklen_t;
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket close
#endif

#ifdef _WIN32
static DWORD WINAPI serverThreadProc(LPVOID lpParam) {
    MemoryVisualizerServer* server = static_cast<MemoryVisualizerServer*>(lpParam);
    if (server) {
        server->runServerLoop();
    }
    return 0;
}
#endif

MemoryVisualizerServer::MemoryVisualizerServer(int port, ArenaAllocator& arena, PoolAllocator& pool, FreeListAllocator& freeList)
    : serverPort(port), arenaRef(arena), poolRef(pool), freeListRef(freeList) {}

MemoryVisualizerServer::~MemoryVisualizerServer() {
    stop();
}

void MemoryVisualizerServer::start() {
    isRunning = true;
#ifdef _WIN32
    HANDLE hThread = CreateThread(NULL, 0, serverThreadProc, this, 0, NULL);
    threadHandle = static_cast<void*>(hThread);
#endif
}

void MemoryVisualizerServer::stop() {
    if (isRunning) {
        isRunning = false;
#ifdef _WIN32
        if (threadHandle) {
            WaitForSingleObject(static_cast<HANDLE>(threadHandle), 1000);
            CloseHandle(static_cast<HANDLE>(threadHandle));
            threadHandle = nullptr;
        }
#endif
    }
}

std::string MemoryVisualizerServer::readFile(const std::string& filepath) {
    std::ifstream file(filepath.c_str(), std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        return "";
    }
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

std::string MemoryVisualizerServer::buildJsonStats() {
    MemoryTracker& tracker = MemoryTracker::getInstance();
    std::ostringstream ss;

    ss << "{\n";
    ss << "  \"totalAllocated\": " << tracker.getTotalAllocated() << ",\n";
    ss << "  \"currentActive\": " << tracker.getActiveAllocations() << ",\n";
    ss << "  \"peakMemory\": " << tracker.getPeakMemory() << ",\n";
    ss << "  \"allocationCount\": " << tracker.getAllocationCount() << ",\n";

    ss << "  \"arena\": {\n";
    ss << "    \"capacity\": " << arenaRef.getCapacity() << ",\n";
    ss << "    \"used\": " << arenaRef.getUsedMemory() << ",\n";
    ss << "    \"free\": " << arenaRef.getRemainingMemory() << "\n";
    ss << "  },\n";

    ss << "  \"pool\": {\n";
    ss << "    \"blockSize\": " << poolRef.getBlockSize() << ",\n";
    ss << "    \"totalBlocks\": " << poolRef.getBlockCount() << ",\n";
    ss << "    \"usedBlocks\": " << poolRef.getUsedBlocks() << ",\n";
    ss << "    \"freeBlocks\": " << poolRef.getFreeBlocks() << "\n";
    ss << "  },\n";

    ss << "  \"freeList\": {\n";
    ss << "    \"capacity\": " << freeListRef.getCapacity() << ",\n";
    ss << "    \"used\": " << freeListRef.getUsedMemory() << ",\n";
    ss << "    \"free\": " << freeListRef.getFreeMemory() << "\n";
    ss << "  },\n";

    ss << "  \"snapshots\": [\n";
    auto snaps = tracker.getSnapshots();
    for (size_t i = 0; i < snaps.size(); ++i) {
        ss << "    {\n";
        ss << "      \"address\": \"" << snaps[i].address << "\",\n";
        ss << "      \"size\": " << snaps[i].size << ",\n";
        ss << "      \"allocatorType\": \"" << snaps[i].allocatorType << "\",\n";
        ss << "      \"active\": " << (snaps[i].active ? "true" : "false") << "\n";
        ss << "    }" << (i + 1 < snaps.size() ? "," : "") << "\n";
    }
    ss << "  ]\n";
    ss << "}\n";

    return ss.str();
}

std::string MemoryVisualizerServer::handleRequest(const std::string& request) {
    if (request.find("GET /api/stats") != std::string::npos) {
        std::string body = buildJsonStats();
        std::ostringstream response;
        response << "HTTP/1.1 200 OK\r\n"
                 << "Content-Type: application/json\r\n"
                 << "Access-Control-Allow-Origin: *\r\n"
                 << "Content-Length: " << body.size() << "\r\n\r\n"
                 << body;
        return response.str();
    }

    if (request.find("GET /api/allocate") != std::string::npos) {
        if (request.find("allocator=Arena") != std::string::npos) {
            void* p = arenaRef.allocate(64);
            if (p) uiArenaPtrs.push_back(p);
        } else if (request.find("allocator=Pool") != std::string::npos) {
            void* p = poolRef.allocate();
            if (p) uiPoolPtrs.push_back(p);
        } else if (request.find("allocator=FreeList") != std::string::npos) {
            void* p = freeListRef.allocate(96);
            if (p) uiFreeListPtrs.push_back(p);
        }
        std::string body = "{\"status\":\"ok\"}";
        return "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: 15\r\n\r\n" + body;
    }

    if (request.find("GET /api/reset") != std::string::npos) {
        arenaRef.reset();
        poolRef.reset();
        freeListRef.reset();
        MemoryTracker::getInstance().reset();
        uiArenaPtrs.clear();
        uiPoolPtrs.clear();
        uiFreeListPtrs.clear();

        std::string body = "{\"status\":\"reset_ok\"}";
        return "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: 21\r\n\r\n" + body;
    }

    std::string fileContent;
    std::string mimeType = "text/html";

    if (request.find("GET /styles.css") != std::string::npos) {
        fileContent = readFile("webui/styles.css");
        mimeType = "text/css";
    } else if (request.find("GET /app.js") != std::string::npos) {
        fileContent = readFile("webui/app.js");
        mimeType = "text/javascript";
    } else {
        fileContent = readFile("webui/index.html");
        mimeType = "text/html";
    }

    if (fileContent.empty()) {
        return "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
    }

    std::ostringstream response;
    response << "HTTP/1.1 200 OK\r\n"
             << "Content-Type: " << mimeType << "\r\n"
             << "Content-Length: " << fileContent.size() << "\r\n\r\n"
             << fileContent;
    return response.str();
}

void MemoryVisualizerServer::runServerLoop() {
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "[VisualizerServer] Failed to create socket\n";
        return;
    }

    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(serverPort);

    if (bind(serverSocket, (struct sockaddr*)&address, sizeof(address)) == SOCKET_ERROR) {
        std::cerr << "[VisualizerServer] Failed to bind to port " << serverPort << "\n";
        closesocket(serverSocket);
        return;
    }

    if (listen(serverSocket, 10) == SOCKET_ERROR) {
        std::cerr << "[VisualizerServer] Listen failed\n";
        closesocket(serverSocket);
        return;
    }

    std::cout << "\n=======================================================\n";
    std::cout << " [SERVER ONLINE] Memory Visualizer UI available at:\n";
    std::cout << "                http://localhost:" << serverPort << "\n";
    std::cout << "=======================================================\n\n";

    while (isRunning) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(serverSocket, &readfds);

        timeval timeout{0, 200000}; // 200ms timeout
        int activity = select(static_cast<int>(serverSocket + 1), &readfds, nullptr, nullptr, &timeout);

        if (activity > 0 && FD_ISSET(serverSocket, &readfds)) {
            sockaddr_in clientAddr{};
            socklen_t addrLen = sizeof(clientAddr);
            SOCKET clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &addrLen);

            if (clientSocket != INVALID_SOCKET) {
                char buffer[2048] = {0};
                int bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
                if (bytesRead > 0) {
                    std::string response = handleRequest(std::string(buffer));
                    send(clientSocket, response.c_str(), static_cast<int>(response.size()), 0);
                }
                closesocket(clientSocket);
            }
        }
    }

    closesocket(serverSocket);
#ifdef _WIN32
    WSACleanup();
#endif
}
