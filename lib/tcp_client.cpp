#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <strings.h>
#include <cstring>
#include <cerrno>
#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

#include "tcp_client.h"

TcpConnection TcpClient::connect(std::string address, int port) {
    int conn_fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, address.c_str(), &server_addr.sin_addr);

    int connect_rt = ::connect(conn_fd, (struct sockaddr *) &server_addr, sizeof(server_addr));
    if (connect_rt < 0) {
        cerr << "TcpClient connect error: " << strerror(errno) << endl;
        exit(1);
    }
    return TcpConnection{conn_fd};
}
