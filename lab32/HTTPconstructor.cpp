#include "HTTPconstructor.h"
#include "constants.h"

const int HTTPconstructor::NO_HOST_IN_URL = 1;
const int HTTPconstructor::SUCCESS = 0;

string HTTPconstructor::create_peer_request(string host_name, string url) {
    int pos = url.find(host_name);
    if (pos == string::npos) {
        return string("");
    }
    url.erase(0, pos + host_name.length());
    string request = HTTPrequest::GET + string(" ") + url + string(" ") + 
    HTTPrequest::EXPECTED_HTTP_VERSION +
    string("\n") + string("Host: ") + host_name + string("\n\n");

    return request;
}