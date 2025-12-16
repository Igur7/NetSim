#include "nodes.hpp"

Storehouse::Storehouse(std::unique_ptr<IPackageQueue> queue)
    : queue_(std::move(queue)) {}

void Storehouse::receivePackage(Package&& package) {
    queue_->push(std::move(package));
}

Package Storehouse::releasePackage() {
    return queue_->pop();
}