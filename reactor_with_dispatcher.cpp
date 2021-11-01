#include <iostream>
#include <memory>
#include <unordered_map>
#include <vector>
#include <functional>
#include <unistd.h>

using namespace std;

#include "dispatcher.h"
#include "tcp_listener.h"
#include "connection_channel.h"
#include "acceptor_channel.h"


void on_message(TcpConnection& conn) {
    if (!conn.buffer_out.eof()) {
        string msg = conn.buffer_read_line();
        cout << "server received: " << msg;
        conn.buffer_write(msg);
//        conn.blocking_send(msg);

//        sleep(10);
    }
}

int main() {
    unordered_map<int, unique_ptr<Channel>> channel_map;
    unique_ptr<Dispatcher> dispatcher{new EpollDispatcher()};

    unique_ptr<Channel> acceptor{new AcceptorChannel(8888)};

    dispatcher->add(acceptor.get());
    channel_map[acceptor->fd()] = move(acceptor);

    while (true) {
        vector<DispatcherEvent> events{dispatcher->dispatch()};
        for (auto &event : events) {
            if (!channel_map.count(event.fd)) {
                continue;
            }
            Channel *channel = channel_map[event.fd].get();
            vector<ChannelOp> channel_ops{channel->on_event(event.revents)};
            for (auto &op : channel_ops) {
                if (op.op == CHANNEL_ADD) {
                    if (op.channel_type == CHANNEL_CONN) {
                        ConnectionChannel *channel = new ConnectionChannel(op.fd);
                        channel->message_callback = on_message;
                        dispatcher->add(channel);
                        channel_map[op.fd] = unique_ptr<Channel>(channel);
                        cout << "new connection established: fd=" << op.fd << endl;
                    }
                } else if (op.op == CHANNEL_REMOVE) {
                    Channel *channel = channel_map[op.fd].get();
                    dispatcher->remove(channel);
                    channel_map.erase(op.fd);
                    close(op.fd);
                }
            }
        }
    }

    return 0;
}