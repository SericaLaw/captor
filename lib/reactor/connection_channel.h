#ifndef NETWORK_101_CONNECTION_CHANNEL_H
#define NETWORK_101_CONNECTION_CHANNEL_H

#include <functional>

#include "channel.h"

class ConnectionChannel : public Channel {
public:
    ConnectionChannel() = delete;
    explicit ConnectionChannel(int fd) {
        cout << "new connection with fd: " << fd << endl;
        conn.set_fd(fd);
        _fd = fd;
        _read_callback = bind(&ConnectionChannel::handle_connection_read, this);
        _write_callback = bind(&ConnectionChannel::handle_connection_write, this);
    }

    std::function<void(TcpConnection&)> message_callback{nullptr};
    std::function<void(TcpConnection&)> write_completed_callback{nullptr};
    std::function<void(TcpConnection&)> connection_closed_callback{nullptr};

private:
    vector<ChannelOp> handle_connection_read() {
        cout << "ConnectionChannel: handle_connection_read\n";
        if (conn.buffer_receive() > 0) {
            if (message_callback != nullptr) {
                message_callback(conn);
            }
        } else {
            conn.buffer_out.end_input();
            return handle_connection_closed();
        }
        return {};
    }

    vector<ChannelOp> handle_connection_write() {
        cout << "ConnectionChannel: handle_connection_write\n";
        conn.buffer_send();
        if (conn.buffer_in.eof()) {
            if (write_completed_callback != nullptr) {
                write_completed_callback(conn);
            }
        }
        return {};
    }

    vector<ChannelOp> handle_connection_closed() {
        cout << "ConnectionChannel: connection closed\n";
        vector<ChannelOp> ret;
        if (connection_closed_callback != nullptr) {
            connection_closed_callback(conn);
        }
        ret.push_back({CHANNEL_REMOVE, CHANNEL_CONN, _fd});
        return ret;
    }
    TcpConnection conn{-1};
};

#endif //NETWORK_101_CONNECTION_CHANNEL_H
