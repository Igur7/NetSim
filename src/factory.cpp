#include "factory.hpp"
#include "types.hpp"
#include <memory>
#include <map>
#include <sstream>
#include <stdexcept>
#include <istream>

bool Factory::has_reachable_storehouse(
    const PackageSender* sender,
    std::map<const PackageSender*, NodeColor>& node_colors) const
{
    // Jeśli już zweryfikowany – wiemy, że OK
    if (node_colors[sender] == NodeColor::VERIFIED) {
        return true;
    }

    // Jeśli wracamy do VISITED → cykl bez magazynu
    if (node_colors[sender] == NodeColor::VISITED) {
        return false;
    }

    node_colors[sender] = NodeColor::VISITED;

    const auto& preferences = sender->get_receiver_preferences();
    if (preferences.empty()) {
        throw std::logic_error("Sender has no receivers");
    }

    bool reachable_storehouse = false;

    for (const auto& [receiver, _] : preferences) {

        if (receiver->get_receiver_type() == ReceiverType::STOREHOUSE) {
            reachable_storehouse = true;
        }
        else if (receiver->get_receiver_type() == ReceiverType::WORKER) {

            auto worker_ptr = dynamic_cast<Worker*>(receiver.get());
            auto sender_ptr = static_cast<PackageSender*>(worker_ptr);

            // pomijamy połączenie do samego siebie
            if (sender_ptr == sender)
                continue;

            // DFS tylko jeśli nie był jeszcze odwiedzony
            if (node_colors[sender_ptr] == NodeColor::UNVISITED) {
                if (has_reachable_storehouse(sender_ptr, node_colors)) {
                    reachable_storehouse = true;
                }
            }
        }
    }

    node_colors[sender] = NodeColor::VERIFIED;

    if (!reachable_storehouse) {
        throw std::logic_error("No reachable storehouse from sender");
    }

    return true;
}


bool Factory::is_consistent() const {
    std::map<const PackageSender*, NodeColor> colors;

    for (const auto& ramp : ramps_) {
        colors[&ramp] = NodeColor::UNVISITED;
    }
    for (const auto& worker : workers_) {
        colors[&worker] = NodeColor::UNVISITED;
    }

    try {
        for (const auto& ramp : ramps_) {
            has_reachable_storehouse(&ramp, colors);
        }
    }
    catch (const std::logic_error&) {
        return false;
    }

    return true;
}

template <class Node>
void Factory::remove_receiver(NodeCollection<Node>& collection, ElementId id) {

    // 1. Usuwamy połączenia z ramp
    for (auto& ramp : ramps_) {
        auto& prefs = ramp.get_receiver_preferences();

        for (auto it = prefs.begin(); it != prefs.end(); ) {
            if (it->first->get_id() == id) {
                ramp.remove_receiver(it->first);
                it = prefs.begin(); // mapa się zmienia ,zaczynamy od nowa
            } else {
                ++it;
            }
        }
    }

    // 2. Usuwamy połączenia z workerów
    for (auto& worker : workers_) {
        auto& prefs = worker.get_receiver_preferences();

        for (auto it = prefs.begin(); it != prefs.end(); ) {
            if (it->first->get_id() == id) {
                worker.remove_receiver(it->first);
                it = prefs.begin();
            } else {
                ++it;
            }
        }
    }

    // 3. Na końcu usuwamy obiekt z kolekcji
    collection.remove_by_id(id);
    //*it  // std::pair<const std::shared_ptr<IPackageReceiver>, double>
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

std::vector<std::string> IO::tokenize(std::string str, char delimiter){
    //przykąłdowe wejscie "WORKER id=1 processing-time=2"
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);

    while (std::getline(tokenStream, token, delimiter)) {
        if (!token.empty()){
            tokens.push_back(token);
        }
    }

    return tokens;
    // wyjscie: { "WORKER", "id=1", "processing-time=2" }
}

ParsedLineData IO::parse_line(const std::string& line){
    ParsedLineData parsed_data;
    //rozwalnie na tokney z odzielaczem spacja
    std::vector<std::string> tokens = tokenize(line,' ');

    if (tokens.empty()){
        throw std::runtime_error("Empty line");
    }

    std::string typeStr = tokens[0];
    if(typeStr == "LOADING_RAMP"){
        parsed_data.element_type = ElementType::RAMP;
    }
    else if(typeStr == "WORKER"){
        parsed_data.element_type = ElementType::WORKER;
    }
    else if(typeStr == "STOREHOUSE"){
        parsed_data.element_type = ElementType::STOREHOUSE;
    }
    else if(typeStr == "LINK"){
        parsed_data.element_type = ElementType::LINK;
    }
    else{
        throw std::runtime_error("Unknown element type: " + typeStr);
    }

    for (size_t i = 1 ; i < tokens.size(); i++){
        const std::string& token = tokens[i];

        std::vector<std::string> key_value = tokenize(token,'=');

        if(key_value.size() != 2){
            throw std::runtime_error("Invalid parameter format: " + token);
        }

        parsed_data.parameters[key_value[0]] = key_value[1];
    }

    return parsed_data;
}

PackageQueueType get_package_queue_type(std::string& packae_queue_type_string){
    std::map<std::string, PackageQueueType> mapping {
        {"FIFO", PackageQueueType::Fifo},
        {"LIFO", PackageQueueType::Lifo}
    };
    return mapping.at(packae_queue_type_string);
}

void link(Factory& factory, const std::map<std::string, std::string>& parameters){
    std::map<std::string, NodeType> node_type_mapping {
        {"ramp", NodeType::RAMP},
        {"worker", NodeType::WORKER},
        {"store", NodeType::STOREHOUSE}
    };

    std::string src_str = parameters.at("src");
    std::string dest_str = parameters.at("dest");

    std::vector<std::string> src_param = IO().tokenize(src_str, '-');
    NodeType src_node_type = node_type_mapping.at(src_param[0]);
    ElementId src_id = std::stoi(src_param[1]);

    std::vector<std::string> dest_param = IO().tokenize(dest_str, '-');
    NodeType dest_node_type = node_type_mapping.at(dest_param[0]);
    ElementId dest_id = std::stoi(dest_param[1]);

    std::shared_ptr<IPackageReceiver> package_receiver;

    // Pozwala to shared_ptr wskazywać na obiekt zarządzany przez Factory
    // bez próby jego usunięcia (delete), gdy shared_ptr wygaśnie.
    auto no_op_deleter = [](IPackageReceiver*){};

    switch(dest_node_type){
        case NodeType::WORKER:
        {
            auto worker_it = factory.find_worker_by_id(dest_id);
            if (worker_it == factory.worker_cend()){
                throw std::runtime_error("Worker with id " + std::to_string(dest_id) + " not found");
            }
            // FIX: Wskazujemy na istniejący obiekt, zamiast tworzyć kopię przez make_shared
            package_receiver = std::shared_ptr<IPackageReceiver>(&(*worker_it), no_op_deleter);
            break;
        }
        case NodeType::STOREHOUSE:
        {
            auto storehouse_it = factory.find_storehouse_by_id(dest_id);
            if (storehouse_it == factory.storehouse_cend()){
                throw std::runtime_error("Storehouse with id " + std::to_string(dest_id) + " not found");
            }
            package_receiver = std::shared_ptr<IPackageReceiver>(&(*storehouse_it), no_op_deleter);
            break;
        }
        case NodeType::RAMP:
        {
            break; 
        }
    }

    // Ustawienie połączenia źródłowego PackageSendera do docelowego IPackageReceivera
    switch(src_node_type){
        case NodeType::RAMP:
        {
            auto ramp_it = factory.find_ramp_by_id(src_id);
            if (ramp_it == factory.ramp_cend()){
                throw std::runtime_error("Ramp with id " + std::to_string(src_id) + " not found");
            }
            ramp_it->add_receiver(package_receiver);
            break;
        }
        case NodeType::WORKER:{
            auto worker_it = factory.find_worker_by_id(src_id);
            if (worker_it == factory.worker_cend()){
                throw std::runtime_error("Worker with id " + std::to_string(src_id) + " not found");
            }
            worker_it->add_receiver(package_receiver);
            break;
        }
        case NodeType::STOREHOUSE:
        {
            break;
        }
    }
}

Factory IO::load_factory_structure(std::istream& is){
    Factory factory;
    std::string line;
    while (std::getline(is,line)){
        if (line.empty() || line[0] == ';' || line[0] == '#') {
            continue; 
        }
        ParsedLineData parsed_data = parse_line(line);
        
        switch(parsed_data.element_type){
            case ElementType::RAMP:
            {
                ElementId id = std::stoi(parsed_data.parameters.at("id"));
                TimeOffset di = std::stoi(parsed_data.parameters.at("delivery-interval"));
                factory.add_ramp(Ramp(id,di));
                break;
            }
            case ElementType::WORKER:
            {
                ElementId id = std::stoi(parsed_data.parameters.at("id"));
                TimeOffset pt = std::stoi(parsed_data.parameters.at("processing-time"));
                PackageQueueType queue_type = get_package_queue_type(parsed_data.parameters.at("queue-type"));
                factory.add_worker(Worker(id,pt,std::make_unique<PackageQueue>(queue_type)));
                break;
            }
            case ElementType::STOREHOUSE:
            {
                ElementId id = std::stoi(parsed_data.parameters.at("id"));
                factory.add_storehouse(Storehouse(id));
                break;
            }
            case ElementType::LINK:
            {
                link(factory, parsed_data.parameters);
                break;
            }
        }
    } 
    return factory;
}

void link_fill(std::stringstream& link_stream, const PackageSender& package_sender, ElementId package_sender_id, std::string package_sender_type){
    const auto& prefs = package_sender.get_receiver_preferences();

    std::for_each(prefs.begin(), prefs.end(), [&](const auto& key_value){
        std::string dest_type = (key_value.first->get_receiver_type() == ReceiverType::WORKER ? "worker" : "store");
        
        link_stream << "LINK src=" << package_sender_type << '-' << package_sender_id 
                    << " dest=" << dest_type << '-' << key_value.first->get_id() << '\n';
    });
}

void IO::save_factory_structure(Factory& factory, std::ostream& os){
    std::stringstream link_stream;

    //Zapis RAMP (LOADING_RAMP)
    std::for_each(factory.ramp_cbegin(), factory.ramp_cend(), [&](const Ramp& ramp) {
        os << "LOADING_RAMP id=" << ramp.get_id() << " delivery-interval=" << ramp.get_delivery_interval() << '\n';
        link_fill(link_stream, ramp, ramp.get_id(), "ramp");
    });

    //Zapis WORKER 
    std::for_each(factory.worker_cbegin(), factory.worker_cend(), [&](const Worker& worker) {
        PackageQueueType queue_type = worker.get_queue()->getQueueType();
        std::string queue_type_str = (queue_type == PackageQueueType::Fifo) ? "FIFO" : "LIFO";
        
        os << "WORKER id=" << worker.get_id() 
           << " processing-time=" << worker.get_processing_duration() 
           << " queue-type=" << queue_type_str << '\n';
           
        link_fill(link_stream, worker, worker.get_id(), "worker");
    });

    //Zapis STOREHOUSE
    std::for_each(factory.storehouse_cbegin(), factory.storehouse_cend(), [&](const Storehouse& storehouse) {
        os << "STOREHOUSE id=" << storehouse.get_id() << '\n';
    });

    //Zapis LINKÓW
    os << link_stream.str();

    os.flush();
}