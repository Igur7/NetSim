#include "factory.hpp"
#include "types.hpp"
#include <memory>

bool Factory::has_reachable_storehouse(const PackageSender* sender, std::map<const PackageSender*, NodeColor>& node_colors) const {
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

    // rampa nie podaje do innej ramppy nigdy tyl;ko do workera lub magazynu dlatego tego onei sprowadziałem ale to jest do review
    node_colors[sender] = NodeColor::VERIFIED;
    if (has_any_receiver) {
        return true;
    } else {
        throw std::logic_error("No reachable storehouse from sender.");
    }


}

bool Factory::is_consistent() const {
    std::map<const PackageSender *, NodeColor> color;
    for (const auto& ramp : ramps_) {
        color[&ramp] = NodeColor::UNVISITED;
    }
    for (const auto& worker : workers_) {
        color[&worker] = NodeColor::UNVISITED;
    }
    try {
        for (const auto& ramp : ramps_) {
            has_reachable_storehouse(&ramp, color);
        }
    } 
    catch (const std::logic_error& e) {

        return false;
    }

    return true;
}

template <class Node>
void Factory::remove_receiver(NodeCollection<Node>& collection, ElementId id) {
    collection.remove_by_id(id);
}

void Factory::do_deliveries(Time t){
    for(auto& ramp : ramps_){
        ramp.deliver_goods(t);
    }
}

void Factory::do_package_passing(){
    for(auto& ramp : ramps_){
        ramp.send_package();
    }
    for(auto& worker : workers_){
        worker.send_package();
    }
}

void Factory::do_work(Time t){
    for(auto& worker : workers_){
        worker.do_work(t);
    }
}

void Factory::add_ramp(Ramp&& ramp){
    ramps_.add(std::move(ramp));
}

void Factory::remove_ramp(ElementId id){
    remove_receiver(ramps_, id);
}

NodeCollection<Ramp>::iterator Factory::find_ramp_by_id(ElementId id){
    return ramps_.find_by_id(id);
}

NodeCollection<Ramp>::const_iterator Factory::find_ramp_by_id(ElementId id) const{
    return ramps_.find_by_id(id);
}

NodeCollection<Ramp>::const_iterator Factory::ramp_cbegin() const{
    return ramps_.cbegin();
}

NodeCollection<Ramp>::const_iterator Factory::ramp_cend() const{
    return ramps_.cend();
}

void Factory::add_worker(Worker&& worker){
    workers_.add(std::move(worker));
}

void Factory::remove_worker(ElementId id){
    remove_receiver(workers_, id);
}

NodeCollection<Worker>::iterator Factory::find_worker_by_id(ElementId id){
    return workers_.find_by_id(id);
}

NodeCollection<Worker>::const_iterator Factory::find_worker_by_id(ElementId id) const{
    return workers_.find_by_id(id);
}

NodeCollection<Worker>::const_iterator Factory::worker_cbegin() const{
    return workers_.cbegin();
}

NodeCollection<Worker>::const_iterator Factory::worker_cend() const{
    return workers_.cend();
}

void Factory::add_storehouse(Storehouse&& storehouse){
    storehouses_.add(std::move(storehouse));
}

void Factory::remove_storehouse(ElementId id){
    remove_receiver(storehouses_, id);
}

NodeCollection<Storehouse>::iterator Factory::find_storehouse_by_id(ElementId id){
    return storehouses_.find_by_id(id);
}

NodeCollection<Storehouse>::const_iterator Factory::find_storehouse_by_id(ElementId id) const{
    return storehouses_.find_by_id(id);
}

NodeCollection<Storehouse>::const_iterator Factory::storehouse_cbegin() const{
    return storehouses_.cbegin();
}

NodeCollection<Storehouse>::const_iterator Factory::storehouse_cend() const{
    return storehouses_.cend();
}
