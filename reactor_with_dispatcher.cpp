#include <iostream>
#include <memory>
#include <unordered_map>
#include <vector>

#include "dispatcher.h"
#include "tcp_listener.h"

using namespace std;

// TODO: replace with channel op (add, remove, update)
vector<struct Channel> handle_connection_established(int fd) {
    TcpListener listener{fd};
    TcpConnection conn = listener.accept();
    conn.set_nonblocking();

    struct Channel new_channel{conn.conn_fd(), CHANNEL_CONN};
    new_channel.set_readable();
    vector<struct Channel> ret;
    ret.push_back(new_channel);
    cout << "new connection established: fd=" << conn.conn_fd() << endl;
    return ret;
}

vector<struct Channel> handle_connection_read(int fd) {
    TcpConnection conn{fd};
    string msg;
    if (!(msg = conn.receive_line()).empty()) {
        cout << "server received: " << msg;
    }
    return {};
}

vector<struct Channel> handle_connection_write(int fd) {
    TcpConnection conn{fd};
    string msg;
    if (!(msg = conn.receive_line()).empty()) {
        cout << "server received: " << msg;
    }
    return {};
}

int main() {
    TcpListener listener;
    listener.listen(8888, true);
    unordered_map<int, Channel> channel_map;
    auto dispatcher = unique_ptr<Dispatcher>(new PollDispatcher());

    Channel acceptor{listener.listen_fd(), CHANNEL_ACCEPT, handle_connection_established, nullptr};
    acceptor.set_readable();
    cout << "listen_fd=" << listener.listen_fd() << endl;

    channel_map[listener.listen_fd()] = acceptor;
    dispatcher->add(acceptor);

    while (true) {
        vector<DispatcherEvent> events = move(dispatcher->dispatch());
        cout << "dispatcher events size: " << events.size() << endl;
        for (auto &event : events) {
            cout << "dispatcher event fd=" << event.fd << ", revents=" << event.revents << endl;
            Channel &channel = channel_map[event.fd];
            cout << "channel fd=" << channel.fd << endl;
            auto new_channels = channel.on_event(event.revents);
            for (auto &new_channel : new_channels) {
                if (new_channel.type == CHANNEL_CONN) {
                    new_channel.read_callback = handle_connection_read;
                }
                channel_map[new_channel.fd] = new_channel;
                dispatcher->add(new_channel);
            }
        }
    }

    return 0;
}