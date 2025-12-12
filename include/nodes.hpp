#pragma once
#include "types.hpp"
#include "package.hpp" 
#include <map>
#include "helpers.hpp"
#include <memory>
#include <optional> // for std::optional (C++17)

class IPackageReceiver {
    public:
        virtual void receive_package(Package&& package) = 0;
        virtual ElementId get_id() const = 0;
        virtual ~IPackageReceiver() = default;
};

class ReceiverPreferences {
    public:
        using preferences_t = std::map<std::shared_ptr<IPackageReceiver>, double>;
        ReceiverPreferences(ProbabilityGenerator pg = ProbabilityGenerator()) : probability_generator_(std::move(pg)) {};
        void add_receiver(std::shared_ptr<IPackageReceiver> r);
        void remove_receiver(std::shared_ptr<IPackageReceiver> r);
        std::shared_ptr<IPackageReceiver> choose_receiver();
        const preferences_t& get_preferences() const {return preferences_; }

    private:
        preferences_t preferences_;
        ProbabilityGenerator probability_generator_;
};

class PackageSender {
    public:
        PackageSender() = default;
        PackageSender(PackageSender&&) = default;
        void send_package();
        const std::optional<Package>& get_sending_buffer() const { return sending_buffer_; }
        void push_package(Package&& package) { sending_buffer_ = std::move(package); }

    protected:
        ReceiverPreferences receiver_preferences_;
        std::optional<Package> sending_buffer_ = std::nullopt;
};

class Worker : public IPackageReceiver, public PackageSender {
    public:
        
    private:

};