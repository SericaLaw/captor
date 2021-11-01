#include <sys/poll.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <cstring>
#include <functional>
#include <unordered_map>

using namespace std;

#include "lib/tcp_listener.h"


const int MAX_EVENTS = 128;

class PollController {
public:
    vector<struct pollfd> events;

    PollController() {
        events = vector<struct pollfd>(MAX_EVENTS);
        for (int i = 0; i < MAX_EVENTS; i++) {
            events[i].fd = -1;
        }
    }

    void add_event(int sock_fd, short e) {
        int i;
        for (i = 0; i < MAX_EVENTS; i++) {
            if (events[i].fd < 0) {
                events[i].fd = sock_fd;
                events[i].events = e;
                break;
            }
        }
        if (i == MAX_EVENTS) {
            cerr << "PollController add_event error: can not hold so many clients\n";
        }
    }

    void remove_event(int sock_fd) {
        int i = 0;
        for (i = 0; i < MAX_EVENTS; i++) {
            if (events[i].fd == sock_fd) {
                events[i].fd = -1;
                break;
            }
        }
        if (i == MAX_EVENTS) {
            cerr << "PollController remove_event error: can not find such event\n";
        }
    }
};

struct Channel {
    int fd;

    function<void(int, PollController&, unordered_map<int, Channel>&)> read_callback;

    void on_event(short events, PollController &controller, unordered_map<int, Channel> &channel_map) {
        cout << "on event with sock_fd: " << fd << endl;
        if ((events & POLLRDNORM) && (read_callback != nullptr)) {
            read_callback(fd, controller, channel_map);
        }
    }
};

void read_callback(int conn_fd, PollController &controller, unordered_map<int, Channel> &channel_map) {
    cout << "in read callback\n";
    TcpConnection conn{conn_fd};
    string msg;
    if (!(msg = conn.blocking_receive_line()).empty()) {
        cout << "server received: " << msg;
        conn.blocking_send(msg);
    } else {
        conn.close();
        cout << "server closed conn_fd: " << conn_fd << endl;
        controller.remove_event(conn_fd);
        channel_map.erase(conn_fd);
    }
}

void accept_callback(int listen_fd, PollController &controller, unordered_map<int, Channel> &channel_map) {
    TcpListener listener{listen_fd};
    TcpConnection conn = listener.accept();
    conn.set_nonblocking();

    int conn_fd = conn.conn_fd();
    controller.add_event(conn_fd, POLLRDNORM);
    channel_map[conn_fd] = Channel{conn_fd, read_callback};
}


int main() {
    unordered_map<int, Channel> channel_map;
    TcpListener listener;
    listener.listen(8888, true);

    PollController controller;
    controller.add_event(listener.listen_fd(), POLLRDNORM);
    Channel acceptor{listener.listen_fd(), accept_callback};
    channel_map[listener.listen_fd()] = acceptor;

    int n_ready;

    while (true) {
        if ((n_ready = poll(controller.events.data(), MAX_EVENTS, -1)) < 0) {
            cerr << "poll error: " << strerror(errno) << endl;
            exit(1);
        }

        for (int i = 0; i < MAX_EVENTS; i++) {
            int sock_fd;
            if ((sock_fd = controller.events[i].fd) < 0) {
                continue;
            }
            if (controller.events[i].revents & POLLHUP) {
                cerr << "poll error" << endl;
                close(sock_fd);
                controller.events[i].fd = -1;
                channel_map.erase(sock_fd);
            } else if (controller.events[i].revents & (POLLRDNORM | POLLERR)) {
                channel_map[sock_fd].on_event(controller.events[i].revents, controller, channel_map);
                if (--n_ready <= 0) {
                    break;
                }
            }
        }
    }
}