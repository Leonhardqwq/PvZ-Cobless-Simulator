#include "../inc/tools/Zomboni.hpp"
#include "../inc/util.hpp"

void test(){
    ZomboniConfig config("zomboni_config.json");
    ZomboniSimulator sim(config);
    for(auto& m : sim.melon_infos)
        std::cout<<m.x_atk<<" "<<m.work<<std::endl;
}

int main() {
    ZomboniConfig  config("zomboni_config.json");
    // 测试进家在这里修改 config.x_crush

    auto start = std::chrono::high_resolution_clock::now();
    auto ci = wilson_confidence_interval( work(config, 1), config.num_test);
    auto end = std::chrono::high_resolution_clock::now(); 

    printf("Wilson confidence interval: [%.6lf%%, %.6lf%%]\n", 100.0*ci.first, 100.0*ci.second);
    printf("Total time: %.2lf seconds\n", std::chrono::duration<double>(end - start).count());
    printf("Press Enter to exit...\n");
    fflush(stdout);

    std::cin.get();
    return 0;
}
