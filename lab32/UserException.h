#pragma once
#include <exception>
#include <string>

using namespace std;

class BaseException: public exception {
public:
    string name;
    string type;
    string msg;

    enum Output{STDERR=0, STDOUT=1};
    
    BaseException(const string name, string type, string msg);
    BaseException();
    string get_message();
    void print_exeption(Output output);
};

class ServerException: public BaseException {
public:
    static const string SERVER_CREATE_ERROR;
    static const string SERVER_EXECUTION_ERROR;
    static const string name;
    ServerException(string type, string msg);
};

class HandlerException: public BaseException {
public:
    static const string name;
    static const string HANDLER_THREAD_CREATION;
    HandlerException(string type, string msg);
};

class ServerWorkflowException: public exception {
public:
    string msg;
    int code;

    ServerWorkflowException();
    ServerWorkflowException(int code, string msg);
};