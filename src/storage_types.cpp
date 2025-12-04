#include "storage_types.hpp"
#include <stdexcept>

PackageQueue::PackageQueue(PackageQueueType queue_type) : queue_type_(queue_type), _list() {}

Package PackageQueue::pop(){
    if (_list.empty()) {
        throw std::out_of_range("Attempted to pop from an empty PackageQueue");
    }
    Package pkg;
    if (queue_type_ == PackageQueueType::Fifo) {
        pkg = std::move(_list.front());
        _list.pop_front();
    } else {
        pkg = std::move(_list.back());
        _list.pop_back();
    }
    return pkg;
}

PackageQueueType PackageQueue::getQueueType() const {
    return queue_type_;
}

void PackageQueue::push(Package&& other) {
    _list.push_back(std::move(other));
}

std::size_t PackageQueue::size() const {
    return _list.size();
}

bool PackageQueue::empty() const {
    return _list.empty();
}

PackageQueue::const_iterator PackageQueue::begin() const {
    return _list.begin();
}

PackageQueue::const_iterator PackageQueue::end() const {
    return _list.end();
}

PackageQueue::const_iterator PackageQueue::cbegin() const {
    return _list.cbegin();
}

PackageQueue::const_iterator PackageQueue::cend() const {
    return _list.cend();
}