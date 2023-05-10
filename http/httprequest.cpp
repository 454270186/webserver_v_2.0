#include <algorithm>
#include <sstream>
#include "httprequest.h"
#include "buffer.h"

using std::search;
using std::istringstream;

void HttpRequest::init() {
    state_ = REQUEST_LINE;
    method_ = "";
    url_ = "";
    version_ = "";
    body_ = "";

    headers_.clear();
}

bool HttpRequest::parse(Buffer& buffer) {
    const char* CRLF = "\r\n";
    if (buffer.readable_size() <= 0) {
        return false;
    }

    while (buffer.readable_size() > 0 && state_ != FINISH) {
        const char* new_line_end = search(buffer.peek(), buffer.begin_write(), CRLF, CRLF + 2);
        string new_line(buffer.peek(), new_line_end);

        switch (state_) {
            case REQUEST_LINE:
                if (!parse_req_line(new_line)) {
                    return false;
                }
                break;
            case HEADER:
                parse_header(new_line);
                break;
            case BODY:
                parse_body(new_line);
                break;
            default:
                break;
        }
        if (new_line_end == buffer.begin_write()) {
            break;
        }
        buffer.retrieve_until(new_line_end + 2);
    }
    printf("[%s] [%s] [%s]\n", method_, url_, version_);
    return true;
}

bool HttpRequest::parse_req_line(const string& line) {
    istringstream req_line_stream(line);
    req_line_stream >> method_ >> url_ >> version_;

    if (method_ == "" || url_ == "" || version_ == "") {
        return false;
    }

    state_ = HEADER;
    return true;
}

void HttpRequest::parse_header(const string& line) {
    if (line == "\r\n") {
        state_ = BODY;
    }
    istringstream header_stream(line);
    string key, value;
    header_stream >> key >> value;

    headers_[key] = value;
}

// TODO: parse body of post request
void HttpRequest::parse_body(const string& line) {
    body_ = line;
    state_ = FINISH;
}

bool HttpRequest::is_keep_alive() {
    if (headers_.count("Connection:") == 1) {
        return headers_["Connection:"] == "Keep-alive" && version_ == "HTTP/1.1";
    }

    return false;
}