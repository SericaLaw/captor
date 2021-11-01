#ifndef NETWORK_101_BUFFER_H
#define NETWORK_101_BUFFER_H


#include <algorithm>
#include <deque>
#include <memory>
#include <numeric>
#include <string>
#include <string_view>
#include <sys/uio.h>
#include <vector>

//! \brief A reference-counted read-only string that can discard bytes from the front
class Buffer {
private:
    std::shared_ptr<std::string> _storage{};
    size_t _starting_offset{};

public:
    Buffer() = default;

    //! \brief Construct by taking ownership of a string
    Buffer(std::string &&str) noexcept : _storage(std::make_shared<std::string>(std::move(str))) {}

    //! \name Expose contents as a std::string_view
    //!@{
    std::string_view str() const {
        if (not _storage) {
            return {};
        }
        return {_storage->data() + _starting_offset, _storage->size() - _starting_offset};
    }

    operator std::string_view() const { return str(); }
    //!@}

    //! \brief Get character at location `n`
    uint8_t at(const size_t n) const { return str().at(n); }

    //! \brief Size of the string
    size_t size() const { return str().size(); }

    //! \brief Make a copy to a new std::string
    std::string copy() const { return std::string(str()); }

    //! \brief Discard the first `n` bytes of the string (does not require a copy or move)
    //! \note Doesn't free any memory until the whole string has been discarded in all copies of the Buffer.
    void remove_prefix(const size_t n);

    friend class BufferList;
};

class BufferList {
private:
    std::shared_ptr<std::string> _storage{};
    size_t _starting_offset{};
    size_t _ending_offset{};

public:
    BufferList() = default;

    //! \brief Construct by taking ownership of a string
    BufferList(std::string &&str) noexcept : _storage(std::make_shared<std::string>(std::move(str))) {}
    BufferList(const BufferList &AnotherBuffer)
            : _storage(AnotherBuffer._storage)
            , _starting_offset(AnotherBuffer._starting_offset)
            , _ending_offset(AnotherBuffer._ending_offset) {}
    BufferList(const Buffer &bf)
            : _storage(bf._storage)
            , _starting_offset(bf._starting_offset){}
    //! \name Expose contents as a std::string_view
    //!@{
    std::string_view str() const {
        if (not _storage) {
            return {};
        }
        return {_storage->data() + _starting_offset, _storage->size() - _starting_offset - _ending_offset};
    }

    operator std::string_view() const { return str(); }
    //!@}

    //! \brief Get character at location `n`
    uint8_t at(const size_t n) const { return str().at(n); }

    //! \brief Size of the string
    size_t size() const { return _storage ? _storage->size() - _starting_offset - _ending_offset : 0; }
    size_t statring_offset() const { return _starting_offset; }
    size_t ending_offset() const { return _ending_offset; }

    //! \brief Make a copy to a new std::string
    std::string copy() const { return std::string(str()); }

    //! \brief Discard the first `n` bytes of the string (does not require a copy or move)
    //! \note Doesn't free any memory until the whole string has been discarded in all copies of the Buffer.
    void remove_prefix(const size_t n);
    void remove_suffix(const size_t n);
};

#endif //NETWORK_101_BUFFER_H
