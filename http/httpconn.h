#ifndef _HTTP_CONN
#define _HTTP_CONN
#include <atomic>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/uio.h> 
#include "buffer.h"
#include "httprequest.h"
#include "httpresponse.h"

using std::atomic;

class HttpConn {
public:
    HttpConn();
    ~HttpConn();

    void init(int fd, sockaddr_in addr);
    void close_conn();

    ssize_t read(int* err_no);
    ssize_t write(int* err_no);
    
    bool process() {return true; };

    int get_fd() { return fd_; }

    static const char* src_dir_;
    static atomic<int> user_cnt_;
    static bool is_ET_;

    int num{100000}; // for debug
private:
    int fd_;
    sockaddr_in addr_;

    bool is_closed_{true};

    int iov_cnt_;
    iovec ioc_[2];

    Buffer read_buf_;
    Buffer write_buf_;

    HttpRequest req_;
    HttpResponse res_;
};

#endif