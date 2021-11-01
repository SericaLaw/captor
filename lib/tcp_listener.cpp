#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <csignal>
#include <cerrno>
#include <iostream>
#include <fcntl.h>

using namespace std;

#include "tcp_listener.h"

void TcpListener::listen(int port, bool non_blocking) {
    if (_listen_fd != -1) {
        cerr << "TcpListener already listening with listen_fd = " << _listen_fd << endl;
        return;
    }

    _listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (non_blocking) {
        fcntl(_listen_fd, F_SETFL, O_NONBLOCK);
    }

    struct sockaddr_in server_addr{};
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    int on = 1;
    setsockopt(_listen_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    int err = bind(_listen_fd, (struct sockaddr *) &server_addr, sizeof(server_addr));
    if (err < 0) {
        cerr << "TcpServer bind error: " << strerror(errno) << endl;
        exit(1);
    }

    err = ::listen(_listen_fd, 1024);
    if (err < 0) {
        cerr << "TcpServer listen error: " << strerror(errno) << endl;
        exit(1);
    }

    signal(SIGPIPE, SIG_IGN);
}

TcpConnection TcpListener::accept() const {
    if (_listen_fd == -1) {
        cerr << "TcpListener accept error: not listening\n";
        return TcpConnection{-1};
    }
    struct sockaddr_storage ss{};
    socklen_t len = sizeof(ss);
    int conn_fd = ::accept(_listen_fd, (struct sockaddr *) &ss, &len);
    if (conn_fd < 0) {
        cerr << "TcpServer accept error: " << strerror(errno) << endl;
        exit(1);
    }
    return TcpConnection{conn_fd};
}

void TcpListener::shutdown() {
    ::close(_listen_fd);
    _listen_fd = -1;
}

int TcpListener::listen_fd() const {
    return _listen_fd;
}
