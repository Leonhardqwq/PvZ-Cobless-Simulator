#pragma once
#include <fstream>

#include <thread>
#include <atomic>
#include <chrono>

#include "../json.hpp"
#include "../calculate_position.hpp"
#include "../calculate_hit_time.hpp"

using json = nlohmann::json;

class ZomboniConfig {
public:
    // basic param
    int num_test = 500000;
    bool show_progress = false;
    int test_type_plant = 0;       // 0 normal / 1 fast / 2 slow

    // scene param
    int crush_type = 1;       // 0 norm / 1 pump / 2 ice_trail / 3 cob
    int crush_col = 6;        

    // plant param
    std::vector<std::vector<int>> shroom_list = {    // col / type / work
        {6, 0, 1}, 
        {4, 1, 1}
    };

    std::vector<std::vector<int>> melon_list = {    // space / type / work
        {80, 2, 1}, 
        {40, 2, 1}
    };

    // auto param
    int x_crush=-1;

public:
    ZomboniConfig(const char* filename) {
        std::ifstream fin(filename);
        if(!fin){ 
            std::cerr << "无法打开 config.json" << std::endl;
            return;
        }
        json j;
        fin>>j;

        num_test = j["num_test"];
        show_progress = j["show_progress"];
        test_type_plant = j["test_type_plant"];

        crush_type = j["crush_type"];
        crush_col = j["crush_col"];

        shroom_list = j["shroom_list"].get<std::vector<std::vector<int>>>();
        melon_list = j["melon_list"].get<std::vector<std::vector<int>>>();

        init();
    }

    void init(){
        // x_crush
        x_crush = 80*crush_col;
        switch (crush_type){
            case 0: break;
            case 1:
                x_crush += 30;
                break;
            case 2:
                if (crush_col == 9) 
                    x_crush += - 80 - 8;
                else if (crush_col == 1) 
                    x_crush +=  - 80 - 10;
                else 
                    x_crush += - 80 - 11;
                break;
            case 3:
                x_crush += - 10;
                break;
        }   
    }

    void show_info(){
        std::cout << "num_test: " << num_test << std::endl;

        std::cout << "crush_type: " << crush_type << std::endl;
        std::cout << "crush_col: " << crush_col << std::endl;

        std::cout << "shroom: " << std::endl;
        for (const auto& shroom : shroom_list) {
            for (int s : shroom) std::cout << s << " ";
            std::cout << std::endl;
        }

        std::cout << "x_crush: " << x_crush << std::endl;
    }
};

class ZomboniSimulator {
private:
    rng rng;
public:
    // auto param
    int M=-1;
    int t_crush=-1;

    ZomboniConfig config;
    PositionCalculator zomboni;

    std::vector<HitTimeCalculator> shrooms;
    struct ShroomInfo {
        int x;
        bool work;
    };
    std::vector<ShroomInfo> shroom_infos;

    std::vector<HitTimeCalculator> melons;
    struct MelonInfo {
        int x_atk;
        bool work;
    };
    std::vector<MelonInfo> melon_infos;

    ZomboniSimulator(const ZomboniConfig& config) : config(config),
        zomboni(ZombieType::Zomboni, 8000, false, {}, {}) {
        zomboni.type_cal = PositionCalculator::TypeCal::FASTEST;
        zomboni.init();
        zomboni.calculate_position();

        for(int i=0;i<8000;i++)
            if (int(zomboni.x[i]) <= config.x_crush) {
                t_crush = i+1;
                M = t_crush + 100;
                break;
            }

        for (const auto& v : config.shroom_list) {
            shrooms.push_back(HitTimeCalculator(PlantType(v[1]), M));
            shroom_infos.push_back(ShroomInfo{v[0]*80-40, v[2]==1});
            shrooms.back().type_cal = HitTimeCalculator::TypeCal(config.test_type_plant);
        }
        for (const auto& v : config.melon_list) {
            melons.push_back(HitTimeCalculator(PlantType(v[1]), M));
            melon_infos.push_back(MelonInfo{
                v[0] + (80*config.crush_col + (config.crush_type!=2?40:-40))
                , v[2]==1});
            melons.back().type_cal = HitTimeCalculator::TypeCal(config.test_type_plant);
        }
    }

    // return crush
    bool simu() {
        std::vector<bool> shroom_works;
        for (size_t i=0;i<shrooms.size();i++){
            shrooms[i].calculate_hit_time();
            shroom_works.push_back(shroom_infos[i].work);
        }

        std::vector<bool> melon_works;
        for (size_t i=0;i<melons.size();i++){
            melons[i].calculate_hit_time();
            melon_works.push_back(melon_infos[i].work);
        }

        int hp = 1350;
        for(int i=0;i<M;i++){
            // plant
            if (i!=0){
                int x_int = zomboni.x[i-1];
                for (size_t j=0;j<shrooms.size();j++){
                    auto& p = shrooms[j];
                    if (p.state[i]==2 && shroom_works[j]){
                        if (x_int + zomboni.z.def_x.first<=800 &&
                            shroom_infos[j].x + p.p.atk.first <= x_int + zomboni.z.def_x.second &&
                            shroom_infos[j].x + p.p.atk.second >= x_int + zomboni.z.def_x.first)
                            hp -= p.p.dmg;
                    }
                    if (p.state[i]==1 && !shroom_works[j]){
                        if (x_int + zomboni.z.def_x.first<=800 &&
                            shroom_infos[j].x + p.p.trig.first <= x_int + zomboni.z.def_x.second &&
                            shroom_infos[j].x + p.p.trig.second >= x_int + zomboni.z.def_x.first)
                            shroom_works[j] = true;
                    }
                }
            }

            // dying
            if (hp<=0)  return false;
            // crush
            if (i == t_crush) return true;
            // near death
            if (hp<200)
                if (rng.randint(5)==0)
                    hp -= 3;
            
            // melon
            int x_int_now = zomboni.x[i];
            for (size_t j = 0; j < melons.size(); j++) {
                auto& p = melons[j];
                if (p.state[i] == 2 && melon_infos[j].work) {
                    if (x_int_now + zomboni.z.def_x.first <= 800 &&
                        melon_infos[j].x_atk >= x_int_now + zomboni.z.def_x.first)
                        hp -= p.p.dmg;
                }
                if (p.state[i] == 1 && !melon_infos[j].work) {
                    if (x_int_now + zomboni.z.def_x.first <= 800)
                        melon_infos[j].work = true;
                }
            }
        }
      
        return false;
    }

};

// true为并行，false为串行
double work(ZomboniConfig config, bool parallel = false) {
    // std::cout<<config.M<<std::flush;

    auto N = config.num_test;
    if (!parallel) {
        // 串行计算
        ZomboniSimulator sim(config);
        int ans = 0;
        for (int i = 0; i < N; ++i) {
            if (sim.simu()) ans++;
            if (config.show_progress && i % (N/100) == 0 && i > 0) {
                printf("(%5.2lf%%) %5.5f%%\n", 100.0 * i / N, 100.0 * ans / i);
                fflush(stdout);
            }
        }
        printf("%5.5f%% = %d / %d\n", 100.0 * ans / N, ans, N);
        fflush(stdout);
        return 1.0 * ans / N;
    }
    else{
        // 并行计算
        int num_threads = std::thread::hardware_concurrency();
        if (num_threads == 0) num_threads = 4;
        int per_thread = N / num_threads;
        int remain = N % num_threads;

        std::atomic<int> ans(0);
        std::atomic<int> finished(0);
        std::vector<std::thread> threads;
        for (int i = 0; i < num_threads; ++i) {
            int this_N = per_thread + (i < remain ? 1 : 0);
            threads.emplace_back([&, this_N]() {
                ZomboniSimulator sim(config);
                for (int j = 0; j < this_N; ++j) {
                    if (sim.simu())     ans++;
                    finished++; 
                }
            });
        }

        // 进度展示
        if(config.show_progress)
            while (finished < N) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                int now = finished.load();
                if (now == 0)   continue;
                printf("(%5.2lf%%) %5.5f%%\n", 100.0 * now / N, 100.0 * ans.load() / now);
                fflush(stdout);
            }
        for (auto& t : threads) t.join();
        printf("%5.5f%% = %d / %d\n", 100.0 * ans.load() / N, ans.load(), N);
        fflush(stdout);
        return 1.0 * ans.load() / N;
    }
}
