#include <arpa/inet.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <functional>
#include "webserver.h"
#include "threadpool.h"

const int WebServer::MAX_FD = 65535;

WebServer::WebServer(int thread_num) : threadpool_(new Threadpool(thread_num)), epoller_(new Epoll()) {
    listen_event_ = EPOLLIN | EPOLLET;
    conn_event_ = EPOLLONESHOT | EPOLLRDHUP | EPOLLET;
    src_dir_ = getcwd(NULL, 256);
    strncat(src_dir_, "/static/", 20);

    HttpConn::src_dir_ = src_dir_;
    HttpConn::user_cnt_.store(0);
    HttpConn::is_ET_ = true;
    if (!init_socket()) {
        is_closed_ = true;
    }
    is_closed_ = false;
}

WebServer::~WebServer() {
    close(listen_fd_);
    is_closed_ = true;
    free(src_dir_);
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
            continue;
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
    do {
        int cfd = accept(listen_fd_, (sockaddr*)&client_addr, &client_len);
        if (cfd <= 0) {
            return;
        } else if (HttpConn::user_cnt_.load() >= MAX_FD) {
            printf("Server busy\n");
            return;
        }
        add_client(cfd, client_addr);
    } while (listen_event_ & EPOLLET);
}

void WebServer::deal_read(HttpConn* client) {
    assert(client);
    threadpool_->append(std::bind(&WebServer::web_read, this, client));
}

void WebServer::deal_write(HttpConn* client) {
    assert(client);
    threadpool_->append(std::bind(&WebServer::web_write, this, client));
}

void WebServer::web_read(HttpConn* client) {
    int read_err_no = -1;
    auto ret = client->read(&read_err_no);
    if (ret <= 0 && read_err_no != EAGAIN) {
        close_conn(client);
        return;
    }

    web_process(client);
}

void WebServer::web_process(HttpConn* client) {
    if (client->process()) {
        epoller_->mod_fd(client->get_fd(), conn_event_ | EPOLLOUT); // 如果只有EPOLLOUT或者EPOLLIN就会错
    } else{
        epoller_->mod_fd(client->get_fd(), conn_event_ | EPOLLIN);
    }
}

void WebServer::web_write(HttpConn* client) {
    int write_err_no = -1;
    auto ret = client->write(&write_err_no);
    if (client->to_write_bytes() == 0) {
        // finish write
        if (client->is_keep_alive()) {
            web_process(client);
            return;
        }
    } else if (ret < 0) {
        if (write_err_no == EAGAIN) {
            epoller_->mod_fd(client->get_fd(), conn_event_ | EPOLLOUT);
            return;
        }
    }

    close_conn(client);
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

void WebServer::close_conn(HttpConn* client) {
    assert(client);
    epoller_->remove_fd(client->get_fd());
    client->close_conn();
}