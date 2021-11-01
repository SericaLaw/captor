#ifndef NETWORK_101_TCP_LISTENER_H
#define NETWORK_101_TCP_LISTENER_H

#include "tcp_connection.h"

class TcpListener {
public:
    explicit TcpListener(int listen_fd = -1) : _listen_fd(listen_fd){};

    void listen(int port, bool non_blocking = false);
    TcpConnection accept() const;
    void shutdown();

    int listen_fd() const;

    ~TcpListener() = default;

private:
    int _listen_fd{-1};
};


#endif //NETWORK_101_TCP_LISTENER_H
