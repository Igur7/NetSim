#include "nodes.hpp"

Storehouse::Storehouse(ElementId id, std::unique_ptr<IPackageQueue> queue)
    : queue_(std::move(queue)), id_(id) {}

void Storehouse::receive_package(Package&& package) {
    queue_->push(std::move(package));
}

Package Storehouse::releasePackage() {
    return queue_->pop();
}
void ReceiverPreferences::add_receiver(std::shared_ptr<IPackageReceiver> r) {
    double count = preferences_.size();

    if (count == 0) {
        preferences_[r] = 1.0;
        return;
    }
    double new_prob = 1.0 / (count + 1);
    // referencja do pary w mapie pair.first - klucz, pair.second - wartość prawdopodobieństwa
    for (auto& pair : preferences_) {
        pair.second = new_prob;
    }
    //dodanie nowego odbiorcy z prawdopodobieństwem
    preferences_[r] = new_prob;

}

void ReceiverPreferences::remove_receiver(std::shared_ptr<IPackageReceiver> r) {
    auto it = preferences_.find(r);
    if (it == preferences_.end()) return;

    preferences_.erase(it);
    
    if (preferences_.empty()) return;

    double new_prob = 1.0 / preferences_.size();
    for (auto& pair : preferences_) {
        pair.second = new_prob;
    }
}

std::shared_ptr<IPackageReceiver> ReceiverPreferences::choose_receiver() {
    double p = probability_generator_();

    double cumulative_probability = 0.0;
    for (const auto& pair : preferences_) {
        cumulative_probability += pair.second;
        if (p <= cumulative_probability) {
            return pair.first;
        }
    }
    return nullptr;
}

void PackageSender::send_package() {
    if (!sending_buffer_.has_value()) {
        return; // Brak paczki do wysłania
    }

    auto receiver = receiver_preferences_.choose_receiver();
    if (receiver) {
        receiver->receive_package(std::move(sending_buffer_.value()));
        sending_buffer_.reset(); // Wyczyść bufor po wysłaniu paczki
    }
}

void Worker::do_work(Time t){// w symulacji update czasu t
    if (!_buffer_.has_value() && !queue_->empty()) {
        _buffer_ = queue_->pop();
        time_ = t;
    }

    if (_buffer_.has_value()) {
        if (t - time_ >= process_durration_) {
            push_package(std::move(_buffer_.value()));
            _buffer_.reset();
        }
    }
}

void Worker::receive_package(Package&& package) {
    queue_->push(std::move(package)); //zdejmuje paczke z kolejki i umieszcza w kolejce pracownika
}

ReceiverType Worker::get_receiver_type() const {
    return ReceiverType::WORKER;
}   

void Ramp::deliver_goods(Time t) {
    if (!buffer_) {
        buffer_.emplace(id_);   // tworzymy paczkę w buforze rampy, bo tam jest std::optional to jak sie tam da id_ to on odpadli konstruktori paczki i doda paczka do buffer
        time_ = t;              // zapamiętujemy moment rozpoczęcia
        return;
    }

    // 2. Jeśli paczka była produkowana i minął czas dostawy
    if (t - time_ == di_) {
        push_package(std::move(*buffer_)); // przekazujemy do wysyłki
        buffer_.reset();                   // bufor znów pusty
    }
}

TimeOffset Ramp::get_delivery_interval() const {
    return di_;
}

ElementId Ramp::get_id() const {
    return id_;
}

