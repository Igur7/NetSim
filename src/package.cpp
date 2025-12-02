#include "package.hpp"

Package& Package::operator=(Package&& other) noexcept {
    if (this != &other) {
        assigned_ids_.erase(this->id_);
        freed_ids_.insert(this->id_);
        id_ = other.id_;


    }
    return *this;
}


Package::~Package() {
    freed_ids_.insert(id_);
    assigned_ids_.erase(id_);
}