#pragma once
#include "types.hpp"
#include "package.hpp" 
#include <map>
#include "helpers.hpp"
#include <memory>
#include <optional> // for std::optional (C++17)
#include "storage_types.hpp"

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
        Worker(ElementId id, TimeOffset pd, std::unique_ptr<IPackageQueue> queue) :
            id_(id), process_durration_(pd), time_(0), queue_(std::move(queue)) {}
        
        //wywołana w symulacji w fazie przetowrzenia
        void do_work(Time t); 

        TimeOffset get_processing_duration() const { return process_durration_; }
        
        Time get_package_processing_start_time() const { return time_; }
        
        void receive_package(Package&& package) override; 
        
        ElementId get_id() const override { return id_; }
        
        const std::optional<Package>& get_processing_buffer() const { return _buffer_; } //zwraca referencje do optionala z przetwarzanym pakietem
        
        std::unique_ptr<IPackageQueue>& get_queue() { return queue_; } // zwaraca referencje do uniqueptr na kolejke

    private:
        ElementId id_;
        TimeOffset process_durration_; //czas przetwarzania paczki przez danego workera
        Time time_; //czas rozpoczecia przetwarzania paczki
        std::unique_ptr<IPackageQueue> queue_;
        std::optional<Package> _buffer_ = std::nullopt;
};