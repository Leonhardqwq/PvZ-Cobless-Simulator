# pragma once
# include "rng.hpp"
# include "common.hpp"

class HitTimeCalculator {
private:
    rng rng;

public:
    // 自动入参
    int M;
    PlantData p;
    bool idle;

    // 手动入参 
    enum TypeCal{
        RANDOM,
        FASTEST,
        SLOWEST,
    } type_cal;

    // 过程变量
    std::vector<int> t0_dist;

    // 结果
    std::vector<int> state;  // 0/1trig/2hit

    HitTimeCalculator(
        PlantType t, int M
    ) : p(PlantData(t)), M(M) { 
        type_cal = TypeCal::RANDOM;
        for (int i = 0; i < p.max_interval - 14; i++) 
            t0_dist.push_back(15);
        for (int i = 14; i > 0; i--) 
            t0_dist.push_back(i);
    }

    void calculate_hit_time(){
        state.clear();
        state = std::vector<int>(M+p.max_interval, 0);
        auto t0 = 1 + rng.random_weighted_sample(t0_dist);
        for(;t0<M+p.max_interval;){
            state[t0] = 1;
            for(auto &i : p.hit_list)
                if(t0 + i < M+p.max_interval)
                    state[t0 + i] = 2;
                
            if(type_cal==TypeCal::RANDOM)
                t0 += p.max_interval - rng.randint(15);
            else if(type_cal==TypeCal::FASTEST)
                t0 += p.max_interval - 14;
            else if(type_cal==TypeCal::SLOWEST)
                t0 += p.max_interval;
        }
        state.erase(state.begin(), state.begin() + p.max_interval);
    }
    
private:
    std::vector<size_t> test_rnd(int N){
        std::vector<size_t> arr;
        for (int i = 0; i < N; i++) 
            arr.push_back(1 +rng.random_weighted_sample(t0_dist));
        return arr;
    }
};
