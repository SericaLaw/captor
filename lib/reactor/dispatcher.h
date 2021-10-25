#ifndef NETWORK_101_DISPATCHER_H
#define NETWORK_101_DISPATCHER_H
#include <functional>
#include <memory>

#define EVENT_READ  0x01
#define EVENT_WRITE 0x02

#define CHANNEL_ACCEPT  0x01
#define CHANNEL_CONN    0x02

struct DispatcherEvent {
    int fd{-1};
    int revents{0};
};


struct Channel {
    int fd{-1};
    int type{-1};
    std::function<std::vector<struct Channel>(int)> read_callback{nullptr};
    std::function<std::vector<struct Channel>(int)> write_callback{nullptr};
    int _events{0};

    void set_readable() { _events |= EVENT_READ; }
    bool readable() const { return (_events & EVENT_READ) > 0; }
    void set_writable() { _events |= EVENT_WRITE; }
    bool writable() const { return (_events & EVENT_WRITE) > 0; }

    std::vector<struct Channel> on_event(int events) {
        std::vector<struct Channel> ret1, ret2;
        if ((events & EVENT_READ) && read_callback != nullptr) {
            ret1 = read_callback(fd);
        }
        if ((events & EVENT_WRITE) && write_callback != nullptr) {
            ret2 = write_callback(fd);
        }
        std::vector<struct Channel> new_channels;
        new_channels.reserve(ret1.size() + ret2.size());
        new_channels.insert(new_channels.end(), ret1.begin(), ret1.end());
        new_channels.insert(new_channels.end(), ret2.begin(), ret2.end());
        return std::move(new_channels);
    }
};

class Dispatcher {
public:
    virtual bool add(const struct Channel &channel) = 0;
    virtual bool remove(const struct Channel &channel) = 0;
    virtual bool update(const struct Channel &channel) = 0;
    virtual std::vector<struct DispatcherEvent> dispatch() = 0;

    const int DISPATCHER_MAX_EVENTS{128};
};


class EpollDispatcher : public Dispatcher {
public:
    EpollDispatcher();
    bool add(const Channel &channel) override;
    bool remove(const struct Channel &channel) override;
    bool update(const struct Channel &channel) override;
    std::vector<struct DispatcherEvent> dispatch() override;

private:
    int do_epoll_ctl(int op, const struct Channel &channel) const;

private:
    int _efd{-1};
    std::vector<struct epoll_event> _events;

};


class PollDispatcher : public Dispatcher {
public:
    PollDispatcher();
    bool add(const struct Channel &channel) override;
    bool remove(const struct Channel &channel) override;
    bool update(const struct Channel &channel) override;
    std::vector<struct DispatcherEvent> dispatch() override;

private:
    std::vector<struct pollfd> _events;
};

#endif //NETWORK_101_DISPATCHER_H
