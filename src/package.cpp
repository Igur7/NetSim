#include "package.hpp"

std::set<ElementId> Package::freed_ids_;
std::set<ElementId> Package::assigned_ids_;

ElementId Package::generate_id() {
    if(!freed_ids_.empty()) {
        ElementId id = *freed_ids_.begin();
        freed_ids_.erase(freed_ids_.begin());
        assigned_ids_.insert(id);
        return id;
    }
    ElementId new_id = assigned_ids_.empty() ? 1 : (*assigned_ids_.rbegin()) + 1;
    assigned_ids_.insert(new_id);
    return new_id;
}

Package::Package() : id_(generate_id()) {}

Package::Package(ElementId id) : id_(id) {
    assigned_ids_.insert(id_);
}

// jak a = std::move(b)  to to niżej to nadpisane tej operacji zeby move działał efektywniej cnie
Package::Package(Package&& other) noexcept: id_(other.id_) {
    other.id_ = 0; // na ID juz nie ma znaczenia
}

Package& Package::operator=(Package&& other) noexcept {
    if (this != &other) {

        // zwolnienie starego id 
        if (id_ != 0) {
            assigned_ids_.erase(id_);
            freed_ids_.insert(id_);
        }

        // move id
        id_ = other.id_;
        other.id_ = 0;

        if (id_ != 0) {
            assigned_ids_.insert(id_);
        }
    }
    return *this;
}

Package::~Package() {
    if (id_ != 0) {
        assigned_ids_.erase(id_);
        freed_ids_.insert(id_);
    }
}
