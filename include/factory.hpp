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
    NodeCollection<Worker> workers_;
    NodeCollection<Storehouse> storehouses_;
    NodeCollection<Ramp> ramps_;
    bool has_reachable_storehouse(const PackageSender *sender, std::map<const PackageSender *, NodeColor> &node_colors);

public:
    bool is_consistent() const;
};