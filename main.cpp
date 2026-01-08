#include <iostream>
#include <fstream>
#include "simulation.hpp"
#include "reports.hpp"
#include <cstdlib>  


int main() {
    Factory factory;
    IO io;

    std::ifstream input("fa.txt");
    if (!input.is_open()) {
        std::cerr << "Nie mozna otworzyc pliku fa.txt\n";
        return 1;
    }

    factory = io.load_factory_structure(input);

    std::ofstream output_struct("struct-report.txt");
    if (!output_struct.is_open()) {
        std::cerr << "Nie mozna otworzyc pliku struct-report.txt\n";
        return 1;
    }

    std::ofstream output_sim("sim-report.txt");
    if (!output_sim.is_open()) {
        std::cerr << "Nie mozna otworzyc pliku sim-report.txt\n";
        return 1;
    }
    generate_structure_report(factory, output_struct);
    
    std::ofstream dot_file("factory.dot");
    io.save_factory_graphviz(factory, dot_file);
    dot_file.close();


    simulate(factory, 67, [&output_sim](Factory& f, Time t) {
        generate_simulation_turn_report(f, output_sim, t);
        output_sim << "\n";
    });

    int result = std::system("dot -Tpng factory.dot -o factory.png");

    if (result != 0) {
        std::cerr << "Blad podczas generowania PNG (czy Graphviz jest zainstalowany?)\n";
    }

    return 0;
}
