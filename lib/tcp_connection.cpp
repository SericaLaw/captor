#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <iostream>

using std::cerr;
using std::endl;
#include "tcp_connection.h"

void TcpConnection::set_nonblocking() {
    fcntl(_conn_fd, F_SETFL, O_NONBLOCK);
}

int TcpConnection::conn_fd() const {
    return _conn_fd;
}

int TcpConnection::send(const std::string &msg) const {
    return send_n(msg.c_str(), msg.size());
}

int TcpConnection::send_n(const char *msg, const int n) const {
    if (_conn_fd == -1) {
        cerr << "TcpConnection send error: not connected\n";
        return -1;
    }

    int remain = n;
    int wc;
    const char *ptr = msg;

    while (remain) {
        if ((wc = ::send(_conn_fd, ptr, remain, 0)) <= 0) {
            if (wc < 0 && errno == EINTR) continue;
            cerr << "TcpConnection send_n error: " << strerror(errno) << endl;
            return -1;
        }
        remain -= wc;
        ptr += wc;
    }
    return n;
}

std::string TcpConnection::receive_line() const {
    if (_conn_fd == -1) {
        cerr << "TcpConnection receive error: not connected\n";
        return "";
    }

    std::string msg;
    char c;
    int rc;

    while (true) {
        if ((rc = ::recv(_conn_fd, &c, 1, 0)) == 1) {
            msg += c;
            if (c == '\n') {
                break;
            }
        } else if (rc == 0) {
            break;
        } else {
            if (errno == EINTR) continue;
            cerr << "TcpConnection receive_line error: " << strerror(errno) << endl;
            break;
        }
    }
    return msg;
}

int TcpConnection::receive_n(char *msg, const int n) const {
    if (_conn_fd == -1) {
        cerr << "TcpConnection receive error: not connected\n";
        return -1;
    }

    int remain = n;
    int rc;
    char *ptr = msg;

    while (remain) {
        if ((rc = ::recv(_conn_fd, ptr, remain, 0)) < 0) {
            if (errno == EINTR) {
                rc = 0;
            } else {
                cerr << "TcpConnection receive_n error: " << strerror(errno) << endl;
                return -1;
            }
        } else if (rc == 0) {
            break;
        }
        remain -= rc;
        ptr += rc;
    }
    return n - remain;
}

void TcpConnection::close() {
    if (active()) {
        ::close(_conn_fd);
        _conn_fd = -1;
    }
}

bool TcpConnection::active() {
    return _conn_fd != -1;
}
