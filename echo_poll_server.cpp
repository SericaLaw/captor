#include <sys/poll.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <vector>
#include <cstring>

using namespace std;

#include "tcp_listener.h"

const int MAX_EVENTS = 128;

int main() {
    TcpListener listener;
    listener.listen(8888, true);

    vector<struct pollfd> events(MAX_EVENTS);
    events[0].fd = listener.listen_fd();
    events[0].events = POLLRDNORM;

    // init empty event with fd{-1}
    int i;
    for (i = 1; i < MAX_EVENTS; i++) {
        events[i].fd = -1;
    }

    int n_ready;

    while (true) {
        if ((n_ready = poll(events.data(), MAX_EVENTS, -1)) < 0) {
            cerr << "poll error: " << strerror(errno) << endl;
            exit(1);
        }

        // check listen_fd first
        if (events[0].revents & POLLRDNORM) {
            int conn_fd = listener.accept().conn_fd();

            for (i = 1; i < MAX_EVENTS; i++) {
                if (events[i].fd < 0) {
                    events[i].fd = conn_fd;
                    events[i].events = POLLRDNORM;
                    break;
                }
            }

            if (i == MAX_EVENTS) {
                cerr << "can not hold so many clients\n";
            }

            if (--n_ready <= 0) {
                continue;
            }
        }

        for (i = 1; i < MAX_EVENTS; i++) {
            int conn_fd;
            if ((conn_fd = events[i].fd) < 0) {
                continue;
            }
            if (events[i].revents & POLLHUP) {
                cerr << "poll error" << endl;
                close(events[i].fd);
                events[i].fd = -1;
            } else if (events[i].revents & (POLLRDNORM | POLLERR)) {
                TcpConnection conn{conn_fd};
                string msg;
                if (!(msg = conn.receive_line()).empty()) {
                    cout << "server received: " << msg;
                    conn.send(msg);
                } else {
                    conn.close();
                    cout << "server closed conn_fd: " << conn_fd << endl;
                    events[i].fd = -1;
                }
                if (--n_ready <= 0) {
                    break;
                }
            }
        }
    }
}