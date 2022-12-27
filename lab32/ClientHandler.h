#pragma once
#include <pthread.h>
#include <unordered_map>

#include "Data.h"

using namespace std;

class ClientHandler {
private:
    // close client socket, relize all necessary resources
    void close_connection(ClientData client_data, string logger_agent);
    
    // send http responce with exception code to client
    void send_error_to_client(ClientData client_data, int exception_code, string logger_agent);
    
    void send_request_to_peer(string logger_agent, int peer_socket, char* data_buff, int data_size);
    // read peer response and send it to client
    void send_response_to_client(string agent, int peer_socket, int client_socket);
    // send data through specified socket
    int send_to(string agent, int socket, const char* data_buff, int data_size);

    // reading client http request and returns it's total size
    int read_request(char* read_buff, int socket_dscr, string logger_agent);

    // returns http exception code (if failed) or SUCCESS (if ok)
    int check_allowed_request(char* buff, int string_len, string logger_agent);
    
    // get's host name from the absolute URI 
    string get_host(string absolute_uri);

    // get's URI from http-request
    string get_uri(char* buff, int string_len);
    
    // establishing the connection to specified domain with specified port, return socket dscr or throw an exception
    int connect_to_peer(string domain_name, int port, string logger_agent);
    
    // returns index of '\n' in buff or 'NO_LINE'
    int check_line(char* buff, int data_size);
public:
    void handle_new_client(ClientData client_data);
    void handle_client(ClientData client_data);
    ClientHandler();
    ~ClientHandler();
};