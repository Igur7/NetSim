#pragma once 

#include "factory.hpp"
#include <functional>
#include <set>

class IntervalReportNotifier{
    public:
        explicit IntervalReportNotifier(TimeOffset to): to_(to) {};
        bool should_generate_report(Time t) {return t == 1 || t % to_ == 1;};
    private:
        TimeOffset to_;
};

class SpecificTurnsReportNotifier{
    public:
        explicit SpecificTurnsReportNotifier(const std::set<Time> & turns) : turns_(turns) {};
        bool should_generate_report(Time t) {return turns_.find(t) != turns_.cend();};
    private:
        std::set<Time> turns_;
};

void simulate(
    Factory& factory,
    TimeOffset d,
    std::function<void(Factory&, TimeOffset)> rf
);
