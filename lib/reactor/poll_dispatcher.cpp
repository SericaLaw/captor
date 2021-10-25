#include <sys/poll.h>
#include <iostream>
#include <cstring>
using namespace std;

#include "dispatcher.h"

PollDispatcher::PollDispatcher() {
    _events = vector<struct pollfd>(DISPATCHER_MAX_EVENTS);
    for (int i = 0; i < DISPATCHER_MAX_EVENTS; i++) {
        _events[i].fd = -1;
    }
}

bool PollDispatcher::add(const struct Channel &channel) {
    int fd = channel.fd;
    int events = 0;
    if (channel.readable()) {
        events |= POLLRDNORM;
    }
    if (channel.writable()) {
        events |= POLLWRNORM;
    }

    int i;
    for (i = 0; i < DISPATCHER_MAX_EVENTS; i++) {
        if (_events[i].fd < 0) {
            _events[i].fd = fd;
            _events[i].events = events;
            break;
        }
    }
    if (i == DISPATCHER_MAX_EVENTS) {
        cerr << "PollDispatcher add error: can not hold so many clients\n";
        return false;
    }
    return true;
}

bool PollDispatcher::remove(const struct Channel &channel) {
    int fd = channel.fd;

    int i;
    for (i = 0; i < DISPATCHER_MAX_EVENTS; i++) {
        if (_events[i].fd == fd) {
            _events[i].fd = -1;
            break;
        }
    }
    if (i == DISPATCHER_MAX_EVENTS) {
        cerr << "PollDispatcher remove error: can not find event with fd=" << fd << std::endl;
        return false;
    }
    return true;
}

bool PollDispatcher::update(const struct Channel &channel) {
    int fd = channel.fd;
    int events = 0;
    if (channel.readable()) {
        events |= POLLRDNORM;
    }
    if (channel.writable()) {
        events |= POLLWRNORM;
    }

    int i;
    for (i = 0; i < DISPATCHER_MAX_EVENTS; i++) {
        if (_events[i].fd == fd) {
            _events[i].events = events;
            break;
        }
    }
    if (i == DISPATCHER_MAX_EVENTS) {
        cerr << "PollDispatcher update error: can not find event with fd=" << fd << std::endl;
        return false;
    }
    return true;
}

vector<struct DispatcherEvent> PollDispatcher::dispatch() {
    int n_ready;
    if ((n_ready = poll(_events.data(), DISPATCHER_MAX_EVENTS, -1)) < 0) {
        cerr << "PollDispatcher dispatch error: " << strerror(errno) << endl;
        exit(1);
    }

    vector<DispatcherEvent> ret;

    for (int i = 0; i < DISPATCHER_MAX_EVENTS; i++) {
        int fd;
        if ((fd = _events[i].fd) < 0) {
            continue;
        }

        int revents = 0;
        if (_events[i].revents & (POLLRDNORM | POLLERR)) {
            revents |= EVENT_READ;
        }
        if (_events[i].revents & POLLWRNORM) {
            revents |= EVENT_WRITE;
        }
        if (revents != 0) {
            ret.push_back(DispatcherEvent{fd, revents});
            if (--n_ready <= 0) {
                break;
            }
        }
    }
    return move(ret);
}
