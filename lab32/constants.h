#pragma once
#include <string>

using namespace std;

namespace SocketCodes {
    extern int FAILED;
    extern int DEFAULT_PROTOCOL;
    extern int NO_RECV_FLAGS;
    extern int PEER_SHUTDOWN;
    extern int BACKLOG;
}

namespace PthreadCodes {
    extern int SUCCESS;
}

namespace Constants {
    extern int PROXY_BUFF_SIZE;
    extern int MIN_REQ_LEN;
    extern int NO_LINE;
    extern int PEER_PORT;
}

namespace LogLevels {
    extern int LOW;
    extern int MEDIUM;
    extern int HIGHT;
}

namespace HTTPrequest {
    extern string EXPECTED_HTTP_VERSION;
    extern string GET;
    extern string HEAD;
}

namespace HTTPcodes {
    extern int NOT_IMPLEMENTED;
    extern int HTTP_VERSION_NOT_SUPPORTED;
    extern int INTERNAL_SERVER_ERROR;

    string get_status(int code);
}
namespace ServerCodes {
    extern int SUCCESS;
    extern int FAILED;
}