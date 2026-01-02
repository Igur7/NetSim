
#include "reports.hpp"

void generate_struture_report(const Factory& f, std::ostream& os){
  os << "\n== LOADING RAMPS ==\n\n";
  //id rosnąco


  //pętla po rampie
  for (auto iter = f.ramp_cbegin(); iter != f.ramp_cend(); ++iter) {
    std::set<ElementId> workers;
  	std::set<ElementId> storehouses;

    os <<"LOADING_RAMP id=" <<std::to_string(iter->get_id()) << "\n";
    os <<"Delivery interval: " <<std::to_string(iter->get_delivery_interval()) << "\n";
    os <<"Receivers:\n";
    for (auto iter2 = iter->getReceiverPreferences().cbegin(); iter2 != iter->getReceiverPreferences().cend(); ++iter2) {
      if (iter2->first->get_receiver_type() == ReceiverType::WORKER){
        workers.insert(iter2->first->get_id());
      }
      else if (iter2->first->get_receiver_type() == ReceiverType::STOREHOUSE){
        storehouses.insert(iter2->first->get_id());
      }
    }
    for (auto id:storehouses){
      os<<"storehouse #"<<id<<"\n";

  	}
    for (auto id:workers){
      os<<"worker "<<id<<"\n";
    }

    os <<"\n";

  }
  os <<"\n== WORKERS ==\n\n";

  for(auto iter = f.worker_cbegin(); iter != f.worker_cend(); ++iter) {
     std::set<ElementId> workers;
     std::set<ElementId> storehouses;

     std::string queue_type =
     	iter->get_queue()->getQueueType() == PackageQueueType::Fifo? "Fifo" : "Lifo";

     os <<"WORKER #"<<iter->get_id()<<"\n";
     os <<"Processing time: " << iter->get_processing_duration() << "\n";
     os <<"Queue type:" <<queue_type<<"\n";
     os <<"Receivers:\n";
	 for (auto iter2=iter->getReceiverPreferences().cbegin(); iter2!=iter->getReceiverPreferences().cend(); ++iter2) {
           if (iter2->first->get_receiver_type() == ReceiverType::WORKER){
             workers.insert(iter2->first->get_id());
           }
           else if (iter2->first->get_receiver_type() == ReceiverType::STOREHOUSE){
             storehouses.insert(iter2->first->get_id());
           }
	 }
         for (auto id:storehouses){
           os<<"storehouse #"<<id<<"\n";
         }
         for (auto id:workers){
           os<<"worker "<<id<<"\n";
         }
         os <<"\n";

  }
  os <<"\n==STOREHOUSES ==\n\n";
  for (auto iter=f.storehouse_cbegin(); iter != f.storehouse_cend(); ++iter) {
    os << "STOREHOUSE #" << iter->get_id() << "\n\n";
  }
}

void generate_simulation_turn_report(const Factory& f, std::ostream& os, Time t) {

    os << "=== [ Turn: " << std::to_string(t) << " ] ===\n\n";
    os << "== WORKERS ==\n";

    std::set<ElementId> worker_ids;
    for (auto it = f.worker_cbegin(); it != f.worker_cend(); ++it) {
        worker_ids.insert(it->get_id());
    }

    for (auto id : worker_ids) {
        auto it = f.find_worker_by_id(id);
        const Worker& worker = *it;

        os << "WORKER #" << worker.get_id() << "\n";


        const std::optional<Package>& pbuf = worker.get_processing_buffer();
        if (pbuf.has_value()) {
            os << "  PBuffer: #" << pbuf->getID()
               << " (pt = " << std::to_string(t - worker.get_package_processing_start_time() + 1)
               << ")\n";
        } else {
            os << "  PBuffer: (empty)\n";
        }


        if (worker.get_queue()->empty()) {
            os << "  Queue: (empty)\n";
        } else {
            os << "  Queue:";
            for (auto i = worker.get_queue()->cbegin(); i != worker.get_queue()->cend(); ++i) {
                os << " #" << i->getID();
                if (std::next(i) != worker.get_queue()->cend()) os << ",";
            }
            os << "\n";
        }

        const std::optional<Package>& sbuf = worker.get_sending_buffer();
        if (sbuf.has_value()) {
            os << "  SBuffer: #" << sbuf->getID() << "\n";
        } else {
            os << "  SBuffer: (empty)\n";
        }

        os << "\n";
    }

    os << "== STOREHOUSES ==\n\n";

    std::set<ElementId> store_ids;
    for (auto it = f.storehouse_cbegin(); it != f.storehouse_cend(); ++it) {
        store_ids.insert(it->get_id());
    }

    for (auto id : store_ids) {
        auto it = f.find_storehouse_by_id(id);  // iterator
        const Storehouse& store = *it;

        os << "STOREHOUSE #" << store.get_id() << "\n";

        if (store.get_queue()->empty()) {
            os << "  Stock: (empty)\n\n";
        } else {
            os << "  Stock:";
            for (auto i = store.get_queue()->cbegin(); i != store.get_queue()->cend(); ++i) {
                os << " #" << i->getID();
                if (std::next(i) != store.get_queue()->cend()) os << ",";
            }
            os << "\n\n";
        }
    }
}
