#include <cstdio>
# include <fstream>
# include <iomanip>

# include <mutex>
# include <thread>

# include "inc/calculate_position.hpp"
# include "inc/calculate_hit_time.hpp"
# include "inc/util.hpp"
using namespace std;
const int M = 5000;

void test_hit_time(){
    HitTimeCalculator cal(PlantType::Gloomshroom, M);
    cal.calculate_hit_time();
    write_vector_to_csv(cal.state, "output.csv");
}

void test_x(){
    PositionCalculator cal(ZombieType::Catapult_Shoot, M, false, {1}, {});
    cal.type_cal = PositionCalculator::TypeCal::FASTEST;

    cal.init();
    cal.v0 = cal.z.speed.second;
    cal.calculate_position();
    write_vector_to_csv(cal.x, "output.csv");
    std::cout<<"t_enter: "<<cal.t_enter<<endl;
}

void test_x_extrem(){
    PositionCalculator cal(ZombieType::JackInTheBox, M, false, {1}, {});
    cal.type_cal = PositionCalculator::TypeCal::SLOWEST;
    
    auto v_range = cal.z.speed;
    auto v_ull_start = bit_cast<uint32_t>(static_cast<float>(v_range.first));
    auto v_ull_end = bit_cast<uint32_t>(static_cast<float>(v_range.second));

    vector<float> x;
    if (cal.type_cal == PositionCalculator::TypeCal::FASTEST)
        x.resize(M, 1000.0f);
    else
        x.resize(M, -1000.0f);

    for(auto i = v_ull_start; i <= v_ull_end; ++i){
        cal.init();
        cal.v0 = bit_cast<float>(i);
        cal.calculate_position();
        for(int j=0;j<M;j++)
            if(cal.x[j]<x[j] && cal.type_cal == PositionCalculator::TypeCal::FASTEST)
                x[j] = cal.x[j];
            else if (cal.x[j] > x[j] && cal.type_cal == PositionCalculator::TypeCal::SLOWEST)
                x[j] = cal.x[j];
        if (i% 10000 == 0) {
            printf("%u %u %u\n", v_ull_start, i, v_ull_end);
        }
    }
    write_vector_to_csv(x, "output.csv");
}

// 多线程加速版本
void test_x_extrem_mt() {
    ///
    PositionCalculator cal(ZombieType::Pogo, M, false, {1}, {});
    cal.type_cal = PositionCalculator::TypeCal::SLOWEST;
    //

    auto v_range = cal.z.speed;
    auto v_ull_start = bit_cast<uint32_t>(static_cast<float>(v_range.first));
    auto v_ull_end = bit_cast<uint32_t>(static_cast<float>(v_range.second));
    uint32_t total = v_ull_end - v_ull_start + 1;

    unsigned int n_threads = std::thread::hardware_concurrency();
    if (n_threads == 0) n_threads = 4; // fallback

    std::atomic<int> finished(0);

    std::vector<std::vector<float>> x_threads(n_threads);
    for (auto& x : x_threads) {
        if (cal.type_cal == PositionCalculator::TypeCal::FASTEST)
            x.resize(M, 1000.0f);
        else
            x.resize(M, -1000.0f);
    }

    std::vector<std::thread> threads;
    for (unsigned int t = 0; t < n_threads; ++t) {
        threads.emplace_back([&, t]() {
            /// 
            PositionCalculator cal_local(cal.z.type, cal.M, cal.huge_wave, cal.ice_t, cal.splash_t);
            cal_local.type_cal = cal.type_cal;
            uint32_t start = v_ull_start + t * total / n_threads;
            uint32_t end = (t == n_threads - 1) ? v_ull_end : v_ull_start + (t + 1) * total / n_threads - 1;
            for (uint32_t i = start; i <= end; ++i) {
                cal_local.init();
                cal_local.v0 = bit_cast<float>(i);
                cal_local.calculate_position();
                for (int j = 0; j < M; j++) {
                    if (cal_local.type_cal == PositionCalculator::TypeCal::FASTEST) {
                        if (cal_local.x[j] < x_threads[t][j])
                            x_threads[t][j] = cal_local.x[j];
                    } else {
                        if (cal_local.x[j] > x_threads[t][j])
                            x_threads[t][j] = cal_local.x[j];
                    }
                }
                finished++;
            }
        });
    }

    while (finished < total) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        printf("%5.5f%%\n", 100.0 * finished.load() / total);
        fflush(stdout);
    }
    for (auto& th : threads) th.join();

    // 合并结果
    std::vector<float> x;
    if (cal.type_cal == PositionCalculator::TypeCal::FASTEST)
        x.resize(M, 1000.0f);
    else
        x.resize(M, -1000.0f);
    for (unsigned int t = 0; t < n_threads; ++t) {
        for (int j = 0; j < M; ++j) {
            if (cal.type_cal == PositionCalculator::TypeCal::FASTEST) {
                if (x_threads[t][j] < x[j]) x[j] = x_threads[t][j];
            } else {
                if (x_threads[t][j] > x[j]) x[j] = x_threads[t][j];
            }
        }
    }
    write_vector_to_csv(x, "output.csv",true);
}

int main(){
    // test_x();
    test_x_extrem_mt();
    // test_hit_time();
    
    // auto res =  wilson_confidence_interval(0,1000000);
    // std::cout<<res.first*100<<"% ~ "<<res.second*100<<"%"<<std::endl;
    return 0;
}