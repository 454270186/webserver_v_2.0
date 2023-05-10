#include "httpresponse.h"

const unordered_map<int, string> HttpResponse::HTTP_STATUS_CODE = {
    {200, "OK"},
    {400, "Bad Request"},
    {403, "Forbidden"},
    {404, "Not Found"},
};

const unordered_map<string, string> HttpResponse::FILE_TYPE = {
    { ".html",  "text/html" },
    { ".xml",   "text/xml" },
    { ".txt",   "text/plain" },
    { ".png",   "image/png" },
    { ".jpg",   "image/jpeg" },
    { ".jpeg",  "image/jpeg" },
    { ".css",   "text/css "},
    { ".js",    "text/javascript "},
};

void HttpResponse::init(const string& src_dir, const string& src_file, bool is_keep_alive = false, int code = -1) {
    if (m_file_) {
        unmap_file();
    }
    src_dir_ = src_dir;
    src_file_ = src_file;
    is_keep_alive_ = is_keep_alive;
    status_code_ = code;
    m_file_ = nullptr;
    m_file_stat_ = {0};
}

void HttpResponse::unmap_file() {
    if (m_file_) {
        munmap(m_file_, m_file_stat_.st_size);
        m_file_ = nullptr;
    }
}

void HttpResponse::make_response(Buffer& buffer) {
    if (stat((src_dir_ + src_file_).c_str(), &m_file_stat_) < 0 || S_ISDIR(m_file_stat_.st_mode)) {
        status_code_ = 404;
    } else if (!(m_file_stat_.st_mode & S_IROTH)) {
        status_code_ = 403;
    } else if (status_code_ == -1) {
        status_code_ = 200;
    }

    add_status(buffer);
    add_header(buffer);
    add_content(buffer);
}

void HttpResponse::add_status(Buffer& buffer) {
    
}

void HttpResponse::add_header(Buffer& buffer) {

}

void HttpResponse::add_content(Buffer& buffer) {

}