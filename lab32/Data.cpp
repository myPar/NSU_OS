#include "Data.h"

ClientData::ClientData(string ip, int port, int socket) {
    client_ip = ip;
    client_port = port;
    socket_dscr = socket;
}
ClientData::ClientData() {
    client_ip = string("");
    client_port = 0;
    socket_dscr = -1;
}
string ClientData::get_ip() {return client_ip;}
int ClientData::get_port() {return client_port;}
int ClientData::get_socket_dscr() {return socket_dscr;}
