#pragma once
#include <random>

class rng {
private:
    std::mt19937 gen;
public:
    rng() : gen(std::random_device{}()) {}
    unsigned int randint(unsigned int n){
        return gen() % n;
    }
    float randfloat(double a, double b){
        std::uniform_real_distribution<double> dis(a, b);
        return static_cast<float>(dis(gen));
    }
    template<typename A>
	size_t random_weighted_sample(const A& v) {
		std::discrete_distribution<> d(v.begin(), v.end());
		return d(gen);
	}
};

