#ifndef _WEB_SERVER
#define _WEB_SERVER
#include <memory>
#include <unordered_map>
#include "threadpool.h"
#include "epoll.h"
#include "httpconn.h"

using std::unique_ptr;
using std::unordered_map;

class WebServer {
public:
    WebServer(int thread_num);
    ~WebServer();

    void run(); // start the server on default port
    void run(int port);

private:
    bool init_socket();
    int set_fd_non_block(int fd);

    void add_client(int fd, sockaddr_in addr);

    void deal_listen();
    void deal_read(HttpConn* client);
    void deal_write(HttpConn* client);

    void web_process(HttpConn* client) {};
    void web_read(HttpConn* client) {};
    void web_write(HttpConn* client) {};

    void close_conn(HttpConn* client) {};

    int port_{8080};
    bool is_closed_{false};
    int listen_fd_{-1};
    char* src_dir_{0};

    unique_ptr<Threadpool> threadpool_;
    unique_ptr<Epoll> epoller_;
    unordered_map<int, HttpConn> users_;

    uint32_t listen_event_;
    uint32_t conn_event_;
};

#endif