#include "../../inc/tools/Jack.hpp"
#include "../../inc/util.hpp"
#include <algorithm>


void test_ice(int me_st, int me_en, int t_en=550){
    JackConfig config("../jack_config.json");

    auto start = std::chrono::high_resolution_clock::now();
    std::vector<std::vector<double>> all_results;
    for(int i=0;i<=t_en;i+=10){
        printf("%d:\n", i);
        fflush(stdout);
        for(int m=me_st;m<=me_en;m++){
            if(i==0)    all_results.push_back({});
            JackConfig tmp = config;
            if(i==0) tmp.ice_t = {1};
            else tmp.ice_t = {i};
            tmp.init();
            tmp.melon += m;
            all_results[m-me_st].push_back(100.0*work(tmp, 1));
        }
        printf("\n");
    }
    auto end = std::chrono::high_resolution_clock::now(); 
    printf("Total time: %.2lf seconds\n", std::chrono::duration<double>(end - start).count());

    write_2dvector_to_csv(all_results, "results.csv",true);
}

void test_splash_late(int me_st, int me_en, int t_en=1320){
    JackConfig config("../jack_config.json");

    auto start = std::chrono::high_resolution_clock::now();
    std::vector<std::vector<double>> all_results;
    for(int i=170;i<=t_en;i+=10){
        printf("%d:\n", i);
        fflush(stdout);
        for(int m=me_st;m<=me_en;m++){
            if(i==170)    all_results.push_back({});
            JackConfig tmp = config;
            if(m==1) tmp.splash_t = {i};
            else if(m>=2) tmp.splash_t = {i, std::min(i+998, 1321)}, tmp.melon += m-2;
            all_results[m-me_st].push_back(100.0*work(tmp, 1));
        }
        printf("\n");
    }
    auto end = std::chrono::high_resolution_clock::now(); 
    printf("Total time: %.2lf seconds\n", std::chrono::duration<double>(end - start).count());

    write_2dvector_to_csv(all_results, "results.csv",true);
}

void test_splash_early(int me_st, int me_en, int t_en=1320){
    JackConfig config("../jack_config.json");

    auto start = std::chrono::high_resolution_clock::now();
    std::vector<std::vector<double>> all_results;
    for(int i=170;i<=t_en;i+=10){
        printf("%d:\n", i);
        fflush(stdout);
        for(int m=me_st;m<=me_en;m++){
            if(i==170)    all_results.push_back({});
            JackConfig tmp = config;
            tmp.splash_t = {i};
            tmp.melon += m-1;
            all_results[m-me_st].push_back(100.0*work(tmp, 1));
        }
        printf("\n");
    }
    auto end = std::chrono::high_resolution_clock::now(); 
    printf("Total time: %.2lf seconds\n", std::chrono::duration<double>(end - start).count());

    write_2dvector_to_csv(all_results, "results.csv",true);
}


int main() {
    /*
    test_splash_early(
        1
        , 3
        , 700
    );
    //*/

    ///*
    test_splash_late(
        5
        ,12
        ,1320
    );
    //*/

    /*
    test_ice(
        0
        , 3
        , 550
    );    
    //*/

    return 0;
}