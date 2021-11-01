#ifndef NETWORK_101_ACCEPTOR_CHANNEL_H
#define NETWORK_101_ACCEPTOR_CHANNEL_H

#include <functional>

#include "channel.h"
#include "../tcp_listener.h"

class AcceptorChannel : public Channel {
public:
    AcceptorChannel() = delete;
    explicit AcceptorChannel(int port) {
        cout << "listening port: " << port << endl;
        listener.listen(port, true);
        _fd = listener.listen_fd();
        _read_callback = std::bind(&AcceptorChannel::handle_connection_established, this);
    }

private:
    vector<ChannelOp> handle_connection_established() {
        TcpConnection conn = listener.accept();
        conn.set_nonblocking();
        vector<ChannelOp> ret;
        ret.push_back({CHANNEL_ADD, CHANNEL_CONN, conn.conn_fd()});
        cout << "AcceptorChannel handle_connection_established: new conn fd=" << conn.conn_fd() << endl;
        return ret;
    }
    TcpListener listener;
};

#endif //NETWORK_101_ACCEPTOR_CHANNEL_H
