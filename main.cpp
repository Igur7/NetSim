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

    std::ofstream output_struct("C:\\Users\\IGOR\\OneDrive\\Pulpit\\astudia\\rok_II\\ZPO\\NetSim\\struct-report.txt");
    if (!output_struct.is_open()) {
        std::cerr << "Nie mozna otworzyc pliku struct-report.txt\n";
        return 1;
    }

    std::ofstream output_sim("C:\\Users\\IGOR\\OneDrive\\Pulpit\\astudia\\rok_II\\ZPO\\NetSim\\sim-report.txt");
    if (!output_sim.is_open()) {
        std::cerr << "Nie mozna otworzyc pliku sim-report.txt\n";
        return 1;
    }
    generate_structure_report(factory, output_struct);
    simulate(factory, 67, [&output_sim](Factory& f, Time t) {
        generate_simulation_turn_report(f, output_sim, t);
        output_sim << "\n";
    });

    return 0;
}
