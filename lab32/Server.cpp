#include <sys/socket.h>
#include <arpa/inet.h> 
#include <errno.h>
#include <string.h>
#include <iostream>
#include <unistd.h>

#include "Server.h"
#include "constants.h"
#include "UserException.h"
#include "ClientHandler.h"
#include "Data.h"
#include "UserLogger.h"

using namespace std;

Server::Server(const char *ip, int port) {
    string logger_agent("Server");

    // init TCP-IPv4 socket
    server_socket = socket(AF_INET, SOCK_STREAM, SocketCodes::DEFAULT_PROTOCOL);
    if (server_socket == SocketCodes::FAILED) {
        string system_err_msg = string(strerror(errno));
        string msg = string("can't init server socket - ") + system_err_msg;

        throw ServerException(ServerException::SERVER_CREATE_ERROR, msg);
    }
    UserLogger::log(logger_agent, string("server socket created."), LogLevels::LOW);
    
    // init socket address we want to bind to:
    struct sockaddr_in server_address;

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons((uint16_t) port);
    in_addr_t address_value = inet_addr(ip);

    if ((int) address_value == SocketCodes::FAILED) {
        cout << "INVALID IP" << endl; // TODO REMOVE LATER
        string msg = string("invalid ip address: ") + string(ip);

        throw ServerException(ServerException::SERVER_CREATE_ERROR, string("invalid ip address: "));
    }
    server_address.sin_addr.s_addr = address_value;

    // bind server socket
    int bind_status = bind(server_socket, (struct sockaddr*) &server_address,  sizeof(server_address));
    if (bind_status == SocketCodes::FAILED) {
        string system_err_msg = string(strerror(errno));
        string msg = string("can't bind socket to address=") + string(ip) + string(", port=") + to_string(port) + string(" - ") + system_err_msg;

        throw ServerException(ServerException::SERVER_CREATE_ERROR, msg);
    }
    UserLogger::log(logger_agent, string("server socket binded."), LogLevels::LOW);

    int listen_status = listen(server_socket, SocketCodes::BACKLOG);
    listen_status = -1; // TODO REMOVE LATER
    if (listen_status == SocketCodes::FAILED) {
        cout << "LISTEN ERROR" << endl; // TODO REMOVE LATER
        string system_err_msg = string(strerror(errno));
        string msg = string("can't turn server socket to listen mode: ") + system_err_msg;

        throw ServerException(ServerException::SERVER_CREATE_ERROR, msg);
    }
    UserLogger::log(logger_agent, string("server socket turned in listening mode."), LogLevels::LOW);

    handler = new ClientHandler();
    interrupted = false;
}

void Server::run() {
    string log_agent("Server");

    while(!interrupted) {
        struct sockaddr_in client_addr;
        socklen_t address_len;

        int client_socket_dscr = accept(server_socket, (struct sockaddr*)&client_addr, &address_len);

        if (client_socket_dscr == SocketCodes::FAILED) {
            string system_err_msg = string(strerror(errno));
            string msg = string("accept client error - ") + system_err_msg;

            throw ServerException(ServerException::SERVER_EXECUTION_ERROR, msg);
        }
        // construct client data
        char* address = inet_ntoa(client_addr.sin_addr);
        string client_ip = string(address);
        int port = (int) ntohs(client_addr.sin_port);

        ClientData client_data = ClientData(client_ip, port, client_socket_dscr);
        UserLogger::log(log_agent, UserLogger::get_client_name(&client_data) + " accepted", LogLevels::LOW);
        try {
            handler->handle_new_client(client_data);
        }
        catch (HandlerException e) {
            UserLogger::log_error(string("Handler"), e);
            close(client_socket_dscr);
        }
    }
}

void Server::interrupt() {
    interrupted = true;
}

Server::~Server() {
    delete handler;
}