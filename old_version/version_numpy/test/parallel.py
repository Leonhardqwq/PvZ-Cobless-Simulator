import time

test_N = 5 * 10**7
show_pro = True  

def main_simu(N):
    time.sleep(0.1)
    return N

import concurrent.futures

if __name__ == "__main__":    
    inner_N_const = 1* 10**4
    outer_N = test_N//inner_N_const
    inner_Ns = [inner_N_const]*outer_N 
    if test_N%inner_N_const != 0:
        inner_Ns.append(test_N%inner_N_const)  
        outer_N += 1

    start_time = time.time()
    cumulative_res = 0
    cumulative_tests = 0
    
    # 多线程
    with concurrent.futures.ProcessPoolExecutor() as executor:
        results = list(executor.map(main_simu, inner_Ns))
        for i, (res, inner_N) in enumerate(zip(results, inner_Ns)):
            cumulative_res += res
            cumulative_tests += inner_N
            res_current = cumulative_res / cumulative_tests
            if show_pro:
                print(f"进度{i+1}/{outer_N}: {res_current*100:,}%")

    end_time = time.time()
    print(f"\n总运行时间: {end_time-start_time:.2f}s")
    print(f"最终结果: {res_current*100:.6f}%")