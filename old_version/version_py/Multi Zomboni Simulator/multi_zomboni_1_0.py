import random
import numpy as np
import pandas as pd
# 你需要修改的地方会用 ### 和 ### 作提示
###
show_pro = True     # 是否展示测试过程
trans_exe = True    # 终端输入

a = 1
n = 4              # 测试集大小 a*10^n
test_N = a * 10**n          # a * 10**n

num_zomboni = 6     # 0表示随机生成

# 模拟出怪
total_weight = 7401
land_num = 5
is_flag = False

crush_col = 6       # 守列
crush_type = 0      # 普通0 南瓜1 冰道2
x_crush = -200

plant_list = [
    [6, 0, True],
    [6, 0, True],
    [5, 0, True],
    [5, 0, False],

    [4, 1, True],

    ##[0, 2, True],
    [-40, 2, True],
    [[0,0], 4, True],
    [[0,0], 4, True],
    [[0,0], 4, True],
    [0, 2, True],
    [0, 2, True],
    [0, 2, True],
    [0, 2, True],
]
# 添加输出植物格式:
# IO(参数,植物类型 曾哥0/大喷1/指定溅射范围瓜2/指定溅射时间瓜3,是否永动True/False),
# 如果种类为曾0/喷1,则参数代表植物所在列
# 如果种类为指定溅射范围瓜2，则默认溅射范围一格(x_cans=80+40px),输入参数单位px修正溅射范围,如希望少溅射半格,则参数输入-40(一格80px)
# 如果种类为指定溅射时间瓜3，则参数单位cs代表可溅射的时间
# 如果种类为指定正面瓜4，则参数单位cs为两个值，代表溅射范围与直射范围，数值同种类2
###
def show_info():
    print(
        show_pro,
        test_N ,
        crush_col ,
        crush_type)
    print(plant_list)
    print(num_zomboni,total_weight,land_num,is_flag)
if trans_exe:
    # show_info()
    file_name = 'zomboni_test_info.xlsx'
    info_read = pd.read_excel(file_name, usecols='B', skiprows=0, nrows=20, header=None)
    info_list = info_read.iloc[:, 0].tolist()

    crush_col = int(info_list[0])
    tmp = info_list[1]
    if tmp == "普通":
        crush_type = 0
    elif tmp == "南瓜":
        crush_type = 1
    else:
        crush_type = 2
    test_N = int(info_list[3])  # a * 10**n
    show_pro = info_list[4] == "是"

    num_zomboni = int(info_list[6])  # a * 10**n

    if num_zomboni==0:
        land_num=int(info_list[8])
        is_flag = info_list[9] == "是"
        check_turn = info_list[10] == "是"
        total_weight = 2401+1000*int(info_list[13])+1500*int(info_list[14])+2000*int(info_list[15])+3000*int(info_list[16])+3500*int(info_list[17])
        total_weight = total_weight + (1000*int(info_list[18]) if is_flag else 0)
        if int(info_list[19])==1:
            if is_flag: total_weight = total_weight + 6000
            elif not check_turn: total_weight = total_weight + 1000

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

    info_read = pd.read_excel(file_name, usecols='I:L', skiprows=2, header=None, na_values='')
    info_read.dropna(inplace=True)
    info_list = info_read.apply(lambda row: row.tolist(), axis=1).tolist()
    for tmp in info_list:
        for _ in range(int(tmp[3])):
            a = [int(80*(tmp[0]-1)),int(80*(tmp[1]-1))]
            b = 4
            c = tmp[2] == "永动"
            plant_list.append([a, b, c])


    info_read = pd.read_excel(file_name, usecols='N:P', skiprows=2, header=None, na_values='')
    info_read.dropna(inplace=True)
    info_list = info_read.apply(lambda row: row.tolist(), axis=1).tolist()
    for tmp in info_list:
        for _ in range(int(tmp[2])):
            a = int(80*(tmp[0]-1))
            b = 2
            c = tmp[1] == "永动"
            plant_list.append([a, b, c])

    info_read = pd.read_excel(file_name, usecols='R:T', skiprows=2, header=None, na_values='')
    info_read.dropna(inplace=True)
    info_list = info_read.apply(lambda row: row.tolist(), axis=1).tolist()
    for tmp in info_list:
        for _ in range(int(tmp[2])):
            a = int(tmp[0])
            b = 3
            c = tmp[1] == "永动"
            plant_list.append([a, b, c])

    # show_info()

M_sup = 3500
M = M_sup
t_crush = 0
def x_list():
    xx = np.zeros(8000)
    xx[0] = 800
    for i in range(1,8000):
        tmp = int(xx[i-1])
        dlt_x = 0.25 if tmp>700 else (0.10 if tmp<=400 else (0.0005*tmp-0.10))
        xx[i] = xx[i-1] - dlt_x
        if xx[i]<-150:
            break
    return xx
x = x_list()
def init():
    global x_crush
    global t_crush
    global M
    if x_crush<=-50:
        tmp = crush_col*80
        if crush_type == 0:
            x_crush = tmp
        elif crush_type == 1:
            x_crush = tmp + 30
        elif crush_type == 2:
            x_crush = tmp - 80 -9
    l = 0
    r = len(x)-1
    while True:
        if l == r-1:
            t_crush = r
            break
        mid = int((l+r)/2)
        if int(x[mid])<=x_crush:
            r = mid
        else :
            l = mid
    M = t_crush + 50
init()
class IO:
    def __init__(self,co,ty,al):
        self.type = ty
        self.x_atk = 800
        self.list = []
        self.start = -1
        if ty == 0: #yyg
            self.x_atk = min(co*80-40+159 , 800)
            self.list = self.hit_list(al,[158,130,102,74],200,20)
        elif ty == 1: #dpg
            self.x_atk = min(co*80-40+399 , 800)
            self.list = self.hit_list(al,[49],150,20)
        elif ty == 2: #melon set space
            self.x_atk = min(int((crush_col if crush_type!=2 else crush_col-1)*80+120+co) ,800)
            self.list = self.hit_list(al,[150],300,26)
        elif ty == 3: #melon set time
            self.x_atk = min(int(x[t_crush-co]),800)
            self.list = self.hit_list(al,[150],300,26)
        elif ty == 4: #melon direct co[0]溅射 co[1]直接
            self.x_atk = min(int((crush_col if crush_type!=2 else crush_col-1)*80+120+co[0]) ,800)
            self.x_dir_atk = min(int((crush_col if crush_type!=2 else crush_col-1)*80+120+co[1]) ,800)
            self.list = self.hit_list(al,[150],300,26)
    @staticmethod
    def hit_list(always, hit, max_cd, dmg):
        tmp = np.zeros(M)
        interval = random.randint(max_cd-14,max_cd)
        start = random.randint(0,interval-1)
        t = interval-start
        # fix
        prob = random.randint(0,(max_cd-7)*15-1)
        if prob<15*(max_cd-14):
            t = 1 + int(prob/15)
            start = interval - t
        else:
            it_list = [14,27,39,50,60,69,77,84,90,95,99,102,104,105]
            prob = prob - 15*(max_cd-14)    #[0,105)
            for i in range(0,14):
                if prob < it_list[i]:
                    t = max_cd - 14 +1 +i
                    interval = random.randint(max_cd - 14+i + 1,max_cd)
                    start = interval - t
                    break
        # end fix
        if always:
            for i in hit:
                if start<=i:
                    tmp[i-start] = dmg
        while True:
            interval = random.randint(max_cd-14,max_cd)
            if t + interval>=M:
                break
            for i in hit:
                tmp[t+i] = dmg
            t = t +interval
        return tmp


# print(f'x_real = {x[t_crush]}')
# print(f't_capacity = {M}')
def is_crush():
    # 生成输出植物
    plt = []
    for p in plant_list:
        plt.append(IO(p[0], p[1], p[2]))

    # 模拟自然出怪
    global num_zomboni
    check = num_zomboni==0
    if check:
        m = np.random.binomial(41 if is_flag else 50,2000.0/total_weight)
        num_zomboni = np.random.binomial(m,1.0/land_num)

    # 生成冰车
    hp = [1350]*num_zomboni
    if check:   num_zomboni=0

    for i in range(len(x)):
        x_now = int(x[i])

        hp = [_hp for _hp in hp if _hp > 0]
        if len(hp)<=0:
            return False
        if x_now <= x_crush:
            return True
        for j in range(len(hp)):
            if hp[j]<=199 and random.randint(1,5)<=3:
                hp[j] = hp[j] - 1
        for p in plt:
            if p.start!=-1:
                if x_now <= p.x_atk and p.list[i-p.start] != 0:
                    for j in range(len(hp)):
                        if j==0 and p.type==4 and x_now<=p.x_dir_atk:
                            hp[j] = hp[j] - 80
                        else:   hp[j] = hp[j] - p.list[i-p.start]
            elif x_now <= p.x_atk+1:
                p.start = i

def my_tst(N):
    cnt = 0
    tmp = 0
    for i in range(1,N+1):
        if is_crush():
            cnt=cnt+1
        if i == N/100+tmp and show_pro:
            print(f"{int(100*i/N)}% {100.0*cnt/i}%")
            tmp = i
    print(f'crush info:(x,t) = ({x_crush},{t_crush})')
    print(f"rate:{100.0*cnt/N:.3f}%")

my_tst(test_N)

if trans_exe:
    print("回车结束程序",end='')
    end_exe = input()
