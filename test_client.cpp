//
// Created by serica on 10/23/21.
//

#include <iostream>
#include <string>
#include <unistd.h>
#include <cstring>
#include <netinet/in.h>

using namespace std;

#include "lib/tcp_client.h"

int main() {
    TcpClient client;
    TcpConnection conn = client.connect("127.0.0.1", 8888);
    conn.send("hello 1");
    conn.send(" hello 2\n");
    cout << "client received: " << conn.receive_line();

    struct {
        u_int32_t f1_length;
        u_int32_t f2_length;
        char data[128] = "client field 1client field 2";
    } message;
    message.f1_length = htonl(14);
    message.f2_length = htonl(14);
    conn.send_n((char *) &message, sizeof(message.f1_length) + sizeof(message.f2_length) + strlen(message.data));

    conn.close();
}

