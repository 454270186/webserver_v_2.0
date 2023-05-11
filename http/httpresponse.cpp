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

    add_status_line(buffer);
    add_header(buffer);
    add_content(buffer);
}

void HttpResponse::add_status_line(Buffer& buffer) {
    string status;
    if (HTTP_STATUS_CODE.count(status_code_) == 1) {
        status = HTTP_STATUS_CODE.find(status_code_)->second;
    } else {
        status_code_ = 400; // Bad request
        status = HTTP_STATUS_CODE.find(400)->second;
    }

    buffer.append("HTTP/1.1" + std::to_string(status_code_) + status + "\r\n");
}

void HttpResponse::add_header(Buffer& buffer) {
    // Connecttion + Content-type
    buffer.append("Connection: ");
    if (is_keep_alive_) {
        buffer.append("keep-alive\r\n");
    } else {
        buffer.append("close\r\n");
    }

    buffer.append("Content-type: " + get_file_type() + "\r\n");
}

/// @brief memory mapping the source file and add content-length header
/// @param buffer 
void HttpResponse::add_content(Buffer& buffer) {
    int src_fd = open((src_dir_ + src_file_).c_str(), O_RDONLY);
    if (src_fd <= 0) {
        printf("File not found\n");
        return;
    }

    void* m_ret = mmap(NULL, m_file_stat_.st_size, PROT_WRITE, MAP_PRIVATE, src_fd, 0);
    if (m_ret == MAP_FAILED) {
        printf("Error while file mapping\n");
        return;
    }

    m_file_ = (char*)m_ret;
    close(src_fd);
    buffer.append("Content-length: " + std::to_string(m_file_stat_.st_size) + "\r\n\r\n");
}

string HttpResponse::get_file_type() {
    size_t suf_index = src_file_.find_last_of('.');
    if (suf_index == string::npos) {
        return "text/plain";
    }

    string file_suffix = src_file_.substr(suf_index);
    if (FILE_TYPE.count(file_suffix) != 1) {
        return "text/plain";
    }

    return FILE_TYPE.find(file_suffix)->second;
}