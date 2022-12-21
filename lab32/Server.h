#pragma once
#include <string>
#include "ClientHandler.h"

using namespace std;

class Server {
private:
    ClientHandler *handler;
    int server_socket;
    bool interrupted;

public:
    void run();
    void interrupt();

    Server(const char *server_ip, int server_port);
    ~Server();
};