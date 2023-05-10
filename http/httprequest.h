#ifndef _HTTP_REQ
#define _HTTP_REQ
#include <string>
#include <unordered_map>

using std::string;
using std::unordered_map;

class HttpRequest {
public:
    enum PARSE_STATE {
        REQUEST_LINE = 0,
        HEADER,
        BODY,
        FINISH,
    };

    enum HTTP_CODE {
        NO_REQUEST = 0,
        GET_REQUEST,
        BAD_REQUEST,
        NO_RESOURSE,
        FORBIDDENT_REQUEST,
        FILE_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTION,
    };

    HttpRequest();
    ~HttpRequest() = default;


private:
    string method_;
    string url_;
    string version_;
    
    unordered_map<string, string> headers_{};
};  

#endif