#include <iostream>
#include <string>
#include "Server.h"
#include "constants.h"
#include "UserException.h"
#include "UserLogger.h"
using namespace std;

int main() {
    int exit_status = ServerCodes::SUCCESS;
    string ip;
    int port;
    int log_level;

    cout << "enter Server ip:" << endl;
    cin >> ip;
    cout << "enter Server port:" << endl;
    cin >> port;
    cout << "enter log level:" << endl;
    cin >> log_level;
    UserLogger::set_level(log_level);
    
    cout << "entered:" << ip << ":" << port << endl;
    cout << "log level = " << log_level << endl;

    try {
        Server server = Server(ip.c_str(), port);
        //server.run();
    }
    catch(ServerException e) {
        exit_status = ServerCodes::FAILED;
        UserLogger::log_error(string("Server"), e);
    }
    
    return exit_status;
}