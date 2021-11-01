#include <sys/epoll.h>
#include <iostream>
#include <cstring>
#include <vector>
#include <unistd.h>

using namespace std;

#include "dispatcher.h"


EpollDispatcher::EpollDispatcher() {
    _efd = epoll_create1(EPOLL_CLOEXEC);
    if (_efd == -1) {
        std::cerr << "EpollDispatcher init failed: " << strerror(errno) << std::endl;
        exit(1);
    }
    _events = std::vector<struct epoll_event>(DISPATCHER_MAX_EVENTS);
}

bool EpollDispatcher::add(const Channel *channel) {
    if (do_epoll_ctl(EPOLL_CTL_ADD, channel) == -1) {
        cerr << "EpollDispatcher add failed: " << strerror(errno) << endl;
        exit(1);
    }
    return true;
}

bool EpollDispatcher::remove(const Channel *channel) {
    if (do_epoll_ctl(EPOLL_CTL_DEL, channel) == -1) {
        cerr << "EpollDispatcher remove failed: " << strerror(errno) << endl;
        exit(1);
    }
    return true;
}

bool EpollDispatcher::update(const Channel *channel) {
    if (do_epoll_ctl(EPOLL_CTL_MOD, channel) == -1) {
        cerr << "EpollDispatcher update failed: " << strerror(errno) << endl;
        exit(1);
    }
    return true;
}

vector<struct DispatcherEvent> EpollDispatcher::dispatch() {
    int n_ready = epoll_wait(_efd, _events.data(), DISPATCHER_MAX_EVENTS, 1000);
    vector<struct DispatcherEvent> ret;
    for (int i = 0; i < n_ready; i++) {
        int fd = _events[i].data.fd;
        if (_events[i].events & (EPOLLERR | EPOLLHUP)) {
            cerr << "EpollDispatcher dispatch error\n";
            close(fd);
            continue;
        }
        int revents = 0;
        if (_events[i].events & EPOLLIN) {
            cout << "epoll in\n";
            revents |= EVENT_READ;
        }

        if (_events[i].events & EPOLLOUT) {
            cout << "epoll out\n";
            revents |= EVENT_WRITE;
        }

        if (revents != 0) {
            ret.push_back(DispatcherEvent{fd, revents});
        }
    }
    return ret;
}

int EpollDispatcher::do_epoll_ctl(int op, const Channel *channel) const {
    int fd = channel->fd();
    cout << "EpollDispatcher do_epoll_ctl fd=" << fd << endl;
    int events = EPOLLET;   // edge-triggered
    if (channel->readable()) {
        cout << "channel readable\n";
        events |= EPOLLIN;
    }
    if (channel->writable()) {
        cout << "channel writable\n";
        events |= EPOLLOUT;
    }

    struct epoll_event event{};
    event.data.fd = fd;
    event.events = events;
    return epoll_ctl(_efd, op, fd, &event);
}
