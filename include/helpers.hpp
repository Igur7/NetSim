#pragma once 
#include <random>

class ProbabilityGenerator {
public:
    ProbabilityGenerator() 
        : gen_(std::random_device{}()), 
          dist_(0.0, 1.0)               
    {}

    //moge użyc tego obiektu tej kalsy jako funckji 
    double operator()() {
        return dist_(gen_);
    }

private:
    std::mt19937 gen_; // Standardowy generator Mersenne Twister
    std::uniform_real_distribution<double> dist_;
};