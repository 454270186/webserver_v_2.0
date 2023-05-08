#ifndef _WEB_SERVER
#define _WEB_SERVER
#include <memory>
#include "threadpool.h"
#include "epoll.h"

using std::unique_ptr;

class WebServer {
public:
    WebServer();
    ~WebServer();

private:
    unique_ptr<Threadpool> threadpool_;
    unique_ptr<Epoll> epoller_;
};

#endif