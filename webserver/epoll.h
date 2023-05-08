#ifndef _EPOLL
#define _EPOLL
#include <vector>
#include <sys/epoll.h>
#include <fcntl.h>

using std::vector;

class Epoll {
public:
    Epoll(int max_events = 2048);
    ~Epoll();

    bool add_fd(int fd);
    bool mod_fd(int fd);
    bool remove_fd(int fd);

    int wait(int timeout); // epoll_wait()

private:
    int epoll_fd_{-1};
    vector<epoll_event> ep_events_;
};

#endif