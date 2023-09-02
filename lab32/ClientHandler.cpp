#include <pthread.h>
#include <strings.h>
#include <iostream>
#include <sstream>
#include <netdb.h>
#include <arpa/inet.h> 
#include <unistd.h>
#include <resolv.h>
#include <cassert>

#include "ClientHandler.h"
#include "constants.h"
#include "UserException.h"
#include "Tokenizer.h"
#include "UserLogger.h"
#include "HTTPconstructor.h"

class StartHandleWrapper {
public:
    ClientHandler* handler;
    ClientData client_data;
    
    StartHandleWrapper(ClientData data, ClientHandler* const handler) {
        this->client_data = data;
        this->handler = handler;
    }
};

void *handle_routine(void *arg) {
    StartHandleWrapper *wrapper = static_cast<StartHandleWrapper*>(arg);

    ClientHandler *handler = wrapper->handler;
    ClientData data = wrapper->client_data;
    delete wrapper;

    handler->handle_client(data);
}

void throw_exception(int error_code, string type, string info) {
    char *sys_msg = strerror(error_code);
    string msg = info + string(": ") + string(sys_msg);

    throw HandlerException(HandlerException::HANDLER_THREAD_CREATION, msg);
}

void ClientHandler::handle_new_client(ClientData client_data) {
    // create detached thread for new client handling:
    pthread_attr_t attr;
    int code = pthread_attr_init(&attr);
    
    if (code != PthreadCodes::SUCCESS) {
        throw_exception(code, HandlerException::HANDLER_THREAD_CREATION, string("can't create pthread attribute"));
    }
    code = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (code != PthreadCodes::SUCCESS) {
        throw_exception(code, HandlerException::HANDLER_THREAD_CREATION, string("can't set detachstate for pthread attribute"));
    }
    StartHandleWrapper *wrapper = new StartHandleWrapper(client_data, this);

    code = pthread_create(NULL, &attr, handle_routine, wrapper);
    if (code != PthreadCodes::SUCCESS) {
        throw_exception(code, HandlerException::HANDLER_THREAD_CREATION, string("can't create a thread to handle client"));
    }
}

int ClientHandler::check_line(char* buff, int data_size) {
    // check \r\n and \n
    for (int i = 0; i < data_size; i++) {
        if(buff[i] == '\n') {
            return i;
        }
        if(buff[i] == '\r') {
            if (i < data_size - 1 && buff[i + 1] == '\n') {
                return i;
            }
        }
    }
    return Constants::NO_LINE;
}

int ClientHandler::check_allowed_request(char* buff, int string_len, string logger_agent) {
    char* line_buff = new char[string_len + 1];
    strncpy(line_buff, buff, string_len);
    line_buff[string_len] = '\0';   // result is not null-terminated according to the documentation

    string http_head = string(line_buff);
    string delimiter = string(" ");

    UserLogger::log(logger_agent, string("http head - ") + http_head, LogLevels::HIGHT);

    // check request type:
    string request_type = Tokenizer::get_token(http_head, delimiter, 0);
    if (request_type.length() == 0) {
        delete[] line_buff;
        return HTTPcodes::INTERNAL_SERVER_ERROR;    // just incorrect request
    }
    if (request_type != HTTPrequest::GET && request_type != HTTPrequest::HEAD) {
        delete[] line_buff;
        return HTTPcodes::NOT_IMPLEMENTED;
    }
    UserLogger::log(logger_agent, string("request type - ") + request_type, LogLevels::MEDIUM);

    // check http-version
    string http_version = Tokenizer::get_token(http_head, delimiter, 2);
    if (request_type.length() == 0) {
        delete[] line_buff;
        return HTTPcodes::INTERNAL_SERVER_ERROR;    // just incorrect request
    }
    UserLogger::log(logger_agent, string("http version - ") + http_version, LogLevels::MEDIUM);
    if (http_version != HTTPrequest::EXPECTED_HTTP_VERSION) {
        delete[] line_buff; 
        return HTTPcodes::HTTP_VERSION_NOT_SUPPORTED;
    }
    delete[] line_buff;

    return ServerCodes::SUCCESS;
}

int ClientHandler::read_request(char* read_buff, int socket_dscr, string logger_agent) {
    int totally_read = 0;
    int read_count = 0;
    bool init_state = true;
    
    UserLogger::log(logger_agent, "start reading the request..", LogLevels::MEDIUM);
    // read header:
    while(true) {
        read_count = recv(socket_dscr, read_buff + totally_read, Constants::PROXY_BUFF_SIZE - totally_read, SocketCodes::NO_RECV_FLAGS);

        if (read_count == SocketCodes::FAILED) {
            // buffer overflow (GET and HEAD requests are too few for it, so this is invalid request)
            string msg = string("buffer overflow, can't read the request");
            throw ServerWorkflowException(HTTPcodes::INTERNAL_SERVER_ERROR, msg);
        }
        if (read_count == SocketCodes::PEER_SHUTDOWN) {
            // request is read
            break;
        }
        UserLogger::log(logger_agent, to_string(read_count) + string(" bytes was read"), LogLevels::MEDIUM);
        totally_read += read_count;
        read_buff[totally_read] = '\0';
        UserLogger::log(logger_agent, string("read data: ") + string(read_buff), LogLevels::HIGHT);

        // check request type (should be GET or HEAD)
        if (init_state) {
            int line_status = check_line(read_buff, totally_read);

            if (line_status != Constants::NO_LINE) {
                UserLogger::log(logger_agent, string("new line pos=") + to_string(line_status), LogLevels::HIGHT);
                int status = check_allowed_request(read_buff, line_status, logger_agent);
                
                if (status != ServerCodes::SUCCESS) {
                    throw ServerWorkflowException(status, "");
                }
                init_state = false;                                
            }
        }        
    }
    UserLogger::log(logger_agent, string("request is read, total size - ") + 
    to_string(totally_read) + string(" byte"), LogLevels::MEDIUM);
    read_buff[totally_read] = '\0';
    UserLogger::log(logger_agent, string("request data: ") + string(read_buff), LogLevels::HIGHT);

    return totally_read;
}

string ClientHandler::get_host(string full_uri) {
    string prefix_removed = Tokenizer::get_token(full_uri, "//", 1);    // http:// removed
    string host_name = Tokenizer::get_token(prefix_removed, "/", 0);    // part by left side of first '/' is domain name
    
    if (host_name.length() == 0) {
        string msg = string("can't get host from uri. uri=") + full_uri;
        throw ServerWorkflowException(HTTPcodes::INTERNAL_SERVER_ERROR, msg);
    }
    return host_name;
}

string ClientHandler::get_uri(char* buff, int string_len) {
    if (string_len == Constants::NO_LINE) {
        string msg = string("can't parse uri. invalid header");
        throw ServerWorkflowException(HTTPcodes::INTERNAL_SERVER_ERROR, msg);
    }
    assert(string_len > 0);
    char* line_buff = new char[string_len];
    strncpy(line_buff, buff, string_len);
    string http_head = string(line_buff);
    delete[] line_buff;

    string delimiter = string(" ");

    string full_uri = Tokenizer::get_token(http_head, delimiter, 1);
    if (full_uri.length() == 0) {
        string msg = string("can't parse uri. request head=") + string(http_head);
        throw ServerWorkflowException(HTTPcodes::INTERNAL_SERVER_ERROR, msg);
    }
    return full_uri;
}

int ClientHandler::connect_to_peer(string domain_name, int port, string logger_agent) {
    UserLogger::log(logger_agent, string("start connecting to peer ") + domain_name 
    + string(":") + to_string(port), LogLevels::MEDIUM);

    const char* name = domain_name.c_str();
    hostent *resolve_result = gethostbyname(name);
    
    if (resolve_result == NULL) {
        // h_errno is set
        const char* msg = hstrerror(h_errno);
        
        throw ServerWorkflowException(HTTPcodes::INTERNAL_SERVER_ERROR, string("can't resolve " + domain_name + ": " + string(msg)));
    }
    char** address_list = resolve_result->h_addr_list;
    in_addr* address_value = (in_addr*) address_list[0];
    
    string string_address = string(inet_ntoa(*address_value));
    UserLogger::log(logger_agent, string("resolved address of ") + domain_name + string(" - ") + string_address, LogLevels::MEDIUM);

    // create socket:
    int s = socket(AF_INET, SOCK_STREAM, SocketCodes::DEFAULT_PROTOCOL);
    if (s == SocketCodes::FAILED) {
        string msg = string("can't create peer socket: ") + string(strerror(errno));
        throw ServerWorkflowException(HTTPcodes::INTERNAL_SERVER_ERROR, msg);
    }
    // init peer address structure:
    struct sockaddr_in peer_address;

    peer_address.sin_family = AF_INET;
    peer_address.sin_port = htons((uint16_t) port);
    peer_address.sin_addr = *address_value;

    // connect socket to peer address:
    UserLogger::log(logger_agent, string("connecting.."), LogLevels::MEDIUM);
    int connect_status = connect(s, (struct sockaddr*) &peer_address, sizeof(peer_address));
    if (connect_status == SocketCodes::FAILED) {
        string msg = string("can't connect to peer: ") + string(strerror(errno));
        throw ServerWorkflowException(HTTPcodes::INTERNAL_SERVER_ERROR, msg);
    }
    UserLogger::log(logger_agent, string("connected"), LogLevels::MEDIUM);
    
    return s;
}

void ClientHandler::send_request_to_peer(string agent, int peer_socket, char* data_buff, int data_size, string request) {
    size_t request_size = request.length();
    const char *request_data = request.c_str();
    strncpy(data_buff, request_data, request_size);
    data_buff[request_size] = '\0';

    UserLogger::log(agent, string("request, converted to peer format: ") + string(data_buff), LogLevels::MEDIUM);

    int totally_send = 0;
    
    while (true) {
        int send_count = send(peer_socket, data_buff + totally_send, data_size - totally_send, SocketCodes::NO_SEND_FLAGS);

        if (send_count == SocketCodes::FAILED) {
            char* sys_msg = strerror(errno);
            string msg = string("error while writing request to peer: ") + string(sys_msg);

            throw ServerWorkflowException(HTTPcodes::INTERNAL_SERVER_ERROR, msg);
        }
        UserLogger::log(agent, to_string(send_count) + string(" bytes send to peer"), LogLevels::HIGHT);
        totally_send += send_count;
        if (totally_send == data_size) {
            break;
        }
    }
    shutdown(peer_socket, SHUT_WR);
}

int ClientHandler::send_to(string agent, int socket, const char* data_buff, int data_size) {
    int totally_send = 0;
    
    while (true) {
        int send_count = send(socket, data_buff + totally_send, data_size - totally_send, SocketCodes::NO_SEND_FLAGS);
        if (send_count == SocketCodes::FAILED) {
            return SocketCodes::FAILED;
        }
        UserLogger::log(agent, to_string(send_count) + string(" bytes send"), LogLevels::HIGHT);
        totally_send += send_count;

        if (totally_send == data_size) {
            break;
        }
    }
    return totally_send;
}

void ClientHandler::send_response_to_client(string agent, int peer_socket, int client_socket) {
    char* buff = new char[Constants::RECV_BUFF_SIZE];
    int totally_read = 0;

    while (true) {
        int read_count = recv(peer_socket, buff, Constants::RECV_BUFF_SIZE, SocketCodes::NO_RECV_FLAGS);
        
        if (read_count == SocketCodes::FAILED) {
            delete[] buff;
            char* sys_msg = strerror(errno);
            string msg = string("error while reading peer response - ") + string(sys_msg);

            throw ServerWorkflowException(HTTPcodes::INTERNAL_SERVER_ERROR, msg);
        }
        if (read_count == SocketCodes::PEER_SHUTDOWN) {
            UserLogger::log(agent, string("full response is sended to client, totall - ") +
            to_string(totally_read) + string(" bytes"), LogLevels::MEDIUM);
            
            break;
        }
        totally_read += read_count;
        UserLogger::log(agent, to_string(read_count) + string(" bytes of peer response received"), LogLevels::HIGHT);

        // send received bytes to client 
        int send_status = send_to(agent, client_socket, buff, read_count);
        if (send_status == SocketCodes::FAILED) {
            delete[] buff;
            char* sys_msg = strerror(errno);
            string msg = string("error while sending response to client") + string(sys_msg);

            throw ServerWorkflowException(HTTPcodes::INTERNAL_SERVER_ERROR, msg);
        }
        UserLogger::log(agent, to_string(send_status) + string(" bytes of response sended to client"), LogLevels::HIGHT);
    }
    delete[] buff;
}

void ClientHandler::handle_client(ClientData data) {
    int client_socket = data.get_socket_dscr();
    char *in_buffer = new char[Constants::REQUEST_BUFF_SIZE];
    char* out_buffer;

    string logger_agent = UserLogger::get_client_name(&data);
    int peer_socket;

    try {
        int data_size = read_request(in_buffer, client_socket, logger_agent);
        int first_line_length = check_line(in_buffer, data_size);
        UserLogger::log(logger_agent, "line checked", LogLevels::HIGHT);

        string uri = get_uri(in_buffer, first_line_length);
        UserLogger::log(logger_agent, string("uri - ") + uri, LogLevels::MEDIUM);
        
        string host_name = get_host(uri);
        UserLogger::log(logger_agent, string("host - ") + host_name, LogLevels::MEDIUM);
        
        peer_socket = ClientHandler::connect_to_peer(host_name, Constants::PEER_PORT, logger_agent);

        out_buffer = new char[Constants::REQUEST_BUFF_SIZE];
        string peer_request = HTTPconstructor::create_peer_request(host_name, uri);

        send_request_to_peer(logger_agent, peer_socket, in_buffer, data_size, peer_request);
        send_response_to_client(logger_agent, peer_socket, client_socket);
        
        close(peer_socket); // TODO remove later
        close(client_socket);
    }
    catch(ServerWorkflowException e) {
        delete[]  in_buffer;
        delete[] out_buffer;
        UserLogger::log_error(string("ClientHandler"), e);
        //send_error_resp(data, request_status);
        //close_connection(data);
        close(peer_socket);
        close(client_socket);
    }
    delete[]  in_buffer;
    delete[] out_buffer;
}

ClientHandler::ClientHandler() {
}

ClientHandler::~ClientHandler() {
}