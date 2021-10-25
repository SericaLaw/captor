#ifndef NETWORK_101_TCP_CONNECTION_H
#define NETWORK_101_TCP_CONNECTION_H

#include <string>

class TcpConnection {
public:
    TcpConnection() = delete;
    explicit TcpConnection(int conn_fd) : _conn_fd(conn_fd) {};

    void set_nonblocking();
    int conn_fd() const;

    int send(const std::string &msg) const;
    int send_n(const char *msg, int n) const;
    std::string receive_line() const;
    int receive_n(char *msg, const int n) const;
    void close();
    bool active();

    ~TcpConnection() = default;

private:
    int _conn_fd{-1};
};


#endif //NETWORK_101_TCP_CONNECTION_H
