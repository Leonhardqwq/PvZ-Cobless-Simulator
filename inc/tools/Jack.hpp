#pragma once
#include <fstream>

#include <thread>
#include <atomic>
#include <chrono>

#include "../json.hpp"
#include "../calculate_position.hpp"
#include "../calculate_hit_time.hpp"

using json = nlohmann::json;

class JackConfig {
public:
    // basic param
    int num_test = 500000;
    bool show_progress = false;
    int test_type_zombie = 0;       // 0 normal / 1 fast / 2 slow
    int test_type_plant = 0;       // 0 normal / 1 fast / 2 slow
    bool hugewave = false;

    // scene param
    int scene = 1;           // 0 front / 1 back / 2 roof
    int boom_type = 1;       // 0 up->down / 1 down->up / 2 direct
    int boom_col = 6;        
    int defense_type = 0;    // 0 norm / 1 pump / 2 cob
    int jack_type = 0;       // 0 norm / 1 early / 2 late
  
    // effect param
    std::vector<int> ice_t = {551};
    std::vector<int> splash_t = {};
    int melon = 0;
    
    // timing param
    int ash_time = 3000;
    bool ash_type_card = true;
    std::pair<int, int> ash_range = {-200, -200};
    int vulnerable_time = 0;

    // plant param
    std::vector<std::vector<int>> plant_list = {    // col / type / work
        {6, 0, 1}, 
        {4, 1, 1}
    };

    // auto param
    int M=-1;
    int x_target=-1;

public:
    JackConfig(const char* filename) {
        std::ifstream fin(filename);
        if(!fin){ 
            std::cerr << "无法打开 config.json" << std::endl;
            return;
        }
        json j;
        fin>>j;

        num_test = j["num_test"];
        show_progress = j["show_progress"];
        test_type_zombie = j["test_type_zombie"];
        test_type_plant = j["test_type_plant"];
        hugewave = j["hugewave"];

        scene = j["scene"];
        boom_type = j["boom_type"];
        boom_col = j["boom_col"];
        defense_type = j["defense_type"];
        jack_type = j["jack_type"];

        ice_t = j["ice_t"].get<std::vector<int>>();
        splash_t = j["splash_t"].get<std::vector<int>>();
        melon = j["melon"];

        ash_time = j["ash_time"];
        ash_type_card = j["ash_type_card"];
        ash_range = j["ash_range"].get<std::pair<int, int>>();
        vulnerable_time = j["vulnerable_time"];

        plant_list = j["plant_list"].get<std::vector<std::vector<int>>>();

        init();
    }

    void init(){
        // M
        M = 600 * ice_t.size() + (jack_type==1?800:2300);
        // M = std::min(M, ash_time + 100);

        // x_target
        int x_boom_const[6][3] = {
            {36, 54, 70},   // front
            {51, 62, 70},   // back/roof6+
            {29, 68, 70},   // roof5
            {23, 70, 69},   // roof4- & norm
            {12, 70, 69},   // roof4- & pump
            {26, 69, 69},   // roof4- & cob
        };
        switch (scene) {
            case 0: 
            case 1:
                x_target = x_boom_const[scene][boom_type];
                break;
            default:
                if (boom_col >= 6) 
                    x_target = x_boom_const[1][boom_type];
                else if(boom_col == 5) 
                    x_target = x_boom_const[2][boom_type];
                else 
                    x_target = x_boom_const[3+defense_type][boom_type];
        };
        x_target += boom_col * 80 - 10;
        if (defense_type == 1)  x_target += 30;
        else if(defense_type == 2) x_target -= 10;
    }

    void show_info(){
        std::cout << "num_test: " << num_test << std::endl;
        std::cout << "test_type_zombie: " << test_type_zombie << std::endl;
        std::cout << "test_type_plant: " << test_type_plant << std::endl;
        std::cout << "hugewave: " << hugewave << std::endl;
        
        std::cout << "scene: " << scene << std::endl;
        std::cout << "boom_type: " << boom_type << std::endl;
        std::cout << "boom_col: " << boom_col << std::endl;
        std::cout << "defense_type: " << defense_type << std::endl;
        std::cout << "jack_type: " << jack_type << std::endl;

        std::cout << "ice_t: ";
        for (int i : ice_t) std::cout << i << " ";
        std::cout << std::endl;
        std::cout << "splash_t: ";
        for (int s : splash_t) std::cout << s << " ";
        std::cout << std::endl;
        std::cout << "melon: " << melon << std::endl;

        std::cout << "ash_time: " << ash_time << std::endl;
        std::cout << "ash_type_card: " << ash_type_card << std::endl;
        std::cout << "ash_range: " << ash_range.first << " " << ash_range.second << std::endl;
        std::cout << "vulnerable_time: " << vulnerable_time << std::endl;

        std::cout << "plant: " << std::endl;
        for (const auto& plant : plant_list) {
            for (int p : plant) std::cout << p << " ";
            std::cout << std::endl;
        }

        std::cout << "M: " << M << std::endl;
        std::cout << "x_target: " << x_target << std::endl;
    }
};

class JackSimulator {
private:
    rng rng;
public:
    JackConfig config;
    PositionCalculator jack;
    std::vector<HitTimeCalculator> shrooms;
    struct ShroomInfo {
        int x;
        bool work;
    };
    std::vector<ShroomInfo> shroom_infos;

    JackSimulator(const JackConfig& config) : config(config),
        jack(ZombieType::JackInTheBox, config.M, config.hugewave, config.ice_t, config.splash_t) {
        jack.type_cal = PositionCalculator::TypeCal(config.test_type_zombie);
        for (const auto& v : config.plant_list) {
            shrooms.push_back(HitTimeCalculator(PlantType(v[1]), config.M));
            shroom_infos.push_back(ShroomInfo{v[0]*80-40, v[2]==1});
            shrooms.back().type_cal = HitTimeCalculator::TypeCal(config.test_type_plant);
        }
    }

    // return boom
    bool simu() {
        jack.init();

        jack.action_cd = rng.randint(300) + 450;
        if ((config.jack_type==1) || (config.jack_type==0 && rng.randint(20)==0))
                jack.action_cd /= 3;
        jack.action_cd =  2 * static_cast<unsigned int>(jack.action_cd / jack.v0);

        jack.calculate_position();

        // 剪枝
        if (jack.res == -1) throw std::runtime_error("M is too small");
        if (jack.res + 110 < config.vulnerable_time)  return false;
        if (config.ash_type_card){
            if (jack.res + 110 >= config.ash_time){
                auto x_lim = std::max(int(jack.x[jack.res-1]), int(jack.x[config.ash_time-1]));
                if (x_lim + jack.z.def_x.first <=800 && config.ash_range.first<=x_lim && x_lim<=config.ash_range.second){
                    return false;
                }
            }
        }
        else{
            if (jack.res + 110 >= config.ash_time + 1) {
                auto x_lim = std::max(int(jack.x[jack.res-1]), int(jack.x[config.ash_time]));
                if (x_lim + jack.z.def_x.first <=800 && config.ash_range.first<=x_lim && x_lim<=config.ash_range.second){
                    return false;
                }
            }
        }
        if (int(jack.x[jack.res-1]) > config.x_target)  return false;

        std::vector<bool> shroom_works;
        for (size_t i=0;i<shrooms.size();i++){
            shrooms[i].calculate_hit_time();
            shroom_works.push_back(shroom_infos[i].work);
        }

        int hp = 335 - config.melon*26;
        int ice_idx = 0, splash_idx = 0;

        for(int i=1;i<config.M;i++){
            int x_int = jack.x[i-1];
            // plant & ice
            if (ice_idx < config.ice_t.size() && config.ice_t[ice_idx] == static_cast<int>(i)){
                hp-=20;
                ice_idx++;
            }
            for (size_t j=0;j<shrooms.size();j++){
                auto& p = shrooms[j];
                if (p.state[i]==2 && shroom_works[j]){
                    if (x_int + jack.z.def_x.first<=800 &&
                        shroom_infos[j].x + p.p.atk.first <= x_int + jack.z.def_x.second &&
                        shroom_infos[j].x + p.p.atk.second >= x_int + jack.z.def_x.first)
                        hp -= p.p.dmg;
                }
                if (p.state[i]==1 && !shroom_works[j]){
                    if (x_int + jack.z.def_x.first<=800 &&
                        shroom_infos[j].x + p.p.trig.first <= x_int + jack.z.def_x.second &&
                        shroom_infos[j].x + p.p.trig.second >= x_int + jack.z.def_x.first)
                        shroom_works[j] = true;
                }
            }

            // ash_card
            // if (i==config.ash_time-110 && config.ash_type_card) return false;
            
            // dying
            if (hp<=0)  return false;

            // cd 
            // status - explode
            if (i==jack.res)    return true;
            // position / progress

            // ash_cob
            // if(i==config.ash_time-110 && !config.ash_type_card) return false;

            // melon
            if (splash_idx < config.splash_t.size() && config.splash_t[splash_idx] == static_cast<int>(i)){
                if(int(jack.x[i])+jack.z.def_x.first <=800)
                    hp-=26;
                splash_idx++;
            }
        }
      
        return false;
    }

};


// true为并行，false为串行
double work(JackConfig config, bool parallel = false) {
    // config.show_info();
    // std::cout<<config.M<<std::flush;

    auto N = config.num_test;
    if (!parallel) {
        // 串行计算
        JackSimulator sim(config);
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
        return 1.0*ans / N;
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
                JackSimulator sim(config);
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
        return 1.0*ans.load() / N;
    }
}
