#pragma once 

#include "types.hpp"
#include <set>

class Package {
    public:
        Package();

        Package(ElementId id) : id_(id) {assigned_ids_.insert(id);}

        Package(Package&& other) : id_(other.id_) {}

        ElementId getID() const { return id_; }

        Package& operator=(Package&& other) noexcept; //nadpisanie move

        ~Package();

        
    private:
        ElementId id_;
        std::set<ElementId> freed_ids_;
        std::set<ElementId> assigned_ids_;
};