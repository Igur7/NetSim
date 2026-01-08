#include "simulation.hpp"

void simulate(
    Factory& factory,
    TimeOffset d,
    std::function<void(Factory&, TimeOffset)> rf
){
    if (!factory.is_consistent()) {
        throw std::logic_error("Factory is not consistent");
    }

    // Simulation runs turns starting from 1 to d (inclusive)
    for (Time i = 1; i <= d; ++i) {
        // 1) deliveries happen at the beginning of the turn
        factory.do_deliveries(i);
        // 2) workers process packages that were in their queues before this turn
        factory.do_work(i);
        // 3) after processing, packages in sending buffers are passed to receivers
        factory.do_package_passing();
        // 4) user-provided report function for this turn
        rf(factory, i);
    }
}
