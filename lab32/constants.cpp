#include "constants.h"

int SocketCodes::FAILED = -1;
int SocketCodes::DEFAULT_PROTOCOL = 0;
int SocketCodes::NO_RECV_FLAGS = 0;
int SocketCodes::PEER_SHUTDOWN = 0;
int SocketCodes::BACKLOG = 5;
int PthreadCodes::SUCCESS = 0;
int Constants::PROXY_BUFF_SIZE = 16 * 1024; // 16 kb
int Constants::REQUEST_BUFF_SIZE = 1024;
int Constants::MIN_REQ_LEN = 4; // GET, HEAD
int Constants::NO_LINE = -1;
int Constants::PEER_PORT = 80;
int HTTPcodes::HTTP_VERSION_NOT_SUPPORTED = 505;
int HTTPcodes::NOT_IMPLEMENTED = 501;
int HTTPcodes::INTERNAL_SERVER_ERROR = 500;
string HTTPrequest::EXPECTED_HTTP_VERSION = "HTTP/1.0";
string HTTPrequest::GET = "GET";
string HTTPrequest::HEAD = "HEAD";
int ServerCodes::SUCCESS = 0;
int ServerCodes::FAILED = 1;
int LogLevels::LOW = 0;
int LogLevels::MEDIUM = 1;
int LogLevels::HIGHT = 2;

string HTTPcodes::get_status(int code) {
    if (code == HTTP_VERSION_NOT_SUPPORTED) {return string("HTTP Version Not Supported");}
    if (code == NOT_IMPLEMENTED) {return string("Not Implemented");}
    if (code == INTERNAL_SERVER_ERROR) {return string("Internal Server Error");}
    return string("");
}
