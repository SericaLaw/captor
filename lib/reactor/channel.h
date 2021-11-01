#ifndef NETWORK_101_CHANNEL_H
#define NETWORK_101_CHANNEL_H

#include <functional>
#include <utility>
#include <vector>
#include <iostream>
#include <memory>

#include "definition.h"

struct ChannelOp {
    int op;
    int channel_type;
    int fd;
};

class Channel {
public:
    Channel() = default;
    Channel(int fd,
            std::function<std::vector<ChannelOp>()> read_callback = nullptr,
            std::function<std::vector<ChannelOp>()> write_callback = nullptr)
            : _fd(fd)
            , _read_callback(std::move(read_callback))
            , _write_callback(std::move(write_callback)) {}

    std::function<std::vector<ChannelOp>()> _read_callback{nullptr};
    std::function<std::vector<ChannelOp>()> _write_callback{nullptr};

    bool readable() const { return _read_callback != nullptr; }
    bool writable() const { return _write_callback != nullptr; }

    int fd() const {
        if (_fd == -1) {
            std::cerr << "Channel warning: _fd value is taken with -1\n";
        }
        return _fd;
    }

    std::vector<ChannelOp> on_event(int events) {
        std::vector<ChannelOp> ret1, ret2;
        if ((events & EVENT_READ) && _read_callback != nullptr) {
            ret1 = move(_read_callback());
        }
        if ((events & EVENT_WRITE) && _write_callback != nullptr) {
            ret2 = move(_write_callback());
        }
        std::vector<ChannelOp> new_channels;
        new_channels.reserve(ret1.size() + ret2.size());
        new_channels.insert(new_channels.end(), ret1.begin(), ret1.end());
        new_channels.insert(new_channels.end(), ret2.begin(), ret2.end());
        return new_channels;
    }

protected:
    int _fd{-1};
};


#endif //NETWORK_101_CHANNEL_H
