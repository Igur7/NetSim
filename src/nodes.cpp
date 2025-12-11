#include "nodes.hpp"

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