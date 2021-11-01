#include <iostream>
#include <memory>
#include <unordered_map>
#include <vector>
#include <functional>
#include <unistd.h>
#include <cassert>
#include <thread>

using namespace std;

#include "dispatcher.h"
#include "tcp_listener.h"
#include "block_queue.h"
#include "connection_channel.h"
#include "acceptor_channel.h"


void on_message(TcpConnection& conn) {
    if (!conn.buffer_out.eof()) {
        string msg = conn.buffer_read_line();
        cout << "Tcp Server on message: " << msg;

        // handle_write will be called because the write event occurs and will be returned together with the read event
        // the most correct approach is always to try with blocking send first
        // and if the write operation is not available, add it to the dispatcher and let it notify
        // this approach will also improve the performance when using poll dispatcher and epoll dispatcher with level-triggered
        conn.buffer_write(msg);

//        conn.blocking_send(msg);

//        sleep(10);
    }
}

void on_connection_closed(TcpConnection& conn) {
    cout << "Tcp Server on connection closed.\n";
}

void worker(const string &name, block_queue<ChannelOp> &work_queue) {
    unordered_map<int, unique_ptr<Channel>> channel_map;
    unique_ptr<Dispatcher> dispatcher{new EpollDispatcher()};

    while (true) {
        auto conn_op = work_queue.try_pop();
        if (conn_op.has_value()) {
            cout << "hi\n";
            assert(conn_op->op == CHANNEL_ADD);
            assert(conn_op->channel_type == CHANNEL_CONN);
            auto *channel = new ConnectionChannel(conn_op->fd);

            channel->message_callback = on_message;
            channel->connection_closed_callback = on_connection_closed;

            dispatcher->add(channel);
            channel_map[conn_op->fd] = unique_ptr<Channel>(channel);
            cout << "[worker " << name << "] new connection established: fd=" << conn_op->fd << endl;
        }
        vector<DispatcherEvent> events = move(dispatcher->dispatch());
        for (auto &event : events) {
            if (!channel_map.count(event.fd)) {
                continue;
            }
            Channel *channel = channel_map[event.fd].get();
            vector<ChannelOp> channel_ops{channel->on_event(event.revents)};
            cout << "[worker " << name << "] handle event\n";

            for (auto &op : channel_ops) {
                assert(op.op == CHANNEL_REMOVE);
                Channel *channel = channel_map[op.fd].get();
                dispatcher->remove(channel);
                channel_map.erase(op.fd);
                close(op.fd);
                cout << "[worker " << name << "] remove conn fd=" << op.fd << endl;
            }
        }
    }
}

int main() {
    block_queue<ChannelOp> work_queue;
    unique_ptr<Dispatcher> dispatcher{new EpollDispatcher()};

    unique_ptr<Channel> acceptor{new AcceptorChannel(8888)};

    dispatcher->add(acceptor.get());

    thread worker1{worker, "1", ref(work_queue)};
    thread worker2{worker, "2", ref(work_queue)};

    while (true) {
        vector<DispatcherEvent> events = move(dispatcher->dispatch());
        for (auto &event : events) {
            vector<ChannelOp> channel_ops{acceptor->on_event(event.revents)};
            for (auto &op : channel_ops) {
                cout << "push op\n";
                work_queue.push(op);
            }
        }
    }

    return 0;
}