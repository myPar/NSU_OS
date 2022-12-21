#pragma once
#include <pthread.h>
#include <string>
using namespace std;

class ClientData {
private:
    string client_ip;
    int client_port;
    int socket_dscr;
public:
    ClientData(string ip, int port, int socket);
    ClientData();
    string get_ip();
    int get_port();
    int get_socket_dscr();
};
