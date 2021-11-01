#include <iostream>
#include <netinet/in.h>

using namespace std;

#include "tcp_listener.h"


int main() {
    TcpListener listener;
    listener.listen(8888);
    while (true) {
        TcpConnection conn = listener.accept();
        string msg;
        if ((msg = conn.blocking_receive_line()).size() > 0) {
            cout << "server received: " << msg;
            conn.blocking_send(msg);
        }

        // decode message struct
        char buf[128];
        uint32_t f1_length, f2_length;
        int rc;

        rc = conn.blocking_receive_n((char *) &f1_length, sizeof(f1_length));
        if (rc != sizeof(f1_length)) {
            cerr << "receive f1_length error\n";
            continue;
        }
        f1_length = ntohl(f1_length);

        rc = conn.blocking_receive_n((char *) &f2_length, sizeof(f2_length));
        if (rc != sizeof(f2_length)) {
            cerr << "receive f2_length error\n";
            continue;
        }
        f2_length = ntohl(f2_length);

        rc = conn.blocking_receive_n(buf, f1_length);
        if (rc != f1_length) {
            cerr << "receive field1 error\n";
            continue;
        }

        string f1(buf);
        cout << "server received: " << f1 << endl;

        rc = conn.blocking_receive_n(buf, f2_length);
        if (rc != f2_length) {
            cerr << "receive field2 error\n";
            continue;
        }

        string f2(buf);
        cout << "server received: " << f2 << endl;
    }
}