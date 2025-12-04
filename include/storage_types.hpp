#pragma once 

#include "package.hpp"
#include <iostream>
#include <list>

enum class PackageQueueType {
    Fifo,Lifo
};

class IPackageStockpile {
public:
    using const_iterator = std::list<Package>::const_iterator;

    virtual void push(Package&& other) = 0;
    virtual bool empty() const = 0;
    virtual std::size_t size() const = 0;
    virtual const_iterator begin() const = 0;
    virtual const_iterator end() const = 0;
    virtual const_iterator cbegin() const = 0;
    virtual const_iterator cend() const = 0;
    virtual ~IPackageStockpile() = default;
};

class IPackageQueue : public IPackageStockpile {
public:
    virtual Package pop() = 0;
    virtual PackageQueueType getQueueType() const = 0;
    ~IPackageQueue() override = default;
};

class PackageQueue: public IPackageQueue {
    public:
        PackageQueue(PackageQueueType queue_type);
        Package pop() override;
        PackageQueueType getQueueType() const override;
        void push(Package&& other) override;
        std::size_t size() const override;
        bool empty() const override;
        const_iterator begin() const override;
        const_iterator end() const override;
        const_iterator cbegin() const override;
        const_iterator cend() const override;
        ~PackageQueue() override = default;
        

    private:
        std::list<Package> _list;
        PackageQueueType queue_type_;
};