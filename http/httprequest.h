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

    HttpRequest() { init(); }
    ~HttpRequest() = default;

    void init();

    bool parse(Buffer& buffer);
    bool parse_req_line(const string& line);
    void parse_header(const string& line);
    void parse_body(const string& line);

    bool is_keep_alive();

private:
    PARSE_STATE state_;

    string method_;
    string url_;
    string version_;
    string body_;
    
    unordered_map<string, string> headers_{};
};  

#endif