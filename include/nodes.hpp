#pragma once 

#include "storage_types.hpp"
#include <memory>

class Storehouse{
    public:
        explicit Storehouse(
        std::unique_ptr<IPackageQueue> queue =
            std::make_unique<PackageQueue>(PackageQueueType::Fifo)
        );

        void receivePackage(Package&& package);
        Package releasePackage();

    private:
    std::unique_ptr<IPackageQueue> queue_;

};