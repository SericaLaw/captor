#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <iostream>
#include <string>

using namespace std;

#include "tcp_connection.h"

void TcpConnection::set_fd(int conn_fd, bool non_blocking) {
    _conn_fd = conn_fd;
    if (non_blocking) {
        set_nonblocking();
    }
}

void TcpConnection::set_nonblocking() {
    fcntl(_conn_fd, F_SETFL, O_NONBLOCK);
}

int TcpConnection::conn_fd() const {
    return _conn_fd;
}

int TcpConnection::blocking_send(const string &msg) const {
    return blocking_send_n(msg.c_str(), msg.size());
}

int TcpConnection::blocking_send_n(const char *msg, const int n) const {
    if (_conn_fd == -1) {
        cerr << "TcpConnection blocking_send error: not connected\n";
        return -1;
    }

    int remain = n;
    int wc;
    const char *ptr = msg;

    while (remain) {
        if ((wc = ::send(_conn_fd, ptr, remain, 0)) <= 0) {
            if (wc < 0 && errno == EINTR) continue;
            cerr << "TcpConnection blocking_send_n error: " << strerror(errno) << endl;
            return -1;
        }
        remain -= wc;
        ptr += wc;
    }
    return n;
}

string TcpConnection::blocking_receive_line() const {
    if (_conn_fd == -1) {
        cerr << "TcpConnection receive error: not connected\n";
        return "";
    }

    string msg;
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
            cerr << "TcpConnection blocking_receive_line error: " << strerror(errno) << endl;
            break;
        }
    }
    return msg;
}

int TcpConnection::blocking_receive_n(char *msg, const int n) const {
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
                cerr << "TcpConnection blocking_receive_n error: " << strerror(errno) << endl;
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
        buffer_in.end_input();
        buffer_out.end_input();
    }
}

bool TcpConnection::active() {
    return _conn_fd != -1;
}

int TcpConnection::buffer_send() {
    if (!buffer_in.buffer_empty()) {
        string msg = buffer_in.peek_output(2048);
        int wc = write(_conn_fd, msg.c_str(), msg.size());
        buffer_in.pop_output(wc);
        return wc;
    }
    return 0;
}

int TcpConnection::buffer_receive() {
    char buf[65536];
    struct iovec vec[1];
    vec[0].iov_base = buf;
    vec[0].iov_len = sizeof(buf);
    int rc = readv(_conn_fd, vec, 1);
    string s{buf};
    buffer_out.write(move(s));
    return rc;
}

string TcpConnection::buffer_read_n(int n) {
    return buffer_out.read(n);
}

string TcpConnection::buffer_read_line() {
    string msg;
    string tmp;

    while (true) {
        if ((tmp = buffer_out.read(1)).size() == 1) {
            msg += tmp;
            if (tmp == "\n") {
                break;
            }
        } else {
            break;
        }
    }
    return msg;
}

int TcpConnection::buffer_write(const string &msg) {
    buffer_in.write(msg);
    return 0;
}

int TcpConnection::buffer_receive_line() {
    string storage = blocking_receive_line();
    int rc = storage.size();
    buffer_out.write(move(storage));
    return rc;
}

