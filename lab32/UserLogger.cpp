#include <iostream>
#include "constants.h"
#include "UserLogger.h"

//default logger level is '0'
int UserLogger::level = -1;
// description of log levels:
const char *UserLogger::log_info = "log levels:\n"
"-1 - no any log messages (default)\n"
"0 - only main info about components initializing and execution stages\n"
"1 - debug info with request/responce parsing results\n"
"2 - full debug info with request/responce data printing\n";

void UserLogger::log(string agent, string msg, int level) {
    if (UserLogger::level >= level) {
        cout << "[" << agent << "] "<< "INFO: " << msg << endl;
    }
}

void UserLogger::set_level(int level) {
    if (level > 0) {
        UserLogger::level = level;
    }
}

void UserLogger::log_error(string agent, BaseException exception) {
    cerr << "[" << agent << "] " << "ERROR: " << exception.get_message() << endl;
}

void UserLogger::log_error(string agent, ServerWorkflowException exception) {
    cerr << "[" << agent << "]" << "ERROR: " << exception.code << " " << HTTPcodes::get_status(exception.code);
    if (exception.msg.length() > 0) {
        cerr << " - " << exception.msg;
    }
    cerr << endl;
}

string UserLogger::get_client_name(ClientData *data) {
    return string("Client|") + data->get_ip() + string(":") + to_string(data->get_port());
}
