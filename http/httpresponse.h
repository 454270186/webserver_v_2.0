#ifndef _HTTP_RES
#define _HTTP_RES
#include <unordered_map>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include "buffer.h"

using std::unordered_map;
using std::string;

class HttpResponse {
public:
    HttpResponse() = default;
    ~HttpResponse() { unmap_file(); }

    void init(const string& src_dir, const string& src_file, bool is_keep_alive = false, int code = -1);
    void make_response(Buffer& buffer);
    void unmap_file();

    char* file() { return m_file_; }
    size_t file_len() { return m_file_stat_.st_size; }
    int get_code() { return status_code_; }

private:
    // make response
    void add_status(Buffer& buffer);
    void add_header(Buffer& buffer);
    void add_content(Buffer& buffer);

    string get_file_type();

    int status_code_{-1};
    bool is_keep_alive_{false};

    // static file
    string src_dir_ = "";
    string src_file_ = "";

    // memory mapping
    char* m_file_{nullptr};
    struct stat m_file_stat_{0};

    static const unordered_map<int, string> HTTP_STATUS_CODE;
    static const unordered_map<string, string> FILE_TYPE;
};

#endif