#ifndef NETWORK_101_TCP_CLIENT_H
#define NETWORK_101_TCP_CLIENT_H

#include <string>

#include "tcp_connection.h"

class TcpClient {
public:
    TcpConnection connect(std::string address, int port);
};


#endif //NETWORK_101_TCP_CLIENT_H
