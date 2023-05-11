#include <assert.h>
#include <unistd.h>
#include "httpconn.h"

const char* HttpConn::src_dir_;
atomic<int> HttpConn::user_cnt_;
bool HttpConn::is_ET_;

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
    res_.unmap_file();
    if (!is_closed_) {
        is_closed_ = true;
        user_cnt_--;
        close(fd_);
    }
}

ssize_t HttpConn::read(int* err_no) {
    ssize_t len = -1;
    do {
        len = read_buf_.Read_fd(fd_, err_no);
        if (len <= 0) {
            break;
        }
    } while (is_ET_);

    return len;
}

ssize_t HttpConn::write(int* err_no) {
    ssize_t len = -1;
    do {
        len = writev(fd_, ioc_, iov_cnt_);
        if (len < 0) {
            *err_no =  errno;
            return len;
        }
        if (ioc_[0].iov_len + ioc_[1].iov_len == 0) {
            // 缓冲区数据已写完
            break;
        } else if (static_cast<size_t>(len) > ioc_[0].iov_len) {
            ioc_[1].iov_base = (uint8_t*)ioc_[1].iov_base + (static_cast<size_t>(len) - ioc_[0].iov_len);
            ioc_[1].iov_len -= static_cast<size_t>(len) - ioc_[0].iov_len;
            
            if (ioc_[0].iov_len) {
                write_buf_.retrieve_all();
                ioc_[0].iov_len = 0;
            }
        } else {
            ioc_[0].iov_base = (uint8_t*)ioc_[0].iov_base + static_cast<size_t>(len);
            ioc_[0].iov_len -= static_cast<size_t>(len);
            write_buf_.retrieve(len);
        }
    } while (is_ET_ || to_write_bytes() > 10240);

    return len;
}

/// @brief process the http request
/// @return true if this htto connection is ready for write
bool HttpConn::process() {
    req_.init();
    if (read_buf_.readable_size() <= 0) {
        return false;
    } else if (req_.parse(read_buf_)) {
        printf("URL: %s\n", req_.get_url().c_str());
        res_.init(src_dir_, req_.get_url(), req_.is_keep_alive(), 200);
    } else {
        // 404 not found
        printf("404 not found\n");
        res_.init(src_dir_, req_.get_url(), false, 404);
    }
    res_.make_response(write_buf_);

    // 分散写：响应头 + 文件
    ioc_[0].iov_base = const_cast<char*>(write_buf_.peek());
    ioc_[0].iov_len = write_buf_.readable_size();

    if (res_.file_len() > 0 && res_.file()) {
        ioc_[1].iov_base = res_.file();
        ioc_[1].iov_len = res_.file_len();
    }

    printf("response length: %d\n", to_write_bytes());
    return true;
}