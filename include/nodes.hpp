#pragma once 

#include "storage_types.hpp"
#include <memory>

class Storehouse{
    public:
        explicit Storehouse(
            std::unique_ptr<IPackageStockpile> stockpile =
             std::make_unique<PackageQueue>()
            );
        void ReceivePackage(Package&& package);
        Package ReleasePackage();
        bool empty() const;

    private:
    std::unique_ptr<IPackageStockpile> stockpile_;

};