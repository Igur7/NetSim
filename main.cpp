#include <iostream>
#include <fstream>
#include "simulation.hpp"
#include "reports.hpp"

int main() {
    Factory factory;
    IO io;

    std::ifstream input("C:\\Users\\IGOR\\OneDrive\\Pulpit\\astudia\\rok_II\\ZPO\\NetSim\\fa.txt");
    if (!input.is_open()) {
        std::cerr << "Nie mozna otworzyc pliku fa.txt\n";
        return 1;
    }

    factory = io.load_factory_structure(input);

    std::ofstream output("C:\\Users\\IGOR\\OneDrive\\Pulpit\\astudia\\rok_II\\ZPO\\NetSim\\simulation_report.txt");
    if (!output.is_open()) {
        std::cerr << "Nie mozna otworzyc pliku simulation_report.txt\n";
        return 1;
    }

    generate_structure_report(factory, output);
    simulate(factory, 3, [&output](Factory& f, Time t) {
        generate_simulation_turn_report(f, output, t);
        output << "\n";
    });

    return 0;
}
