#include <sys/epoll.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <vector>
#include <cstring>

using namespace std;

#include "lib/tcp_listener.h"

const int MAX_EVENTS = 128;
int main() {
    int efd;

    struct epoll_event event{};
    int n_ready;
    vector<struct epoll_event> events(MAX_EVENTS);
    TcpListener listener;
    listener.listen(8888, true);

    efd = epoll_create1(EPOLL_CLOEXEC);
    if (efd == -1) {
        cerr << "epoll create failed: " << strerror(errno) << endl;
        exit(1);
    }

    event.data.fd = listener.listen_fd();
    event.events = EPOLLIN | EPOLLET;

    if (epoll_ctl(efd, EPOLL_CTL_ADD, event.data.fd, &event) == -1) {
        cerr << "epoll_ctl add failed: " << strerror(errno) << endl;
        exit(1);
    }

    while (true) {
        n_ready = epoll_wait(efd, events.data(), MAX_EVENTS, -1);
        for (int i = 0; i < n_ready; i++) {
            if (events[i].events & (EPOLLERR | EPOLLHUP) ||
                !(events[i].events & EPOLLIN)) {
                cerr << "epoll error" << endl;
                epoll_ctl(efd, EPOLL_CTL_DEL, events[i].data.fd, &events[i]);
                close(events[i].data.fd);
            } else if (listener.listen_fd() == events[i].data.fd) {
                TcpConnection conn = listener.accept();
                conn.set_nonblocking();
                event.data.fd = conn.conn_fd();
                event.events = EPOLLIN | EPOLLET;
                if (epoll_ctl(efd, EPOLL_CTL_ADD, event.data.fd, &event) == -1) {
                    cerr << "epoll_ctl add failed: " << strerror(errno) << endl;
                    exit(1);
                }
            } else {
                TcpConnection conn{events[i].data.fd};
                string msg;
                if (!(msg = conn.receive_line()).empty()) {
                    cout << "server received: " << msg;
                    conn.send(msg);
                } else {
                    conn.close();
                    cout << "server closed conn_fd: " << events[i].data.fd << endl;
                    epoll_ctl(efd, EPOLL_CTL_DEL, events[i].data.fd, &events[i]);
                }
            }
        }
    }
}