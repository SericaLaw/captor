#include <sys/epoll.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <functional>

using namespace std;

#include "lib/tcp_listener.h"

const int MAX_EVENTS = 128;

struct Channel {
    int fd;
    int efd;

    function<void(int, int, uint32_t)> read_callback;

    void on_event(uint32_t events) {
        if ((events & EPOLLIN) && (read_callback != nullptr)) {
            read_callback(fd, efd, events);
        }
    }
};

void read_callback(int conn_fd, int efd, uint32_t events) {
    TcpConnection conn{conn_fd};
    string msg;
    if (!(msg = conn.receive_line()).empty()) {
        cout << "server received: " << msg;
        conn.send(msg);
    } else {
        conn.close();
        cout << "server closed conn_fd: " << conn_fd << endl;
        struct epoll_event event{};
        epoll_ctl(efd, EPOLL_CTL_DEL, conn_fd, &event);
    }
}

void accept_callback(int listen_fd, int efd, uint32_t events) {
    TcpListener listener{listen_fd};
    TcpConnection conn = listener.accept();
    conn.set_nonblocking();
    int conn_fd = conn.conn_fd();

    struct epoll_event event{};

    event.events = EPOLLIN | EPOLLET;
    struct Channel *channel = new Channel{conn_fd, efd, read_callback};
    event.data.ptr = channel;
    if (epoll_ctl(efd, EPOLL_CTL_ADD, conn_fd, &event) == -1) {
        cerr << "epoll_ctl add failed: " << strerror(errno) << endl;
        exit(1);
    }
}

int main() {
    int efd;

    struct epoll_event event{};
    int n;
    vector<struct epoll_event> events(MAX_EVENTS);
    TcpListener listener;
    listener.listen(8888, true);

    efd = epoll_create1(EPOLL_CLOEXEC);
    if (efd == -1) {
        cerr << "epoll create failed: " << strerror(errno) << endl;
        exit(1);
    }

    event.events = EPOLLIN | EPOLLET;
    struct Channel *acceptor = new Channel{listener.listen_fd(), efd, accept_callback};
    event.data.ptr = acceptor;

    if (epoll_ctl(efd, EPOLL_CTL_ADD, listener.listen_fd(), &event) == -1) {
        cerr << "epoll_ctl add failed: " << strerror(errno) << endl;
        exit(1);
    }

    while (true) {
        n = epoll_wait(efd, events.data(), MAX_EVENTS, -1);
        for (int i = 0; i < n; i++) {
            if (events[i].events & (EPOLLERR | EPOLLHUP) ||
                !(events[i].events & EPOLLIN)) {
                cerr << "epoll error" << endl;
                struct Channel *channel = (Channel *) events[i].data.ptr;
                epoll_ctl(efd, EPOLL_CTL_DEL, channel->fd, &events[i]);
                close(channel->fd);
                delete channel;
            } else {
                if (events[i].events & EPOLLIN) {
                    cout << "epoll in\n";
                }
                struct Channel *channel = (Channel *) events[i].data.ptr;
                channel->on_event(events[i].events);
            }
        }
    }
}