# %%
from info import *
import multiprocessing as mp
import time

# %%
show_pro = True     # 是否展示测试过程
trans_exe = True    # 终端输入


# %%
z_def = +0  # 根据类型修改
###
test_N = 5 * 10**5          
M_sup = 7000

crush_col = 2       # 守列
crush_type = 2      # 普通0 南瓜1 冰道2 炮3

plant_list = [  # 添加输出植物格式: [参数,植物类型,是否永动True/False],
    [6, 0, True],
    [6, 0, True],
    [5, 0, True],
    [5, 0, False],

    [4, 1, True],

    ##[0, 2, True],
    [1, 2, True],
    [1, 2, True],
    [1, 2, True],
    [1, 2, True],
    [1, 2, True],
    [1, 2, True],
    [1, 2, True],
    [1, 2, True],
]
###

# %%
def show_info():
    print(
        show_pro,
        test_N ,
        crush_col ,
        crush_type)
    print(plant_list)
if trans_exe:
    #show_info()
    file_name = 'zomboni_test_info.xlsx'
    info_read = pd.read_excel(file_name, usecols='B', skiprows=0, nrows=7, header=None)
    info_list = info_read.iloc[:, 0].tolist()

    crush_col = int(info_list[0])
    tmp = info_list[1]
    if tmp == "普通":
        crush_type = 0
    elif tmp == "南瓜":
        crush_type = 1
    elif tmp == "冰道":
        crush_type = 2
    elif tmp == "炮":
        crush_type = 3
    test_N = int(info_list[3])  # a * 10**n
    show_pro = info_list[4] == "是"
    M_sup = int(info_list[6])
    
    plant_list = []
    info_read = pd.read_excel(file_name, usecols='D:G', skiprows=2, header=None, na_values='')
    info_read.dropna(inplace=True)
    info_list = info_read.apply(lambda row: row.tolist(), axis=1).tolist()
    for tmp in info_list:
        for _ in range(int(tmp[3])):
            a = int(tmp[0])
            b = 0 if tmp[1] == "曾" else 1
            c = tmp[2] == "永动"
            plant_list.append([a, b, c])

    info_read = pd.read_excel(file_name, usecols='I:K', skiprows=2, header=None, na_values='')
    info_read.dropna(inplace=True)
    info_list = info_read.apply(lambda row: row.tolist(), axis=1).tolist()
    for tmp in info_list:
        for _ in range(int(tmp[2])):
            a = int(80*(tmp[0]))
            b = 2
            c = tmp[1] == "永动"
            plant_list.append([a, b, c])

    #show_info()


# %%
aaa = np.array([-10.5,-11,-11.5])
a_int = aaa.astype(np.int16)
a_int

# %%
x = generate_zomboni_x(M_sup)
x_int = x.astype(np.int16)

def get_xt_crush():
    if crush_type == 0:
        x_crush = crush_col*80
    elif crush_type == 1:
        x_crush = crush_col*80 + 30
    elif crush_type == 2:
        if crush_col == 9:
            x_crush = crush_col*80 - 80 - 8
        elif crush_col == 1:
            x_crush = crush_col*80 - 80 - 10
        else:
            x_crush = crush_col*80 - 80 - 11
    elif crush_type == 3:
        x_crush = crush_col*80 - 10
    t_crush = np.searchsorted(-x_int, -x_crush, side='left') + 1
    return x_crush, t_crush

x_crush, t_crush = get_xt_crush()


# %%
def main_simu(N):
    if N == 0: return 0
    
    # t1 = time.time()
    
    # O(NM/cd)
    plants = []
    for p in plant_list:
        plants.append(IO(
            p[0],p[1],p[2],
            N, M_sup,
            z_def=z_def
        ))
        if p[1] == 2:
            plants[-1].fix_melon_x_atk(splash_s=p[0], crush_col=crush_col, crush_type=crush_type)
    
    # O(Nlog(M))
    cnt = 0
    x_int = x.astype(np.int16)

    # t2 = time.time()
    # print(f"t1: {t2 - t1:.2f}s")

    # plant index
    t_atkable = [np.searchsorted(-x_int, -p.x_atk, side='left') + 1 - (1 if p.type == 'melon' else 0)  for p in plants]    
    t_trigable = [np.searchsorted(-x_int, -p.x_trigger, side='left') + 1  for p in plants]    

    
    for idx in range(N):
        t_first_trig = [p.t_trigger[idx][np.searchsorted(p.t_trigger[idx], t_trigable[i], side='left')] for i, p in enumerate(plants)]
        idx_atk_left_idle = [np.searchsorted(p.t_atk[idx], max(t_atkable[i], t_first_trig[i]), side='left') for i, p in enumerate(plants)]
        idx_atk_left_work = [np.searchsorted(p.t_atk[idx], t_atkable[i] , side='left') for i, p in enumerate(plants)] 

        def check_t_hp_lower(t, hp_lower):
            hp = 1350
            for i, p in enumerate(plants):
                idx_atk_right = np.searchsorted(p.t_atk[idx], t - (1 if p.type == 'melon' else 0), side='right')
                hp = hp - max(0, idx_atk_right - (idx_atk_left_idle[i] if p.idle else idx_atk_left_work[i]) ) * p.dmg
            return hp < hp_lower

        def get_t_near_death():
            t_lo = 0            # False
            t_hi = t_crush      # True
            # [lo, hi]
            if not check_t_hp_lower(t_hi, 200):     return t_hi
            while t_hi - t_lo > 1: # 长度大于2
                t_mid = (t_lo + t_hi) // 2
                if check_t_hp_lower(t_mid, 200):
                    t_hi = t_mid
                else:
                    t_lo = t_mid
            return t_hi
        
        t_near_death = get_t_near_death()   # 该时刻，可以判定自流血 <=> hp<200
        
        if not check_t_hp_lower(t_crush, 1+ 3*np.random.binomial(t_crush - t_near_death, 0.2)):
            cnt += 1

    # t_end = time.time()

    # print(f"t2: {t_end - t2:.2f}s")
    return cnt


# %%
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
    
# %%
# 多线程14.290 
# 单线程3.492


