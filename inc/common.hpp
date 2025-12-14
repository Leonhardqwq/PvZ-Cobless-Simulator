#pragma once
#include <vector>
#include <cstdio>
#include <iostream>

enum class ZombieType {
    PoleVaulting,
    Newspaper,
    ScreenDoor,
    Football,
    Snorkel,
    Zomboni,
    DolphinRider,
    JackInTheBox,
    Balloon,
    Digger,
    Pogo,
    Ladder,
    Catapult,
    Gargantuar,
    GigaGargantuar,
    Catapult_Shoot,
    Unknown
};

class ZombieData{
public:
    ZombieType type;
    int spawn_l, spawn_offset, spawn_hugewave_offset;
    std::pair<double, double> speed;
    std::pair<int, int> def_x;
    std::pair<int, int> atk;
    int threshold;
    int hp;

    std::vector<float> _ground, anim;
    float total_x;
    unsigned begin_frame, n_frames;

    
    ZombieData(ZombieType t) : type(t) {init();}

    void set_anim(){
        if (_ground.empty()) return;
        anim.clear();
        for (size_t i = 1; i < _ground.size(); ++i) 
            anim.push_back(_ground[i] - _ground[i - 1]);
        total_x = _ground.back() - _ground.front();
        n_frames = static_cast<unsigned>(_ground.size());
    }

    void init(){
        // spawn_l
        switch (type) {
            case ZombieType::PoleVaulting:  spawn_l = 870;break;
            case ZombieType::Zomboni:       spawn_l = 800;break;
            case ZombieType::Catapult_Shoot:
            case ZombieType::Catapult:      spawn_l = 825;break;
            case ZombieType::Gargantuar:
            case ZombieType::GigaGargantuar:spawn_l = 845;break;
            default:                        spawn_l = 780;
        }

        // spawn_offset, spawn_hugewave_offset
        switch (type) {
            case ZombieType::PoleVaulting:  
            case ZombieType::Zomboni:       
            case ZombieType::Catapult:
            case ZombieType::Catapult_Shoot:
            case ZombieType::Gargantuar:
            case ZombieType::GigaGargantuar:spawn_offset = 10;spawn_hugewave_offset = 0; break;
            default:                        spawn_offset = 40;spawn_hugewave_offset = 40;
        }

        // speed
        switch (type) {
            case ZombieType::Pogo:          speed = {double(0.45f), double(0.45f)}; break;
            case ZombieType::Newspaper:     
            case ZombieType::ScreenDoor:    
            case ZombieType::Balloon:       
            case ZombieType::Catapult: 
            case ZombieType::Catapult_Shoot:
            case ZombieType::Gargantuar:
            case ZombieType::GigaGargantuar:speed = {double(0.23f), double(0.37f)}; break;

            case ZombieType::Zomboni:       speed = {double(0.0f), double(0.0f)}; break;

            case ZombieType::DolphinRider:  speed = {double(0.89f), double(0.91f)}; break;

            case ZombieType::Ladder:        speed = {double(0.79f), double(0.81f)}; break;

            default:                        speed = {double(0.66f), double(0.68f)}; 
        }

        // def_x
        switch (type) {
            case ZombieType::Football:      def_x = {50, 107}; break;
            case ZombieType::Snorkel:       def_x = {12, 74}; break;
            case ZombieType::Digger:        def_x = {50, 78}; break;
            case ZombieType::Zomboni: 
            case ZombieType::Catapult_Shoot:
            case ZombieType::Catapult:      def_x = {0, 153}; break;
            case ZombieType::Gargantuar:
            case ZombieType::GigaGargantuar:def_x = {-17, 108}; break;
            default:                        def_x = {36, 78};
        }

        // atk
        switch (type) {
            case ZombieType::Snorkel:       atk = {-5, 50}; break;
            case ZombieType::Ladder:        atk = {10, 60}; break;
            case ZombieType::Pogo:          atk = {10, 40}; break;

            case ZombieType::PoleVaulting:  
            case ZombieType::DolphinRider:  atk = {-29, 41}; break;

            case ZombieType::Football:      
            case ZombieType::Digger:        atk = {50, 70}; break;

            case ZombieType::Zomboni:   
            case ZombieType::Catapult_Shoot:
            case ZombieType::Catapult:      atk = {10, 143}; break;

            case ZombieType::Gargantuar:
            case ZombieType::GigaGargantuar:atk = {-30, 59}; break;
            default:                        atk = {20, 70};
        }

        // hp
        switch (type) {
            case ZombieType::PoleVaulting:      
            case ZombieType::DolphinRider:      
            case ZombieType::JackInTheBox:      
            case ZombieType::Ladder:            
            case ZombieType::Pogo:              hp = 335; break;    // 335 / 500

            case ZombieType::Balloon:           hp = 290; break;
            case ZombieType::Digger:            hp = 281; break;
            case ZombieType::Catapult_Shoot:
            case ZombieType::Catapult:          hp = 850; break;
            case ZombieType::Zomboni:           hp = 1350; break;
            case ZombieType::Football:          hp = 1581; break;   // 1581 / 1670
            case ZombieType::Gargantuar:        hp = 3000; break;
            case ZombieType::GigaGargantuar:    hp = 6000; break;

            default:                            hp = 181; break;    // 181 / 270
        }

        // threshold
        switch (type) {
            case ZombieType::PoleVaulting:
            case ZombieType::Gargantuar:
            case ZombieType::GigaGargantuar:
                threshold = -150;
                break;

            case ZombieType::Catapult:
            case ZombieType::Catapult_Shoot:
            case ZombieType::Football:
            case ZombieType::Zomboni:
                threshold = -175;
                break;

            case ZombieType::Snorkel:
                threshold = -130;
                break;

            default:
                threshold = -100;
                break;
        }

        // ground ScreenDoor
        switch (type) {
            case ZombieType::PoleVaulting:
                _ground = {
                                                            -59.8f, -59.0f,
                    -58.2f, -57.3f, -56.5f, -55.7f, -54.8f, -54.0f, -53.2f,
                    -52.3f, -51.5f, -50.7f, -49.8f, -49.0f, -48.2f, -47.3f,
                    -46.5f, -45.7f, -44.8f, -44.0f, -43.2f, -42.3f, -41.5f,
                    -40.7f, -39.8f, -39.0f, -38.2f, -37.3f, -36.5f, -35.7f,
                    -34.8f, -34.0f, -33.2f, -32.3f, -31.5f, -30.7f, -29.8f,
                };
                set_anim();break;
            case ZombieType::Newspaper:
                _ground = {                 
                                            -59.8f, -59.3f, -58.8f, -58.3f,
                    -57.8f, -54.8f, -51.8f, -48.9f, -45.9f, -43.8f, -41.6f,
                    -39.4f, -37.3f, -35.1f, -33.8f, -32.5f, -31.2f, -29.8f,
                    -28.5f, -28.5f, -28.5f, -28.6f, -28.6f, -27.6f, -26.6f,
                    -25.7f, -24.7f, -23.7f, -21.6f, -19.4f, -17.2f, -15.1f,
                    -12.9f, -11.3f, -9.7f, -8.1f, -6.5f, -4.9f, -4.2f,
                    -3.5f, -2.7f, -2.0f, -2.0f, -2.0f, -2.0f, -2.0f,
                    -2.0f,
                };
                set_anim();break;
            case ZombieType::ScreenDoor:
                _ground = {          
                                                            -9.8f,
                    -8.5f, -7.3f, -6.0f, -4.7f, -3.4f, -2.1f, -0.9f,
                    0.3f, 1.6f, 2.8f, 4.1f, 5.4f, 6.7f, 8.0f,
                    9.2f, 10.5f, 10.6f, 10.7f, 10.7f, 10.8f, 10.8f,
                    10.9f, 11.0f, 12.8f, 14.5f, 16.3f, 18.1f, 19.9f,
                    21.6f, 23.4f, 25.2f, 27.0f, 28.8f, 30.5f, 32.3f,
                    34.0f, 35.9f, 37.6f, 39.4f, 39.5f, 39.5f, 39.6f,
                    39.8f, 39.9f, 39.9f, 40.0f,       
                };
                set_anim();break;
            case ZombieType::Football:
                _ground = {       
                                                                    -59.8f,
                    -57.4f, -55.0f, -52.6f, -50.2f, -47.8f, -47.5f, -47.3f,
                    -47.0f, -46.7f, -46.4f, -46.1f, -45.8f, -44.3f, -42.8f,
                    -41.3f, -39.8f, -38.3f, -36.8f, -35.3f, -33.8f, -33.5f,
                    -33.3f, -33.0f, -32.7f, -32.4f, -32.1f, -31.8f, -30.8f,
                    -29.8f,          
                };
                begin_frame = 21;
                set_anim();break;
            case ZombieType::Snorkel:
                _ground = {           
                    -59.8f, -58.2f, -56.5f, -54.8f, -53.2f, -51.5f, -49.8f,
                    -48.2f, -46.5f, -44.8f, -43.2f, -41.5f, -39.8f, -38.2f,
                    -36.5f, -34.8f, -33.2f, -31.5f, -29.8f, -28.2f, -26.5f,
                    -24.8f, -23.2f, -21.5f, -19.8f, -18.2f, -16.5f, -14.8f,
                    -13.2f, -11.5f, -9.8f, -8.2f, -6.5f, -4.8f, -3.2f,
                    -1.5f, 0.1f,   
                };
                set_anim();break;
            case ZombieType::DolphinRider:
                _ground = {      
                    -59.8f, -58.7f, -57.6f, -56.6f, -55.5f, -52.7f, -49.9f,
                    -47.2f, -45.9f, -44.6f, -43.3f, -42.1f, -40.9f, -39.7f,
                    -38.5f, -37.3f, -36.1f, -35.0f, -33.8f, -32.6f, -31.2f,
                    -29.8f, -28.4f, -27.0f, -25.6f, -23.6f, -21.5f, -19.5f,
                    -17.4f, -15.4f, -13.3f, -11.3f, -9.2f, -7.4f, -5.6f,
                    -3.8f, -2.0f, -0.2f, 1.5f,   
                };
                begin_frame = 15;
                set_anim();break;
            case ZombieType::JackInTheBox:
                _ground = {        
                            -49.8f, -49.8f, -49.2f, -48.6f, -47.9f, -44.3f,
                    -40.7f, -39.7f, -38.7f, -38.7f, -37.8f, -36.8f, -35.8f,
                    -34.0f, -32.1f, -32.1f, -32.1f, -30.5f, -28.9f,  
                };
                begin_frame = 30;
                set_anim();break;
            case ZombieType::Ladder:
                _ground = {      
                                            -39.8f, -39.0f, -38.1f, -37.3f,
                    -36.5f, -33.6f, -30.7f, -27.9f, -25.0f, -21.1f, -17.2f,
                    -13.2f, -9.3f, -5.4f, -4.6f, -3.9f, -3.1f, -2.4f,
                    -1.6f, -1.6f, -1.6f, -1.6f, -1.6f, -0.6f, 0.2f,
                    1.2f, 2.1f, 3.0f, 5.2f, 7.3f, 9.6f, 11.7f,
                    13.9f, 15.9f, 18.0f, 20.1f, 22.2f, 24.2f, 24.7f,
                    25.2f, 25.6f, 26.1f, 26.1f, 26.1f, 26.1f, 26.1f,
                    26.1f,            
                };
                begin_frame = 25;
                set_anim();break;
            case ZombieType::Gargantuar:
            case ZombieType::GigaGargantuar:
                _ground = {         
                    -79.8f, -75.3f, -70.8f, -66.3f, -61.9f, -57.4f, -54.5f,
                    -51.5f, -48.5f, -45.6f, -42.6f, -38.2f, -33.8f, -29.4f,
                    -25.0f, -24.6f, -24.1f, -23.7f, -23.2f, -22.8f, -21.1f,
                    -19.4f, -17.7f, -16.0f, -14.3f, -11.9f, -9.5f, -7.1f,
                    -4.7f, -2.3f, 3.0f, 8.4f, 13.8f, 19.2f, 24.6f,
                    29.1f, 33.5f, 38.0f, 42.5f, 42.5f, 42.5f, 42.5f,
                    42.5f, 42.5f, 42.9f, 43.3f, 43.5f, 43.9f, 44.3f,        
                };
                begin_frame = 22;
                set_anim();break;
            default:
                _ground = {};
                set_anim();break;
        }
    }

/*
    void show_info(){
        printf("Type: %d\n", static_cast<int>(type));
        printf("Spawn: %d - %d | %d - %d\n", spawn_l, spawn_l+spawn_offset-1, spawn_l+spawn_hugewave_offset, spawn_l+spawn_offset+spawn_hugewave_offset-1 );
        printf("Speed: %f - %f\n", speed.first, speed.second);
        printf("Defense: %d - %d\n", def_x.first, def_x.second);
        printf("Attack: %d - %d\n", atk.first, atk.second);
        printf("HP: %d\n", hp);
        printf("Animation %llu: %f = [ \n", anim.size(), total_x);
        for (const auto& frame : anim) 
            printf("%f ", frame);
        printf("\n]\n\n");
    }
*/
};

enum class PlantType {
    Gloomshroom,
    Fumeshroom,
    WinterMelon,
    Melonpult,
    Unknown
};

class PlantData{
public:
    PlantType type;
    int dmg, max_interval;
    std::vector<int> hit_list;
    std::pair<int, int> atk;
    // gloom: offset compared to plant
    // melon: offset compared to defense
    std::pair<int, int> trig;

    PlantData(PlantType t) : type(t) {init();}

    void init(){
        switch (type){
            case PlantType::Gloomshroom:
                dmg = 20;
                max_interval = 200;
                hit_list = {74, 102, 130, 158};
                atk = {-79, 159};
                trig = {-80, 160};
                break;
            case PlantType::Fumeshroom:
                dmg = 20;
                max_interval = 150;
                hit_list = {49};
                atk = {61, 399};
                trig = {60, 400};
                break;
            case PlantType::Melonpult:
            case PlantType::WinterMelon:
                dmg = 26;
                max_interval = 300;
                hit_list = {154};
                atk = {-800, 800};
                trig = {-800, 800};
                break;
            default:
                break;
        }
    }

    void set_atk_second(int high){
        atk.second = high;
    }

};
