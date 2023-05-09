#ifndef _HTTP_CONN
#define _HTTP_CONN
#include <arpa/inet.h>

class HttpConn {
public:
    HttpConn(int fd, sockaddr_in addr) {};
    HttpConn() {};

private:
    int fd_;
    sockaddr_in addr_;
};

#endif