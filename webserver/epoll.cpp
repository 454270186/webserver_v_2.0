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

bool Epoll::add_fd(int fd, uint32_t event) {
    if (fd < 0)
    return false;

    epoll_event epev;
    epev.data.fd = fd;
    epev.events = event;

    return epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &epev) == 0;
}

bool Epoll::mod_fd(int fd, uint32_t event) {
    if (fd < 0)
    return false;

    epoll_event epev;
    epev.data.fd = fd;
    epev.events = event;

    return epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &epev) == 0;
}

bool Epoll::remove_fd(int fd) {
    if (fd < 0)
    return false;

    return epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, NULL) == 0;
}

int Epoll::get_event_fd(size_t index) {
    assert(index >= 0 && index < ep_events_.size());
    return ep_events_[index].data.fd;
}

uint32_t Epoll::get_events(size_t index) {
    assert(index >= 0 && index < ep_events_.size());
    return ep_events_[index].events;
}