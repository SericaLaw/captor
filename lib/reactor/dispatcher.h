#ifndef NETWORK_101_DISPATCHER_H
#define NETWORK_101_DISPATCHER_H

#include "definition.h"
#include "channel.h"

struct DispatcherEvent {
    int fd{-1};
    int revents{0};
};

class Dispatcher {
public:
    virtual bool add(const Channel *channel) = 0;
    virtual bool remove(const Channel *channel) = 0;
    virtual bool update(const Channel *channel) = 0;
    virtual std::vector<struct DispatcherEvent> dispatch() = 0;

    const int DISPATCHER_MAX_EVENTS{128};
};


class EpollDispatcher : public Dispatcher {
public:
    EpollDispatcher();
    bool add(const Channel *channel) override;
    bool remove(const Channel *channel) override;
    bool update(const Channel *channel) override;
    std::vector<struct DispatcherEvent> dispatch() override;

private:
    int do_epoll_ctl(int op, const Channel *channel) const;

private:
    int _efd{-1};
    std::vector<struct epoll_event> _events;

};


class PollDispatcher : public Dispatcher {
public:
    PollDispatcher();
    bool add(const Channel *channel) override;
    bool remove(const Channel *channel) override;
    bool update(const Channel *channel) override;
    std::vector<struct DispatcherEvent> dispatch() override;

private:
    std::vector<struct pollfd> _events;
};

#endif //NETWORK_101_DISPATCHER_H
