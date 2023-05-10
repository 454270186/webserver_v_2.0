#include <iostream>
#include <assert.h>
#include <string.h>
#include <sys/uio.h>
#include "buffer.h"

Buffer::Buffer(int init_buffer_size) : buffer_(init_buffer_size), read_pos_(0), write_pos_(0) {}

size_t Buffer::writable_size() {
    return buffer_.size() - write_pos_;
}

size_t Buffer::readable_size() {
    return write_pos_ - read_pos_;
}

size_t Buffer::pre_size() {
    return read_pos_;
}

void Buffer::check_for_write(size_t len) {
    if (writable_size() < len) {
        make_space(len);
    }

    assert(writable_size() >= len);
    //printf("check for wirte...\n");
}

void Buffer::has_writen(size_t len) {
    write_pos_ += len;
}

const char* Buffer::peek() {
    return begin_ptr_() + read_pos_;
}

const char* Buffer::begin_write() {
    return begin_ptr_() + write_pos_;
}

void Buffer::append(const char* str, size_t len) {
    assert(str);
    check_for_write(len);
    std::copy(str, str + len, begin_ptr_() + write_pos_);
    has_writen(len);
}

void Buffer::append(const string& str) {
    append(str.c_str(), str.length());
}

char* Buffer::begin_ptr_() {
    return &*buffer_.begin();
}

const char* Buffer::begin_ptr_() const {
    return &*buffer_.begin();
}

void Buffer::make_space(int space_size) {
    if (writable_size() + pre_size() < space_size) {
        buffer_.resize(write_pos_ + space_size + 1);
        // printf("append the buffer size...\n");
        // printf("%ld\n", writable_size());
    } else {
        size_t readable = readable_size();
        std::copy(begin_ptr_() + read_pos_, begin_ptr_() + write_pos_, begin_ptr_());
        read_pos_ = 0;
        write_pos_ = read_pos_ + readable;
        assert(readable == readable_size());
    }
}

void Buffer::print_buf() {
    for (auto& ch : buffer_) {
        printf("%c", ch);
    }
    printf("\n");
}

/* I/O for fd */
ssize_t Buffer::Read_fd(int fd, int* err_no) {
    char buf[65535];
    const size_t writable = writable_size();
    struct iovec read_ioc[2];
    read_ioc[0].iov_base = begin_ptr_() + write_pos_;
    read_ioc[0].iov_len = writable;
    read_ioc[1].iov_base = buf;
    read_ioc[1].iov_len = sizeof(buf);

    ssize_t len = readv(fd, read_ioc, 2);
    if (len < 0) {
        printf("ERROR while readv in Read_fd\n");
        *err_no = errno;
    } else if (len <= writable) {
        has_writen(len);
    } else if (len > writable) {
        write_pos_ = buffer_.size();
        append(buf, len - writable);
    }

    return len;
}

ssize_t Buffer::Write_fd(int fd, int* err_no) {
    const size_t readable = readable_size();
    
    ssize_t len = write(fd, peek(), readable);
    if (len < 0) {
        *err_no = errno;
        return len;
    }

    read_pos_ += len;

    return len;
}

void Buffer::retrieve_all() {
    buffer_.clear();
    read_pos_ = 0;
    write_pos_ = 0;
}

/// @brief retrieve bytes with length len
/// @param len 
void Buffer::retrieve(size_t len) {
    assert(len <= readable_size());
    read_pos_ += len;
}

/// @brief retrieve all bytes until meets 'end'
/// @param end 
void Buffer::retrieve_until(const char* end) {
    assert(end >= peek());
    retrieve(end - peek());
}