#pragma once
#include <cstdio>
#include <fstream>

#include <thread>
#include <atomic>
#include <chrono>

#include "../json.hpp"
#include "../calculate_position.hpp"
#include "../calculate_hit_time.hpp"

using json = nlohmann::json;

/*
    不考虑交互
    JackInTheBox 不考虑爆炸
    Snorkel,DolphinRider 暂未考虑无敌？？？
    Pogo 不考虑y轴相位

    Zomboni,Catapult 不考虑自流血仅适用判定
    Catapult 不考虑停止
    Catapult_Shoot 不考虑多投和再次移动

    Gargantuar,GigaGargantuar 不考虑投掷
*/

class EnterConfig {
public:
    // basic param
    int num_test = 500000;
    bool show_progress = false;
    int test_type_zombie = 0;       // 0 normal / 1 fast / 2 slow
    int test_type_plant = 0;       // 0 normal / 1 fast / 2 slow
    bool hugewave = false;
    int M = 4000;
    ZombieType type = ZombieType::Football;

    // effect param
    std::vector<int> ice_t = {551};
    std::vector<int> splash_t = {};
    int melon = 0;
    int extra_dmg = 0;

    // ash param
    std::vector<std::vector<int>> ash_infos = {
        // time / type_card / range_left / range_right
        {3000, 1, -200, -200},
    };
    // int ash_time = 3000;
    // bool ash_type_card = true;
    // std::pair<int, int> ash_range = {-200, -200};

    // plant param
    std::vector<std::vector<int>> plant_list = {    // col / type / work
        {6, 0, 1}, 
        {4, 1, 1}
    };

public:
    EnterConfig(const char* filename) {
        std::ifstream fin(filename);
        if(!fin){ 
            std::cerr << "无法打开 config.json" << std::endl;
            return;
        }
        json j;
        fin>>j;

        num_test = j["num_test"];
        show_progress = j["show_progress"];
        type = ZombieType(j["type"]);
        test_type_zombie = j["test_type_zombie"];
        test_type_plant = j["test_type_plant"];
        hugewave = j["hugewave"];
        M = j["M"];

        ice_t = j["ice_t"].get<std::vector<int>>();
        splash_t = j["splash_t"].get<std::vector<int>>();
        melon = j["melon"];
        extra_dmg = j["extra_dmg"];

        ash_infos = j["ash_infos"].get<std::vector<std::vector<int>>>();
        // ash_time = j["ash_time"];
        // ash_type_card = j["ash_type_card"];
        // ash_range = j["ash_range"].get<std::pair<int, int>>();

        plant_list = j["plant_list"].get<std::vector<std::vector<int>>>();
    }

    void show_info(){
        std::cout << "num_test: " << num_test << std::endl;
        std::cout << "show_progress: " << show_progress << std::endl;
        std::cout << "type: " << int(type) << std::endl;
        std::cout << "test_type_zombie: " << test_type_zombie << std::endl;
        std::cout << "test_type_plant: " << test_type_plant << std::endl;

        std::cout << "hugewave: " << hugewave << std::endl;
        std::cout << "M: " << M << std::endl;

        std::cout << "ice_t: ";
        for (int i : ice_t) std::cout << i << " ";
        std::cout << std::endl;
        std::cout << "splash_t: ";
        for (int s : splash_t) std::cout << s << " ";
        std::cout << std::endl;
        std::cout << "melon: " << melon << std::endl;
        std::cout << "extra_dmg: " << extra_dmg << std::endl;

        std::cout << "ash_infos: " << std::endl;
        for (const auto& info : ash_infos) {
            std::cout << "  time: " << info[0]
                      << ", type_card: " << info[1]
                      << ", range: [" << info[2] << ", " << info[3] << "]"
                      << std::endl;
        }
        // std::cout << "ash_time: " << ash_time << std::endl;
        // std::cout << "ash_type_card: " << ash_type_card << std::endl;
        // std::cout << "ash_range: " << ash_range.first << " " << ash_range.second << std::endl;

        std::cout << "plant: " << std::endl;
        for (const auto& plant : plant_list) {
            for (int p : plant) std::cout << p << " ";
            std::cout << std::endl;
        }
    }
};

class EnterSimulator {
public:
    EnterConfig config;
    PositionCalculator zombie;
    std::vector<HitTimeCalculator> shrooms;
    struct ShroomInfo {
        int x;
        bool work;
    };
    std::vector<ShroomInfo> shroom_infos;

    EnterSimulator(const EnterConfig& config) : config(config),
        zombie(config.type, config.M, config.hugewave, config.ice_t, config.splash_t) {
        zombie.type_cal = PositionCalculator::TypeCal(config.test_type_zombie);
        for (const auto& v : config.plant_list) {
            shrooms.push_back(HitTimeCalculator(PlantType(v[1]), config.M));
            shroom_infos.push_back(ShroomInfo{v[0]*80-40, v[2]==1});
            shrooms.back().type_cal = HitTimeCalculator::TypeCal(config.test_type_plant);
        }
    }

    // 这里的函数都是相对于plant时机来说的，相对于projectile时机传参为i+1
    bool check_dolphin_jumpinpool(int i){
        if (zombie.z.type!=ZombieType::DolphinRider)  return false;
        if (i<2) return false;
        if (720>=int(zombie.x[i-2]) && zombie.x[i-1]>660) return true;
        return false;
    }
    bool check_snorkel_jumpinpool(int i){
        if (zombie.z.type!=ZombieType::Snorkel)  return false;
        if (i<2) return false;
        if (720>=int(zombie.x[i-2]) && i<=zombie.res) return true;
        return false;
    }
    bool check_snorkel_swim(int i){
        if (zombie.z.type!=ZombieType::Snorkel)  return false;
        if (i<2) return false;
        if(i>zombie.res && int(zombie.x[i-2])>25) return true;
        return false;
    }
    // 判断是否非dizzy
    bool check_digger_not_dizzy(int i){
        if (zombie.z.type!=ZombieType::Digger)  return false;
        if(i<=zombie.res) return true;
        return false;
    }

    // return break
    bool simu() {
        zombie.init();
        zombie.calculate_position();
        // printf("%d\n", zombie.t_enter);

        // 剪枝
        if (zombie.t_enter == -1)   throw std::runtime_error("M is too small");
        // ash
        for (const auto& info : config.ash_infos){
            int ash_time = info[0];
            bool ash_type_card = info[1]==1;
            std::pair<int, int> ash_range = {info[2], info[3]};
            if (ash_type_card){
                if (zombie.t_enter >= ash_time){
                    auto x_lim = int(zombie.x[ash_time-1]);
                    if (x_lim + zombie.z.def_x.first <=800 && 
                        ash_range.first<=x_lim && 
                        x_lim<=ash_range.second)
                        return false;
                }
            }
            else{
                if (zombie.t_enter >= ash_time + 1) {
                    auto x_lim = int(zombie.x[ash_time]);
                    if (x_lim + zombie.z.def_x.first <=800 && 
                        ash_range.first<=x_lim && 
                        x_lim<=ash_range.second)
                        return false;
                }
            }
        }

        std::vector<bool> shroom_works;
        for (size_t i=0;i<shrooms.size();i++){
            shrooms[i].calculate_hit_time();
            shroom_works.push_back(shroom_infos[i].work);
        }

        int hp = zombie.z.hp - config.melon * 26 - config.extra_dmg;
        int ice_idx = 0, splash_idx = 0;

        for(int i=1;i<config.M;i++){
            int x_int = zombie.x[i-1];
            // plant & ice
            if (ice_idx < config.ice_t.size() && config.ice_t[ice_idx] == static_cast<int>(i)){
                if (zombie.z.type != ZombieType::Zomboni && 
                    zombie.z.type != ZombieType::Balloon &&
                    zombie.z.type != ZombieType::Pogo &&
                    !check_dolphin_jumpinpool(i) && !check_snorkel_jumpinpool(i) && 
                    !check_digger_not_dizzy(i)
                ){
                    // dophin / snorkel  jumpinwater / digger not dizzy
                    hp-=20;
                }

                ice_idx++;
            }
            for (size_t j=0;j<shrooms.size();j++){
                auto& p = shrooms[j];
                if (p.state[i]==2 && shroom_works[j]){
                    if (zombie.z.type != ZombieType::Balloon &&
                        !check_dolphin_jumpinpool(i) && !check_snorkel_jumpinpool(i) && 
                        !check_snorkel_swim(i) &&
                        !check_digger_not_dizzy(i)
                    ){
                        if (x_int + zombie.z.def_x.first<=800 &&
                            shroom_infos[j].x + p.p.atk.first <= x_int + zombie.z.def_x.second &&
                            shroom_infos[j].x + p.p.atk.second >= x_int + zombie.z.def_x.first){
                            hp -= p.p.dmg;
                            // printf("%d ", i);
                            // fflush(stdout);
                        }
                    }
                }
                if (p.state[i]==1 && !shroom_works[j]){
                    if (zombie.z.type != ZombieType::Balloon &&
                        !check_dolphin_jumpinpool(i) && !check_snorkel_jumpinpool(i) && 
                        !check_snorkel_swim(i) &&
                        !check_digger_not_dizzy(i)
                    )
                        if (x_int + zombie.z.def_x.first<=800 &&
                            shroom_infos[j].x + p.p.trig.first <= x_int + zombie.z.def_x.second &&
                            shroom_infos[j].x + p.p.trig.second >= x_int + zombie.z.def_x.first){
                            shroom_works[j] = true;
                            // printf("%d\n", i);
                            // fflush(stdout);
                        }
                }
            }

            // dying
            if (hp<=0){
                return false;
            }  

            // status
            if (i==zombie.t_enter)    return true;
            // position / progress
            x_int = zombie.x[i];


            // melon
            if (splash_idx < config.splash_t.size() && config.splash_t[splash_idx] == static_cast<int>(i)){
                if( zombie.z.type != ZombieType::Balloon && 
                    zombie.z.type != ZombieType::Digger &&
                    !check_dolphin_jumpinpool(i+1) && !check_snorkel_jumpinpool(i+1)
                )
                    if(int(zombie.x[i])+zombie.z.def_x.first <=800)
                        hp-=26;
                splash_idx++;
            }
        }
        return false;
    }
};

// true为并行，false为串行
double work(EnterConfig config, bool parallel = false) {
    // config.show_info();
    // std::cout<<config.M<<std::flush;

    auto N = config.num_test;
    if (!parallel) {
        // 串行计算
        EnterSimulator sim(config);
        int ans = 0;
        for (int i = 0; i < N; ++i) {
            if (sim.simu()) {
                ans++;
            }
            // printf("\n");
            if (config.show_progress && i % (N/100) == 0 && i > 0) {
                printf("(%5.2lf%%) %5.5f%%\n", 100.0 * i / N, 100.0 * ans / i);
                fflush(stdout);
            }
        }
        printf("%5.5f%%\n", 100.0 * ans / N);
        fflush(stdout);
        return 1.0*ans/N;
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
                EnterSimulator sim(config);
                for (int j = 0; j < this_N; ++j) {
                    if (sim.simu()){
                        ans++;
                    }
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
        printf("%5.5f%%\n", 100.0 * ans.load() / N);
        fflush(stdout);
        return 1.0*ans.load()/N;
    }
    // config.show_info();
}