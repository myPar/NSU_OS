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

typedef struct _args {
    string peer_address;
    int server_port;
    int thread_idx;
} args;

const char* req1 = "POST http://reqbin.com/echo/get/json/page/2 HTTP/1.1\n"
                    "Host: reqbin.com\n";
const char* req2 = "GET http://reqbin.com/echo/get/json/page/2 HTTP/1.0\n"
                    "Host: reqbin.com\n";
const char* req3 = "GET http://reqbin.com/echo/get/json/page/2 HTTP/1.0\n"
                    "Host: reqbin.com\n";

const char *requests[] = {req2, req1, req3};

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
    int send_status = send_request(client_idx, s);
    if (send_status == FAILED) {
        string msg = client_name + string(" send message error to perr ") + client_arg->peer_address + string(":") + to_string(dst_port);
        perror(msg.c_str());
        close(s);
    }
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
