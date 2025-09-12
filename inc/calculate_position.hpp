# pragma once
# include "rng.hpp"
# include "common.hpp"
#include <cstddef>

class PositionCalculator {
private:
    rng rng;

public:
    // 自动入参
    int M;
    ZombieData z;
    bool huge_wave;
    std::vector<int> ice_t;
    std::vector<int> splash_t;   

    // 手动入参 
    enum TypeCal{
        RANDOM,
        FASTEST,
        SLOWEST,
    } type_cal;
    int action_cd;  // jack 类外更新

    // 过程变量
    float v0;
    std::vector<int> frozen_t[2];   // 0first / 1second

    // 结果
    std::vector<float> x;
    int t_enter = -1; 
    int res = -1;    // jack 爆炸时机, snorkel潜入时机

    PositionCalculator(
        ZombieType t, int M, bool huge_wave,
        std::vector<int> i_t, std::vector<int> s_t
    ) : z(ZombieData(t)), M(M), huge_wave(huge_wave), ice_t(i_t), splash_t(s_t) { type_cal = TypeCal::RANDOM;}

    // x0, v0, frozen time
    void init(){
        t_enter = -1;res = -1;
        x.clear();        
        float x0;
        switch (type_cal) {
            case TypeCal::RANDOM:
                x0 = float(z.spawn_l + rng.randint(z.spawn_offset));
                break;
            case TypeCal::FASTEST:
                x0 = float(z.spawn_l);
                break;
            case TypeCal::SLOWEST:
                x0 = float(z.spawn_l + z.spawn_offset - 1);
                break;
        }
        if (huge_wave)  x0 += z.spawn_hugewave_offset;
        x.push_back(x0);
        
        if(z.type == ZombieType::Zomboni)   return;

        v0 = rng.randfloat(z.speed.first, z.speed.second);
        frozen_t[0].clear(); frozen_t[1].clear();
        for(auto t:ice_t){
            int t_rand0, t_rand1;
            if (type_cal == TypeCal::RANDOM){
                t_rand0 = 400 + rng.randint(201);
                t_rand1 = 300 + rng.randint(101);
            }
            else if (type_cal == TypeCal::FASTEST){
                t_rand0 = 400;
                t_rand1 = 300;
            }
            else if (type_cal == TypeCal::SLOWEST){
                t_rand0 = 600;
                t_rand1 = 400;
            }
            frozen_t[0].push_back(t_rand0);
            frozen_t[1].push_back(t_rand1);
        }
    }

    void calculate_position() {
        if (z.type == ZombieType::Zomboni) {
            calculate_zomboni();
        }
        else if(
            z.type == ZombieType::Balloon ||
            z.type == ZombieType::Digger ||
            z.type == ZombieType::Pogo ||
            z.type == ZombieType::Catapult
        ){
            calculate_constant();
        }
        else if(z.type == ZombieType::DolphinRider){
            calculate_dolphin_rider();
        }
        else if (z.type == ZombieType::Snorkel){
            calculate_snorkel();
        }
        else if(z.type == ZombieType::Catapult_Shoot){
            calculate_catapult_shoot();
        }
        else{
            calculate_animation();
        }
    }
    
private:
    void calculate_zomboni() {
        float v;
        for(size_t i=1;i<M;i++){
            float x_old = x[i-1];
            double dx = std::floor(x_old - 700)/400;

            if(x_old>400){
                double dx = std::floor(double(x_old-700)) / -400;
                if (dx>0)
                    dx = dx * -0.1999999992549419 + 0.25;
                else
                    dx = 0.25;
                v = static_cast<float>(dx);
            }

            float x_new = x_old - v;
            x.push_back(x_new);
            if (x_old <= z.threshold && t_enter == -1)   t_enter = i;
        }
    }

    void calculate_animation(){
        // int end_frame = z.begin_frame + z.n_frames - 1;
        float progress = 0.0f;
        float fps = z.n_frames/z.total_x*v0*47;

        progress += static_cast<float>(fps*double(0.01f)/z.n_frames);

        int frozen_cd = 0, slow_cd = 0, ice_idx = 0, splash_idx = 0;
        for(size_t i=1;i<M;i++){
            // plant ice
            if (ice_idx < ice_t.size() && ice_t[ice_idx] == static_cast<int>(i)){
                frozen_cd = std::max(slow_cd>0?frozen_t[1][ice_idx]:frozen_t[0][ice_idx], frozen_cd);
                slow_cd = std::max(2000, slow_cd);
                ice_idx++;
            }

            // cd
            if (z.type==ZombieType::JackInTheBox)
                if(action_cd>0 && frozen_cd==0) action_cd--;
            
            if (frozen_cd>0) frozen_cd--;
            if (slow_cd>0) slow_cd--;

            // status
            if (z.type==ZombieType::JackInTheBox)
                if(action_cd<=0 && res==-1) res = i;
            
            // position
            /*
                float current_frame = progress * (z.n_frames-1) + z.begin_frame;
                double floored_current_frame = floor(current_frame);
                floored_current_frame = round(floored_current_frame);
                auto x_each = z.anim[static_cast<unsigned int>(floored_current_frame) - z.begin_frame];            
            */
            float dx = z.anim[static_cast<unsigned int>(progress*(z.n_frames-1))];
            
            if(frozen_cd>0){
                dx = 0.0f;
            }
            else if(slow_cd>0){
                dx *= 0.01f*(fps*0.5f);
                progress += static_cast<float>(fps* 0.5f *double(0.01f)/z.n_frames);
            }
            else{
                dx *= 0.01f*fps;
                progress += static_cast<float>(fps*double(0.01f)/z.n_frames) ;
            }
            for (; progress >= 1; progress--);
            float x_new = x[i-1] - dx;
            x.push_back(x_new);

            // projectile splash
            if (splash_idx < splash_t.size() && splash_t[splash_idx] == static_cast<int>(i)){
                if (int(x[i])+z.def_x.first <=800)
                    slow_cd = std::max(1000, slow_cd);
                splash_idx++;
            }

            if (int(x[i-1]) <= z.threshold && t_enter == -1)   t_enter = i;
        }
    };

    void calculate_constant(){
        int frozen_cd = 0, slow_cd = 0, ice_idx = 0, splash_idx = 0;
        for(size_t i=1;i<M;i++){
            // plant ice
            if (ice_idx < ice_t.size() && ice_t[ice_idx] == static_cast<int>(i)){
                if(z.type==ZombieType::Catapult)
                    frozen_cd = std::max(slow_cd>0?frozen_t[1][ice_idx]:frozen_t[0][ice_idx], frozen_cd);
                if(z.type!=ZombieType::Digger)
                    slow_cd = std::max(2000, slow_cd);
                ice_idx++;
            }

            // cd
            if (frozen_cd>0) frozen_cd--;
            if (slow_cd>0) slow_cd--;

            // position
            float dx = v0;
            if(frozen_cd>0)     dx = 0.0f;
            else if(slow_cd>0)  dx *= 0.4f;
            
            float x_new = x[i-1] - dx;
            x.push_back(x_new);

            // projectile splash
            if (splash_idx < splash_t.size() && splash_t[splash_idx] == static_cast<int>(i)){
                if(z.type!=ZombieType::Digger && z.type!=ZombieType::Balloon)
                    if (int(x[i])+z.def_x.first <=800)
                        slow_cd = std::max(1000, slow_cd);
                splash_idx++;
            }

            if (int(x[i-1]) <= z.threshold && t_enter == -1)   t_enter = i;
        }
    }

    void calculate_dolphin_rider(){
        // int end_frame = z.begin_frame + z.n_frames - 1;
        float progress = 0.0f;
        float fps = z.n_frames/z.total_x*v0*47;

        progress += static_cast<float>(fps*double(0.01f)/z.n_frames);

        int frozen_cd = 0, slow_cd = 0, ice_idx = 0, splash_idx = 0;
        size_t i = 1;
        unsigned n_frames = 45;


        // walking
        for(;i<M;i++){
            // plant ice
            if (ice_idx < ice_t.size() && ice_t[ice_idx] == static_cast<int>(i)){
                frozen_cd = std::max(slow_cd>0?frozen_t[1][ice_idx]:frozen_t[0][ice_idx], frozen_cd);
                slow_cd = std::max(2000, slow_cd);
                ice_idx++;
            }

            // cd
            if (frozen_cd>0) frozen_cd--;
            if (slow_cd>0) slow_cd--;

            // status
            if(int(x[i-1])<=720 && frozen_cd<=0){
                fps = 16;

                // position
                x.push_back(x[i-1]);

                // progress
                progress = static_cast<float>(fps*double(0.01f)/n_frames) ;
                
                // projectile splash
                if (splash_idx < splash_t.size() && splash_t[splash_idx] == static_cast<int>(i)){
                    if (int(x[i])+z.def_x.first <=800)
                        slow_cd = std::max(1000, slow_cd);
                    splash_idx++;
                }
                i++;break;
            }

            // position
            /*
                float current_frame = progress * (z.n_frames-1) + z.begin_frame;
                double floored_current_frame = floor(current_frame);
                floored_current_frame = round(floored_current_frame);
                auto x_each = z.anim[static_cast<unsigned int>(floored_current_frame) - z.begin_frame];            
            */
            float dx = z.anim[static_cast<unsigned int>(progress*(z.n_frames-1))];
            
            if(frozen_cd>0){
                dx = 0.0f;
            }
            else if(slow_cd>0){
                dx *= 0.01f*(fps*0.5f);
                progress += static_cast<float>(fps* 0.5f *double(0.01f)/z.n_frames);
            }
            else{
                dx *= 0.01f*fps;
                progress += static_cast<float>(fps*double(0.01f)/z.n_frames) ;
            }
            for (; progress >= 1; progress--);
            float x_new = x[i-1] - dx;
            x.push_back(x_new);

            // projectile splash
            if (splash_idx < splash_t.size() && splash_t[splash_idx] == static_cast<int>(i)){
                if (int(x[i])+z.def_x.first <=800)
                    slow_cd = std::max(1000, slow_cd);
                splash_idx++;
            }
        }


        // jump in pool ???
        for (;i<M;i++){
            // plant ice
            if (ice_idx < ice_t.size() && ice_t[ice_idx] == static_cast<int>(i)){
                slow_cd = std::max(2000, slow_cd);
                ice_idx++;
            }

            // cd
            if (frozen_cd>0) frozen_cd--;
            if (slow_cd>0) slow_cd--;

            // status
            if(progress>=1 && frozen_cd<=0){
                // position
                float dx = v0;
                if(frozen_cd>0)     dx = 0.0f;
                else if(slow_cd>0)  dx *= 0.4f;
                float x_new = x[i-1] - dx - 70;
                x.push_back(x_new);
                
                // projectile splash
                if (splash_idx < splash_t.size() && splash_t[splash_idx] == static_cast<int>(i)){
                    if (int(x[i])+z.def_x.first <=800)
                        slow_cd = std::max(1000, slow_cd);
                    splash_idx++;
                }
                i++; break;
            }

            // position
            x.push_back(x[i-1]);

            // progress
            progress += static_cast<float>(fps*double(0.01f)/n_frames) ;
            
            // projectile splash
            if (splash_idx < splash_t.size() && splash_t[splash_idx] == static_cast<int>(i)){
                if (int(x[i])+z.def_x.first <=800)
                    slow_cd = std::max(1000, slow_cd);
                splash_idx++;
            }
        }


        // ride
        for(;i<M;i++){
            // plant ice
            if (ice_idx < ice_t.size() && ice_t[ice_idx] == static_cast<int>(i)){
                frozen_cd = std::max(300, frozen_cd);
                slow_cd = std::max(2000, slow_cd);
                ice_idx++;
            }

            // cd
            if (frozen_cd>0) frozen_cd--;
            if (slow_cd>0) slow_cd--;

            // status
            if(frozen_cd<=0 && int(x[i-1])<=10){
                v0 = rng.randfloat(z.speed.first, z.speed.second);
                // v0 = static_cast<float>(z.speed.second);
                fps = z.n_frames/z.total_x*v0*47;
                progress = 0.0f;

                // position
                /*
                    float current_frame = progress * (z.n_frames-1) + z.begin_frame;
                    double floored_current_frame = floor(current_frame);
                    floored_current_frame = round(floored_current_frame);
                    auto x_each = z.anim[static_cast<unsigned int>(floored_current_frame) - z.begin_frame];            
                */
                float dx = z.anim[static_cast<unsigned int>(progress*(z.n_frames-1))];
                
                if(frozen_cd>0){
                    dx = 0.0f;
                }
                else if(slow_cd>0){
                    dx *= 0.01f*(fps*0.5f);
                    progress += static_cast<float>(fps* 0.5f *double(0.01f)/z.n_frames);
                }
                else{
                    dx *= 0.01f*fps;
                    progress += static_cast<float>(fps*double(0.01f)/z.n_frames);
                }
                for (; progress >= 1; progress--);
                float x_new = x[i-1] - dx;
                x.push_back(x_new);

                // projectile splash
                if (splash_idx < splash_t.size() && splash_t[splash_idx] == static_cast<int>(i)){
                    if (int(x[i])+z.def_x.first <=800)
                        slow_cd = std::max(1000, slow_cd);
                    splash_idx++;
                }
                i++; break;
            }

            // position
            float dx = v0;
            if(frozen_cd>0)     dx = 0.0f;
            else if(slow_cd>0)  dx *= 0.4f;
            
            float x_new = x[i-1] - dx;
            x.push_back(x_new);

            // projectile splash
            if (splash_idx < splash_t.size() && splash_t[splash_idx] == static_cast<int>(i)){
                if (int(x[i])+z.def_x.first <=800)
                    slow_cd = std::max(1000, slow_cd);
                splash_idx++;
            }
        }


        // walking
        for(;i<M;i++){
            // plant ice
            if (ice_idx < ice_t.size() && ice_t[ice_idx] == static_cast<int>(i)){
                frozen_cd = std::max(slow_cd>0?frozen_t[1][ice_idx]:frozen_t[0][ice_idx], frozen_cd);
                slow_cd = std::max(2000, slow_cd);
                ice_idx++;
            }

            // cd
            if (frozen_cd>0) frozen_cd--;
            if (slow_cd>0) slow_cd--;


            // position
            /*
                float current_frame = progress * (z.n_frames-1) + z.begin_frame;
                double floored_current_frame = floor(current_frame);
                floored_current_frame = round(floored_current_frame);
                auto x_each = z.anim[static_cast<unsigned int>(floored_current_frame) - z.begin_frame];            
            */
            float dx = z.anim[static_cast<unsigned int>(progress*(z.n_frames-1))];
            
            if(frozen_cd>0){
                dx = 0.0f;
            }
            else if(slow_cd>0){
                dx *= 0.01f*(fps*0.5f);
                progress += static_cast<float>(fps* 0.5f *double(0.01f)/z.n_frames);
            }
            else{
                dx *= 0.01f*fps;
                progress += static_cast<float>(fps*double(0.01f)/z.n_frames) ;
            }
            for (; progress >= 1; progress--);
            float x_new = x[i-1] - dx;
            x.push_back(x_new);

            // projectile splash
            if (splash_idx < splash_t.size() && splash_t[splash_idx] == static_cast<int>(i)){
                if (int(x[i])+z.def_x.first <=800)
                    slow_cd = std::max(1000, slow_cd);
                splash_idx++;
            }

            if (int(x[i-1]) <= z.threshold && t_enter == -1)   t_enter = i;
        }
    }

    void calculate_snorkel(){
        // int end_frame = z.begin_frame + z.n_frames - 1;
        float progress = 0.0f;
        float fps = z.n_frames/z.total_x*v0*47;

        progress += static_cast<float>(fps*double(0.01f)/z.n_frames);

        int frozen_cd = 0, slow_cd = 0, ice_idx = 0, splash_idx = 0;
        size_t i = 1;
        unsigned n_frames = 19;


        // walking
        for(;i<M;i++){
            // plant ice
            if (ice_idx < ice_t.size() && ice_t[ice_idx] == static_cast<int>(i)){
                frozen_cd = std::max(slow_cd>0?frozen_t[1][ice_idx]:frozen_t[0][ice_idx], frozen_cd);
                slow_cd = std::max(2000, slow_cd);
                ice_idx++;
            }

            // cd
            if (frozen_cd>0) frozen_cd--;
            if (slow_cd>0) slow_cd--;

            // status
            if(int(x[i-1])<=720 && frozen_cd<=0){
                v0 = 0.2f; fps = 16;

                // position
                float x_new = x[i-1] - v0;
                x.push_back(x_new);

                // progress
                progress = static_cast<float>(fps*double(0.01f)/n_frames) ;
                
                // projectile splash
                if (splash_idx < splash_t.size() && splash_t[splash_idx] == static_cast<int>(i)){
                    if (int(x[i])+z.def_x.first <=800)
                        slow_cd = std::max(1000, slow_cd);
                    splash_idx++;
                }
                i++;break;
            }

            // position
            /*
                float current_frame = progress * (z.n_frames-1) + z.begin_frame;
                double floored_current_frame = floor(current_frame);
                floored_current_frame = round(floored_current_frame);
                auto x_each = z.anim[static_cast<unsigned int>(floored_current_frame) - z.begin_frame];            
            */
            float dx = z.anim[static_cast<unsigned int>(progress*(z.n_frames-1))];
            
            if(frozen_cd>0){
                dx = 0.0f;
            }
            else if(slow_cd>0){
                dx *= 0.01f*(fps*0.5f);
                progress += static_cast<float>(fps* 0.5f *double(0.01f)/z.n_frames);
            }
            else{
                dx *= 0.01f*fps;
                progress += static_cast<float>(fps*double(0.01f)/z.n_frames) ;
            }
            for (; progress >= 1; progress--);
            float x_new = x[i-1] - dx;
            x.push_back(x_new);

            // projectile splash
            if (splash_idx < splash_t.size() && splash_t[splash_idx] == static_cast<int>(i)){
                if (int(x[i])+z.def_x.first <=800)
                    slow_cd = std::max(1000, slow_cd);
                splash_idx++;
            }
        }


        // jump in pool ???
        for (;i<M;i++){
            // plant ice
            if (ice_idx < ice_t.size() && ice_t[ice_idx] == static_cast<int>(i)){
                slow_cd = std::max(2000, slow_cd);
                ice_idx++;
            }

            // cd
            if (frozen_cd>0) frozen_cd--;
            if (slow_cd>0) slow_cd--;

            // status
            if(progress>=1 && frozen_cd<=0){
                // status
                if(res==-1) res = i;
            
                // position
                float dx = v0;
                if(frozen_cd>0)     dx = 0.0f;
                else if(slow_cd>0)  dx *= 0.4f;
                float x_new = x[i-1] - dx;
                x.push_back(x_new);
                
                // projectile splash
                if (splash_idx < splash_t.size() && splash_t[splash_idx] == static_cast<int>(i)){
                    if (int(x[i])+z.def_x.first <=800)
                        slow_cd = std::max(1000, slow_cd);
                    splash_idx++;
                }
                i++; break;
            }

            // position
            float x_new = x[i-1] - v0;
            x.push_back(x_new);

            // progress
            progress += static_cast<float>(fps*double(0.01f)/n_frames) ;
            
            // projectile splash
            if (splash_idx < splash_t.size() && splash_t[splash_idx] == static_cast<int>(i)){
                if (int(x[i])+z.def_x.first <=800)
                    slow_cd = std::max(1000, slow_cd);
                splash_idx++;
            }
        }


        // swim
        for(;i<M;i++){
            // plant ice
            if (ice_idx < ice_t.size() && ice_t[ice_idx] == static_cast<int>(i)){
                frozen_cd = std::max(300, frozen_cd);
                slow_cd = std::max(2000, slow_cd);
                ice_idx++;
            }

            // cd
            if (frozen_cd>0) frozen_cd--;
            if (slow_cd>0) slow_cd--;

            // status
            if(frozen_cd<=0 && int(x[i-1])<=25){
                v0 = rng.randfloat(z.speed.first, z.speed.second);
                // v0 = static_cast<float>(z.speed.first);
                fps = z.n_frames/z.total_x*v0*47;
                progress = 0.0f;


                // position
                /*
                    float current_frame = progress * (z.n_frames-1) + z.begin_frame;
                    double floored_current_frame = floor(current_frame);
                    floored_current_frame = round(floored_current_frame);
                    auto x_each = z.anim[static_cast<unsigned int>(floored_current_frame) - z.begin_frame];            
                */
                float dx = z.anim[static_cast<unsigned int>(progress*(z.n_frames-1))];
                
                if(frozen_cd>0){
                    dx = 0.0f;
                }
                else if(slow_cd>0){
                    dx *= 0.01f*(fps*0.5f);
                    progress += static_cast<float>(fps* 0.5f *double(0.01f)/z.n_frames);
                }
                else{
                    dx *= 0.01f*fps;
                    progress += static_cast<float>(fps*double(0.01f)/z.n_frames);
                }
                for (; progress >= 1; progress--);
                float x_new = x[i-1] - 15.0f - dx;
                x.push_back(x_new);

                // projectile splash
                if (splash_idx < splash_t.size() && splash_t[splash_idx] == static_cast<int>(i)){
                    if (int(x[i])+z.def_x.first <=800)
                        slow_cd = std::max(1000, slow_cd);
                    splash_idx++;
                }
                i++; break;
            }

            // position
            float dx = v0;
            if(frozen_cd>0)     dx = 0.0f;
            else if(slow_cd>0)  dx *= 0.4f;
            
            float x_new = x[i-1] - dx;
            x.push_back(x_new);

            // projectile splash
            if (splash_idx < splash_t.size() && splash_t[splash_idx] == static_cast<int>(i)){
                if (int(x[i])+z.def_x.first <=800)
                    slow_cd = std::max(1000, slow_cd);
                splash_idx++;
            }
        }


        // walking
        for(;i<M;i++){
            // plant ice
            if (ice_idx < ice_t.size() && ice_t[ice_idx] == static_cast<int>(i)){
                frozen_cd = std::max(slow_cd>0?frozen_t[1][ice_idx]:frozen_t[0][ice_idx], frozen_cd);
                slow_cd = std::max(2000, slow_cd);
                ice_idx++;
            }

            // cd
            if (frozen_cd>0) frozen_cd--;
            if (slow_cd>0) slow_cd--;


            // position
            /*
                float current_frame = progress * (z.n_frames-1) + z.begin_frame;
                double floored_current_frame = floor(current_frame);
                floored_current_frame = round(floored_current_frame);
                auto x_each = z.anim[static_cast<unsigned int>(floored_current_frame) - z.begin_frame];            
            */
            float dx = z.anim[static_cast<unsigned int>(progress*(z.n_frames-1))];
            
            if(frozen_cd>0){
                dx = 0.0f;
            }
            else if(slow_cd>0){
                dx *= 0.01f*(fps*0.5f);
                progress += static_cast<float>(fps* 0.5f *double(0.01f)/z.n_frames);
            }
            else{
                dx *= 0.01f*fps;
                progress += static_cast<float>(fps*double(0.01f)/z.n_frames) ;
            }
            for (; progress >= 1; progress--);
            float x_new = x[i-1] - dx;
            x.push_back(x_new);

            // projectile splash
            if (splash_idx < splash_t.size() && splash_t[splash_idx] == static_cast<int>(i)){
                if (int(x[i])+z.def_x.first <=800)
                    slow_cd = std::max(1000, slow_cd);
                splash_idx++;
            }

            if (int(x[i-1]) <= z.threshold && t_enter == -1)   t_enter = i;
        }
    }

    void calculate_catapult_shoot(){
        float progress = 0.0f;
        float fps = 24.0f;

        int frozen_cd = 0, slow_cd = 0, ice_idx = 0, splash_idx = 0;
        size_t i = 1;
        unsigned n_frames = 32;

        // move
        for(;i<M;i++){
            // plant ice
            if (ice_idx < ice_t.size() && ice_t[ice_idx] == static_cast<int>(i)){
                frozen_cd = std::max(slow_cd>0?frozen_t[1][ice_idx]:frozen_t[0][ice_idx], frozen_cd);
                slow_cd = std::max(2000, slow_cd);
                ice_idx++;
            }

            // cd
            if (frozen_cd>0) frozen_cd--;
            if (slow_cd>0) slow_cd--;

            // status ???
            if(frozen_cd<=0 && x[i-1]<=650){
                // std::cout<<i<<'\n';
                // position
                x.push_back(x[i-1]);

                // progress
                if(frozen_cd<=0){
                    if(slow_cd>0)
                        progress += static_cast<float>(fps* 0.5f *double(0.01f)/n_frames);         
                    else
                        progress += static_cast<float>(fps*double(0.01f)/n_frames);
                }

                // projectile splash
                if (splash_idx < splash_t.size() && splash_t[splash_idx] == static_cast<int>(i)){
                    if (int(x[i])+z.def_x.first <=800)
                        slow_cd = std::max(1000, slow_cd);
                    splash_idx++;
                }
                i++; break;
            }

            // position
            float dx = v0;
            if(frozen_cd>0)     dx = 0.0f;
            else if(slow_cd>0)  dx *= 0.4f;
            
            float x_new = x[i-1] - dx;
            x.push_back(x_new);

            // projectile splash
            if (splash_idx < splash_t.size() && splash_t[splash_idx] == static_cast<int>(i)){
                if (int(x[i])+z.def_x.first <=800)
                    slow_cd = std::max(1000, slow_cd);
                splash_idx++;
            }
        }

        // shoot
        for(;i<M;i++){
            // plant ice
            if (ice_idx < ice_t.size() && ice_t[ice_idx] == static_cast<int>(i)){
                frozen_cd = std::max(slow_cd>0?frozen_t[1][ice_idx]:frozen_t[0][ice_idx], frozen_cd);
                slow_cd = std::max(2000, slow_cd);
                ice_idx++;
            }

            // cd
            if (frozen_cd>0) frozen_cd--;
            if (slow_cd>0) slow_cd--;

            // status
            if(progress>0.545f && frozen_cd<=0){
                if(t_enter==-1) t_enter = i;
                break;
            }

            // position
            x.push_back(x[i-1]);

            // progress
            if(frozen_cd<=0){
                if(slow_cd>0)
                    progress += static_cast<float>(fps* 0.5f *double(0.01f)/n_frames);         
                else
                    progress += static_cast<float>(fps*double(0.01f)/n_frames);
            }
            for (; progress >= 1; progress--);

            // projectile splash
            if (splash_idx < splash_t.size() && splash_t[splash_idx] == static_cast<int>(i)){
                if (int(x[i])+z.def_x.first <=800)
                    slow_cd = std::max(1000, slow_cd);
                splash_idx++;
            }
        }
        
        for(;i<M;i++)   x.push_back(x[i-1]);
    }

};
