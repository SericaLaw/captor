#include "buffer.h"

using namespace std;

void Buffer::remove_prefix(const size_t n) {
    if (n > str().size()) {
        throw out_of_range("Buffer::remove_prefix");
    }
    _starting_offset += n;
    if (_storage and _starting_offset == _storage->size()) {
        _storage.reset();
    }
}

void BufferList::remove_prefix(const size_t n) {
    if (n > str().size()) {
        _storage.reset();
        return;
    }
    _starting_offset += n;
    if (_storage and _starting_offset + _ending_offset >= _storage->size()) {
        _storage.reset();
    }
}

void BufferList::remove_suffix(const size_t n) {
    if (n > str().size()) {
        _storage.reset();
        return;
    }
    _ending_offset += n;
    if (_storage and _starting_offset + _ending_offset >= _storage->size()) {
        _storage.reset();
    }
}