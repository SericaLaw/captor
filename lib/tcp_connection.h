#ifndef NETWORK_101_TCP_CONNECTION_H
#define NETWORK_101_TCP_CONNECTION_H

#include <string>

#include "byte_stream.h"

class TcpConnection {
public:
    TcpConnection() = delete;
    explicit TcpConnection(int conn_fd) : _conn_fd(conn_fd) {};

    void set_fd(int conn_fd, bool non_blocking = false);
    void set_nonblocking();
    int conn_fd() const;

    int blocking_send(const std::string &msg) const;
    int blocking_send_n(const char *msg, int n) const;
    std::string blocking_receive_line() const;
    int blocking_receive_n(char *msg, const int n) const;

    // to handle read / write in the framework,
    // use buffers as an intermediate between system I/O and user program I/O
    int buffer_send();
    int buffer_receive();
    int buffer_receive_line();
    std::string buffer_read_n(int n);
    std::string buffer_read_line();
    int buffer_write(const std::string &msg);

    void close();
    bool active();

    ~TcpConnection() = default;
    ByteStream buffer_out, buffer_in;

private:
    int _conn_fd{-1};

};


#endif //NETWORK_101_TCP_CONNECTION_H
