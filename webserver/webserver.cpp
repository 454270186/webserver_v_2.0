#include <arpa/inet.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <functional>
#include "webserver.h"
#include "threadpool.h"

WebServer::WebServer(int thread_num) : threadpool_(new Threadpool(thread_num)), epoller_(new Epoll()) {
    listen_event_ = EPOLLIN;
    conn_event_ = EPOLLONESHOT | EPOLLRDHUP;
    src_dir_ = getcwd(NULL, 256);
    strncat(src_dir_, "/static/", 20);

    HttpConn::src_dir_ = src_dir_;
    HttpConn::user_cnt_ = 0;
    if (!init_socket()) {
        is_closed_ = true;
    }
}

WebServer::~WebServer() {
    close(listen_fd_);
    is_closed_ = true;
}

void WebServer::run() {
    if (!is_closed_) {
        printf("--------------------------------\n");
        printf("Server is running on port %d\n", port_);
        printf("--------------------------------\n");
    }
    
    while (!is_closed_) {
        int ev_cnt = epoller_->wait(-1);
        if (ev_cnt < 0) {
            printf("ERROR while epoll wait\n");
            exit(-1);
        }

        for (int i = 0; i < ev_cnt; i++) {
            int cur_fd = epoller_->get_event_fd(i);
            uint32_t cur_event = epoller_->get_events(i);

            if (cur_fd == listen_fd_) {
                deal_listen();
            } else if (cur_event & (EPOLLRDHUP | EPOLLERR | EPOLLHUP)) {
                assert(users_.count(cur_fd) > 0);
                close_conn(&users_[cur_fd]);
            } else if (cur_event & EPOLLIN) {
                assert(users_.count(cur_fd) > 0);
                deal_read(&users_[cur_fd]);
            } else if (cur_event & EPOLLOUT) {
                assert(users_.count(cur_fd) > 0);
                deal_write(&users_[cur_fd]);
            }
        }
    }

    printf("Server stoped...\n");
}

void WebServer::deal_listen() {
    sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int cfd = accept(listen_fd_, (sockaddr*)&client_addr, &client_len);
    add_client(cfd, client_addr);
}

void WebServer::deal_read(HttpConn* client) {
    printf("something to read\n");
    //threadpool_->append(std::bind(&WebServer::web_read, this, client));
}

void WebServer::deal_write(HttpConn* client) {
    printf("something to write\n");
    //threadpool_->append(std::bind(&WebServer::web_write, this, client));
}

void WebServer::add_client(int fd, sockaddr_in addr) {
    assert(fd > 0);
    users_[fd].init(fd, addr);
    // add to epoller
    epoller_->add_fd(fd, EPOLLIN | conn_event_);
    set_fd_non_block(fd);
    printf("client %d connected\n", addr.sin_addr.s_addr);
}

bool WebServer::init_socket() {
    listen_fd_ = socket(PF_INET, SOCK_STREAM, 0);
    if (listen_fd_ == -1) {
        printf("ERROR while init listen fd\n");
        return false;
    }

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8080);

    int optval = 1;
    int ret = setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(optval));
    if (ret == -1) {
        close(listen_fd_);
        printf("ERROR while setsockopt\n");
        return false;
    }

    ret = bind(listen_fd_, (sockaddr*)&server_addr, sizeof(server_addr));
    if (ret == -1) {
        close(listen_fd_);
        printf("ERROR while bind\n");
        return false;
    }

    ret = listen(listen_fd_, 100);
    if (ret == -1) {
        close(listen_fd_);
        printf("ERROR while listen\n");
        return false;
    }

    ret = epoller_->add_fd(listen_fd_, listen_event_);
    if (ret == 0) {
        close(listen_fd_);
        printf("ERROR while add listen fd\n");
        return false;
    }

    set_fd_non_block(listen_fd_);
    return true;
}

int WebServer::set_fd_non_block(int fd) {
    assert(fd > 0);
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
}