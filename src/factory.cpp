#include "factory.hpp"
#include "types.hpp"
#include <memory>

bool Factory::has_reachable_storehouse(const PackageSender* sender, std::map<const PackageSender*, NodeColor>& node_colors) {
    if (node_colors[sender] == NodeColor::VERIFIED) {
        return true;
    }

    node_colors[sender] = NodeColor::VISITED;

    const ReceiverPreferences::preferences_t& preferences = sender->receiver_preferences_.get_preferences();
    
    if(preferences.empty()) {
        throw std::logic_error("Sender has no receivers.");
    }

    bool has_any_receiver = false;

    for (const ReceiverPreferences::preferences_t::value_type& pair : preferences) {

        std::shared_ptr<IPackageReceiver> receiver = pair.first;
        if (receiver->get_receiver_type() == ReceiverType::STOREHOUSE) {
            has_any_receiver = true;
        }

        else if (receiver->get_receiver_type() == ReceiverType::WORKER) {
            IPackageReceiver* receiver_ptr = receiver.get();
            auto worker_ptr = dynamic_cast<Worker *>(receiver_ptr);
            auto sendercv_ptr = dynamic_cast<PackageSender*>(worker_ptr);
            
            if (worker_ptr == sendercv_ptr){
                continue; // unikanie cyklu
            }

            has_any_receiver = true;

            if (node_colors[sendercv_ptr] == NodeColor::UNVISITED) {
                has_reachable_storehouse(sendercv_ptr, node_colors);
            }
        }

    }

    node_colors[sender] = NodeColor::VERIFIED;
    if (has_any_receiver) {
        return true;
    } else {
        throw std::logic_error("No reachable storehouse from sender.");
    }


}
