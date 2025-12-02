#pragma once

#include "types.hpp"
#include <set>

class Package {
public:
    Package();                       // nowe ID
    Package(ElementId id);           // używa stare ID
    Package(Package&& other) noexcept;

    Package& operator=(Package&& other) noexcept;

    ElementId getID() const { return id_; }

    ~Package();

private:
    ElementId id_;

    // pola dzielone przez wszystkie obikety danej klasy
    static std::set<ElementId> freed_ids_;  
    static std::set<ElementId> assigned_ids_;

    static ElementId generate_id();
};
