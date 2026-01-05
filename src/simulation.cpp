#include "simulation.hpp"

void simulate(Factory& factory, TimeOffset d, std::function<void(Factory&, TimeOffset)> && rf){
    if (!factory.is_consistent()) {
        throw std::logic_error();
    }

    for (Time i=0; i<d; i++){
        factory.do_deliveries(i);
        factory.do_package_passing(i);
        factory.do_work(i);
        rf(factory, i);
    }
}