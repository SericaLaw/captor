#include <iostream>
#include <cstring>
#include <netinet/in.h>

using namespace std;

#include "tcp_client.h"


int main() {
    TcpClient client;
    TcpConnection conn = client.connect("127.0.0.1", 8888);
    conn.blocking_send("hello 1");
    conn.blocking_send(" hello 2\n");
    cout << "client received: " << conn.blocking_receive_line();

    struct {
        u_int32_t f1_length;
        u_int32_t f2_length;
        char data[128] = "client field 1client field 2";
    } message;
    message.f1_length = htonl(14);
    message.f2_length = htonl(14);
    conn.blocking_send_n((char *) &message,
                         sizeof(message.f1_length) + sizeof(message.f2_length) + strlen(message.data));

    conn.close();
}

