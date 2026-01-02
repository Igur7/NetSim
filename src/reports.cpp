
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
