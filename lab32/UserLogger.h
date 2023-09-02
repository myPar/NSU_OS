#pragma once
#include <string>
#include "UserException.h"
#include "Data.h"

using namespace std;

class UserLogger {
private:
    static int level;
public:
    static const char* log_info;
    static string get_client_name(ClientData *data);
    static void set_level(int level);
    static void log(string agent, string msg, int log_level);
    static void log_error(string agent, BaseException exception);
    static void log_error(string agent, ServerWorkflowException exception);
};
