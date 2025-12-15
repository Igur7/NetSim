#pragma once 

#include "storage_types.hpp"
#include <memory>

class Storehouse{
    public:
        explicit Storehouse(
        std::unique_ptr<IPackageQueue> queue =
            std::make_unique<PackageQueue>(PackageQueueType::Fifo)
        );

        void ReceivePackage(Package&& package);
        Package ReleasePackage();

    private:
    std::unique_ptr<IPackageQueue> queue_;

};