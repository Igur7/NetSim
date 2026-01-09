#pragma once

#include "types.hpp"
#include <set>

class Package {
public:
    Package();                       // nowe ID
    Package(ElementID id);           // używa stare ID
    Package(Package&& other) noexcept;

    Package& operator=(Package&& other) noexcept;

    ElementID get_id() const { return id_; }

    ~Package();

private:
    ElementID id_;

    // pola dzielone przez wszystkie obikety danej klasy
    static std::set<ElementID> freed_ids_;  
    static std::set<ElementID> assigned_ids_;

    static ElementID generate_id();
};
