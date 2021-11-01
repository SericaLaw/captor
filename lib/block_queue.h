#ifndef NETWORK_101_BLOCK_QUEUE_H
#define NETWORK_101_BLOCK_QUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>

template<typename E>
class block_queue {
public:
    void push(const E &ele) {
        std::scoped_lock lock{_mutex};
        _q.push(ele);
        _cond.notify_one();
    }

    E pop() {
        std::unique_lock lock{_mutex};
        _cond.wait(lock, [this] { return !_q.empty(); });

        auto ret = _q.front();
        _q.pop();
        lock.unlock();
        return ret;
    }

    std::optional<E> try_pop() {
        std::scoped_lock lock{_mutex};
        if (_q.empty()) {
            return {};
        }

        auto ret = _q.front();
        _q.pop();
        return ret;
    }

    std::optional<std::queue<E>> try_pop_batch() {
        std::scoped_lock lock{_mutex};
        if (_q.empty()) {
            return {};
        }

        std::queue<E> ret;
        while (!_q.empty()) {
            ret.push(_q.front());
            _q.pop();
        }
        return ret;
    }

private:
    std::queue<E> _q;
    std::mutex _mutex;
    std::condition_variable _cond;
};


#endif //NETWORK_101_BLOCK_QUEUE_H
