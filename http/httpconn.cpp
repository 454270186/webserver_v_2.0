#include <assert.h>
#include <unistd.h>
#include "httpconn.h"

const char* HttpConn::src_dir_;
atomic<int> HttpConn::user_cnt_;

HttpConn::HttpConn() {
    fd_ = -1;
    addr_ = {0};
}

HttpConn::~HttpConn() {
    close_conn();
}

void HttpConn::init(int fd, sockaddr_in addr) {
    assert(fd > 0);
    fd_ = fd;
    addr_ = addr;
    read_buf_.retrieve_all();
    write_buf_.retrieve_all();
    is_closed_ = false;
    user_cnt_++;
    printf("current user cnt: %d\n", user_cnt_.load());
}

void HttpConn::close_conn() {
    if (!is_closed_) {
        is_closed_ = true;
        user_cnt_--;
        close(fd_);
    }
}