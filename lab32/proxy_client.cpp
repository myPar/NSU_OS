#include <pthread.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <iostream>
#include <strings.h>
#include <arpa/inet.h> 

using namespace std;

#define FAILED -1
#define SUCCESS 0
#define CLIENTS_COUNT 1
#define RECV_BUFF_SIZE 1024

typedef struct _args {
    string peer_address;
    int server_port;
    int thread_idx;
} args;

const char* req1 = "POST http://reqbin.com/echo/get/json/page/2 HTTP/1.1\r\n"
                    "Host: reqbin.com\n\n";
const char* req2 = "GET http://reqbin.com/echo/get/json/page/2 HTTP/1.0\r\n"
                    "Host: reqbin.com\n\n";
const char* req3 = "GET http://httpbin.org/get HTTP/1.0\r\n"
                    "Host: httpbin.org\n\n";
const char* req4 = "GET http://kremlin.ru/events/president/news HTTP/1.0\r\n"
                    "Host: kremlin.ru\n\n";
const char* req5 = "GET http://fit.ippolitov.me/CN_2/2021/list.html HTTP/1.0\r\n"
                    "Host: fit.ippolitov.me\n\n";

const char *requests[] = {req5, req2, req1};

int send_request(int request_idx, int socket) {
    int request_size = (int) strlen(requests[request_idx]);
    int totally_send = 0;
    while (true) {
        int send_count = send(socket, requests[request_idx] + totally_send, request_size - totally_send, 0);
        if (send_count == FAILED) {
            return FAILED;
        }
        totally_send += send_count;
        if (totally_send == request_size) {
            break;
        }
    }
    return SUCCESS;
}

int read_response(int socket, int client_idx) {
    char buffer[RECV_BUFF_SIZE];
    cout << "response to client " << client_idx << endl;

    while (true) {
        int read_count = recv(socket, buffer, RECV_BUFF_SIZE, 0);
        if (read_count == FAILED) {
            return FAILED;
        }
        if (read_count == 0) {
            break;
        }
        buffer[read_count] = '\0';
        cout << buffer;
    }
    cout << endl;

    return SUCCESS;
}

void *client_routine(void *arg) {
    args* client_arg = (args*) arg;

    int dst_port = client_arg->server_port;
    int client_idx = client_arg->thread_idx;
    const char* dst_address = client_arg->peer_address.c_str();
    
    string client_name = string("client ") + to_string(client_idx);
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == FAILED) {
        string msg = client_name + string(" - can't init socket");
        perror(msg.c_str());
        close(s);

        return (void*) FAILED;
    }
    struct sockaddr_in server_address;

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons((uint16_t) dst_port);
    in_addr_t address_value = inet_addr(dst_address);
    
    if ((int) address_value == FAILED) {
        string msg = client_name + string(" - can't get ip address of value - ") + client_arg->peer_address;
        perror(msg.c_str());
        close(s);

        return (void*) FAILED;      
    }
    server_address.sin_addr.s_addr = address_value;

    int connect_status = connect(s, (struct sockaddr*) &server_address, sizeof(server_address));
    cout << "connect status = " << connect_status << endl;
    if (connect_status == FAILED) {
        string msg = client_name + string(" can't connect to peer ") + client_arg->peer_address + string(":") + to_string(dst_port);
        perror(msg.c_str());
        close(s);

        return (void*) FAILED;
    }
    // send request:
    int send_status = send_request(client_idx, s);
    if (send_status == FAILED) {
        string msg = client_name + string(" send message to perr error ") + client_arg->peer_address + string(":") + to_string(dst_port);
        perror(msg.c_str());
        close(s);

        return (void*) FAILED;
    }
    // shutdown write:
    int shutdown_status = shutdown(s, SHUT_WR);
    if (shutdown_status == FAILED) {
        string msg = client_name + string(" shutdown write error");
        perror(msg.c_str());
        close(s);
        
        return (void*) FAILED;
    }
    // read proxy response:
    int read_response_status = read_response(s, client_idx);
    if (read_response_status == FAILED) {
        string msg = client_name + string(" receive response error from peer ") + client_arg->peer_address + string(":") + to_string(dst_port);
        perror(msg.c_str());
        close(s);

        return (void*) FAILED;
    }
    cout << client_name << " response read" << endl;
    close(s);

    return (void*) SUCCESS;
}

void init_args(args* threads_args, int server_port, string server_ip) {
    for(int i = 0; i < CLIENTS_COUNT; i++) {
        threads_args[i].peer_address = server_ip;
        threads_args[i].server_port = server_port;
        threads_args[i].thread_idx = i;
    }
}

int main() {
    pthread_t threads[CLIENTS_COUNT];
    args threads_args[CLIENTS_COUNT];
    int server_port;
    string server_ip;

    cout << "enter server ip:" << endl;
    cin >> server_ip;
    cout << "enter server port:" << endl;
    cin >> server_port;
    cout << "start clients.." << endl;

    init_args(threads_args, server_port, server_ip);
    
    // start clients:
    for (int i = 0; i < CLIENTS_COUNT; i++) {
        int code = pthread_create(&threads[i], NULL, client_routine, (void*) &threads_args[i]);
        if (code != SUCCESS) {
            char* sys_msg = strerror(code);
            cerr << "can't create thread " << i << ": " << sys_msg << endl;
            
            return FAILED;
        }
    }
    // join clients:
    for (int i = 0; i < CLIENTS_COUNT; i++) {
        int code = pthread_join(threads[i], NULL);
        if (code != SUCCESS) {
            char* sys_msg = strerror(code);
            cerr << "join error for thread " << i << ": " << sys_msg << endl;
        }
    }
    cout << "execution finished" << endl;

    return SUCCESS;
}

#undef FAILED
#undef SUCCESS
#undef CLIENTS_COUNT
#undef RECV_BUFF_SIZE