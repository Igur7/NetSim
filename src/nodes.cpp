#include "nodes.hpp"

Storehouse::Storehouse(std::unique_ptr<IPackageQueue> queue)
    : queue_(std::move(queue)) {}

void Storehouse::ReceivePackage(Package&& package) {
    queue_->push(std::move(package));
}

Package Storehouse::ReleasePackage() {
    return queue_->pop();
}