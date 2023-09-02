#include "UserException.h"
#include <iostream>
#include <cassert>

using namespace std;

BaseException::BaseException(const string n, string t, string m) {
    name = n;
    type = t;
    msg = m;
}
BaseException::BaseException() {
    name = "";
    type = "";
    msg = "";
}
string BaseException::get_message() {
    return name + string(" ") + type + string(" exception - ") + msg;
}
void BaseException::print_exeption(BaseException::Output output) {
    string message = get_message();
    
    switch (output) {
        case BaseException::Output::STDERR:
            cerr << message << endl;
            break;
        case BaseException::Output::STDOUT:
            cout << message << endl;
            break;
        default:
            assert(false);
    }
}

// ---Server exeption:
const string ServerException::name = "Server";
const string ServerException::SERVER_CREATE_ERROR = "Server creation";
const string ServerException::SERVER_EXECUTION_ERROR = "Server execution";

ServerException::ServerException(string type, string msg) : BaseException(ServerException::name, type, msg) {}

// ---Handler exeption
const string HandlerException::name = "Handler";
const string HandlerException::HANDLER_THREAD_CREATION = "Creation of the new handler thread";
HandlerException::HandlerException(string type, string msg) : BaseException(HandlerException::name, type, msg) {}

// ---workflow exeption
ServerWorkflowException::ServerWorkflowException() {
    code = 0;
    msg = string("");
};

ServerWorkflowException::ServerWorkflowException(int c, string m) {
    code = c;
    msg = m;
};
