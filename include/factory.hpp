#ifndef FACTORY_HPP
#define FACTORY_HPP

#include <list>
#include <algorithm>
#include <stdexcept>
#include <vector> 
#include "nodes.hpp"

enum class NodeColor { UNVISITED, VISITED, VERIFIED };


template <typename Node>
class NodeCollection {
public:
    using container_t = std::list<Node>;
    using iterator = typename container_t::iterator;
    using const_iterator = typename container_t::const_iterator;

    void add(Node&& node) {
        container_.emplace_back(std::move(node));
    }

    void remove_by_id(ElementId id) {
        auto it = find_by_id(id);
        if (it != container_.end()) {
            container_.erase(it);
        }
    }

    iterator find_by_id(ElementId id) {
        return std::find_if(container_.begin(), container_.end(),
            [id](const Node& node) {
                return node.get_id() == id;
            });
    }

    const_iterator find_by_id(ElementId id) const {
        return std::find_if(container_.cbegin(), container_.cend(),
            [id](const Node& node) {
                return node.get_id() == id;
            });
    }

    iterator begin() { return container_.begin(); }
    iterator end() { return container_.end(); }
    
    const_iterator cbegin() const { return container_.cbegin(); }
    const_iterator cend() const { return container_.cend(); }
    
    const_iterator begin() const { return cbegin(); }
    const_iterator end() const { return cend(); }

private:
    container_t container_;
};

class Factory {
private:
    template<class Node>
    void remove_receiver(NodeCollection<Node>& collection, ElementId id);
    NodeCollection<Worker> workers_;
    NodeCollection<Storehouse> storehouses_;
    NodeCollection<Ramp> ramps_;
    bool has_reachable_storehouse(const PackageSender *sender, std::map<const PackageSender *, NodeColor> &node_colors) const;
public:
    bool is_consistent() const;
    void do_deliveries(Time t);
    void do_package_passing();
    void do_work(Time t);

    void add_ramp(Ramp&& ramp);
    void remove_ramp(ElementId id);
    NodeCollection<Ramp>::iterator find_ramp_by_id(ElementId id);
    NodeCollection<Ramp>::const_iterator find_ramp_by_id(ElementId id) const;
    NodeCollection<Ramp>::const_iterator ramp_cbegin() const;
    NodeCollection<Ramp>::const_iterator ramp_cend() const;
    
    void add_worker(Worker&& worker);
    void remove_worker(ElementId id);
    NodeCollection<Worker>::iterator find_worker_by_id(ElementId id);
    NodeCollection<Worker>::const_iterator find_worker_by_id(ElementId id) const;
    NodeCollection<Worker>::const_iterator worker_cbegin() const;
    NodeCollection<Worker>::const_iterator worker_cend() const;
    
    void add_storehouse(Storehouse&& storehouse);
    void remove_storehouse(ElementId id);
    NodeCollection<Storehouse>::iterator find_storehouse_by_id(ElementId id);
    NodeCollection<Storehouse>::const_iterator find_storehouse_by_id(ElementId id) const;
    NodeCollection<Storehouse>::const_iterator storehouse_cbegin() const;
    NodeCollection<Storehouse>::const_iterator storehouse_cend() const;
};

enum class ElementType{
    RAMP,
    WORKER,
    STOREHOUSE,
    LINK
};

struct ParsedLineData{
    ElementType element_type;
    std::map<std::string, std::string> parameters;
};

// IO handler for loading and saving factory structure
class IO{
    public:
    Factory load_factory_structure(std::istream& is);
    void save_factory_structure(Factory& factory, std::ostream& os);
    std::vector<std::string> tokenize(std::string str, char delimiter);
    ParsedLineData parse_line(const std::string& line);
    
};

#endif // FACTORY_HPP