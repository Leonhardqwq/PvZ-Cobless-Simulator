#include "../inc/tools/Enter.hpp"
#include "../inc/util.hpp"

int main() {
    EnterConfig config("enter_config.json");

    auto start = std::chrono::high_resolution_clock::now();
    auto ci = wilson_confidence_interval( work(config, 1), config.num_test);
    auto end = std::chrono::high_resolution_clock::now(); 

    printf("Wilson Confidence Interval: [%.6lf%%, %.6lf%%]\n", 100.0*ci.first, 100.0*ci.second);
    printf("Total time: %.2lf seconds\n", std::chrono::duration<double>(end - start).count());
    printf("Press Enter to exit...\n");
    fflush(stdout);
    
    // std::cin.get();
    return 0;
}
