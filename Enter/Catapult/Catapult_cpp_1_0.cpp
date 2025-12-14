#include "../../inc/tools/Enter.hpp"
#include "../../inc/util.hpp"


// true为并行，false为串行
std::vector<double> work_catapult(EnterConfig config, bool parallel = false) {
    // config.show_info();
    // std::cout<<config.M<<std::flush;

    auto N = config.num_test;
    if (!parallel) {
        // 串行计算
        EnterSimulator sim(config);
        std::vector<int> results(3000);
        int ans = 0;
        for (int i = 0; i < N; ++i) {
            if(sim.simu()){
                ans++;
                auto res=sim.zombie.t_enter;
                results[res]++;
            }
            if (config.show_progress && i % (N/100) == 0 && i > 0) {
                printf("(%5.2lf%%) %5.5f%%\n", 100.0 * i / N, 100.0 * ans / i);
                fflush(stdout);
            }
        }
        printf("%5.5f%%\n", 100.0 * ans / N);
        fflush(stdout);

        std::vector<double> results_d;
        for(auto x:results) results_d.push_back(1.0*x/N);
        return results_d;
    }
    else{
        // 并行计算
        int num_threads = std::thread::hardware_concurrency();
        if (num_threads == 0) num_threads = 4;
        int per_thread = N / num_threads;
        int remain = N % num_threads;

        std::atomic<int> ans(0);
        std::atomic<int> finished(0);
        std::vector<std::atomic<int>> results(3000);
        std::vector<std::thread> threads;
        for (int i = 0; i < num_threads; ++i) {
            int this_N = per_thread + (i < remain ? 1 : 0);
            threads.emplace_back([&, this_N]() {
                EnterSimulator sim(config);
                for (int j = 0; j < this_N; ++j) {
                    if (sim.simu()){
                        ans++;
                        auto res=sim.zombie.t_enter;
                        results[res]++;
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

        std::vector<double> results_d;
        for(int i=0;i<3000;i++)
            results_d.push_back(1.0*results[i].load()/N);
        return results_d;
    }
    // config.show_info();
}

int main() {
    EnterConfig config("../enter_config.json");

    auto start = std::chrono::high_resolution_clock::now();
    auto v = work_catapult(config, 1);
    auto end = std::chrono::high_resolution_clock::now(); 

    write_vector_to_csv(v, "result.csv");
    for(int i = 0; i < v.size(); i++)
        if(v[i]!=0){
            printf("Earliest shooting time: %d\n", i);
            break;
        }

    printf("Total time: %.2lf seconds\n", std::chrono::duration<double>(end - start).count());
    printf("Press Enter to exit...\n");
    fflush(stdout);
    std::cin.get();

    return 0;
}
