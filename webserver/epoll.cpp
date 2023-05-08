#include <unistd.h>
#include <assert.h>
#include "epoll.h"

Epoll::Epoll(int max_events) : epoll_fd_(epoll_create(100)), ep_events_(max_events) {
    assert(epoll_fd_ >= 0 && ep_events_.size() > 0);
}

Epoll::~Epoll() {
    close(epoll_fd_);
}

int Epoll::wait(int timeout) {
    return epoll_wait(epoll_fd_, &ep_events_[0], static_cast<int>(ep_events_.size()), timeout);
}

