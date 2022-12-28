#pragma once
#include <string>
using namespace std;

class HTTPconstructor {
public:
    static const int NO_HOST_IN_URL;
    static const int SUCCESS;
    static string create_peer_request(string host_name, string url);
};