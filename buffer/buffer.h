#ifndef _BUFFER
#define _BUFFER
#include <unistd.h>
#include <vector>
#include <atomic>
#include <string>

using std::vector;
using std::string;

class Buffer {
public:
    Buffer(int init_buffer_size = 1024);
    ~Buffer() = default;

    size_t writable_size();
    size_t readable_size();
    size_t pre_size();

    void retrieve_all();
    void retrieve(size_t len);
    void retrieve_until(const char* end);

    void check_for_write(size_t len);
    void has_writen(size_t len); // add write_pos_
    const char* peek(); // return the string started at first unread char
    const char* begin_write(); // return the strinf starten at first unwrite char

    void append(const string& str); // accept for c++ string
    void append(const char* str, size_t len); // accept for c string

    ssize_t Read_fd(int fd, int* err_no);
    ssize_t Write_fd(int fd, int* err_no);

    void print_buf(); // for debug
    
private:
    char* begin_ptr_();
    const char* begin_ptr_() const;
    void make_space(int space_size);

    vector<char> buffer_{};
    std::atomic<size_t> read_pos_;
    std::atomic<size_t> write_pos_;
};

#endif