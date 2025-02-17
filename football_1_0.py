import copy
import random
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
# 你需要修改的地方会用 ### 和 ### 作提示
###
show_pro = True     # 是否展示测试过程
trans_exe = True    # 终端输入

a = 1
n = 4               # 测试集大小 a*10^n
test_N = a * 10**n          # a * 10**n


num_football = 3     # 0表示随机生成
# 模拟出怪
total_weight = 7401+2000
land_num = 5+1
is_flag = False

eat_col = 6        # 被啃食列数
eat_type = 0    # 普通0 南瓜1

plant_list = [
    [6, 0, True],
    [6, 0, True],
    # [5, 0, True],
    # [5, 0, True],

    [5, 1, True],

    ##[0, 2, True],
    # [-40, 2, True],
    [[40,40], 4, True],
    [[40,40], 4, True],
    [[40,40], 4, True],
    [[40,40], 4, True],
    [40, 2, True],
    [40, 2, True],
    [40, 2, True],
    # [0, 2, True],
]

ice_t = []       # 冰时机列表

stop_list = [
    # [8,0],
    # [5,1]
]   # 垫材信息 列+type(norm0,xpg1,hp2) 降序排列，严格大于守列

# 添加输出植物格式:
# IO(参数,植物类型 曾哥0/大喷1/指定溅射范围瓜2/指定溅射时间瓜3,是否永动True/False),
# 如果种类为曾0/喷1,则参数代表植物所在列
# 如果种类为指定溅射范围瓜2，则默认溅射范围一格(x_cans=80+40px),输入参数单位px修正溅射范围,如希望少溅射半格,则参数输入-40(一格80px)
# 如果种类为指定正面瓜4，则参数单位cs为两个值，代表溅射范围与直射范围，数值同种类2
###
def show_info():
    print(
        show_pro,
        test_N ,
        eat_col ,
        eat_type)
    print(plant_list)
    print(ice_t)
    print(stop_list)

    print(num_football,total_weight,land_num,is_flag)
if trans_exe:
    # show_info()
    file_name = 'football_test_info.xlsx'
    info_read = pd.read_excel(file_name, usecols='B', skiprows=0, nrows=20, header=None)
    info_list = info_read.iloc[:, 0].tolist()

    eat_col = int(info_list[0])
    tmp = info_list[1]
    if tmp == "普通":
        eat_type = 0
    elif tmp == "南瓜":
        eat_type = 1

    test_N = int(info_list[3])  # a * 10**n
    show_pro = info_list[4] == "是"

    num_football = int(info_list[6])  # a * 10**n
    is_flag = info_list[9] == "是"

    if num_football==0:
        land_num=int(info_list[8])
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

    ice_t = []
    info_read = pd.read_excel(file_name, usecols='R', skiprows=2,header=None,na_values='')
    info_read.dropna(inplace=True)
    info_list = info_read.iloc[:,0].tolist()
    for info in info_list:  ice_t.append(int(info))

    stop_list = []
    info_read = pd.read_excel(file_name, usecols='T:U', skiprows=2,header=None,na_values='')
    info_read.dropna(inplace=True)
    info_list = info_read.apply(lambda row: row.tolist(), axis=1).tolist()
    for info in info_list:
        if info[1]=='普通':   ty=0
        elif info[1]=='花盆': ty = 2
        else:   ty=1
        stop_list.append([int(info[0]),ty])

    # show_info()

M_sup = 4000
M = M_sup
# 橄榄
N_animation = 29
total_x = 30.0
x_each_animation = [0,
	2.4, 2.4, 2.4, 2.4, 2.4,
    0.3, 0.2, 0.3, 0.3, 0.3,
    0.3, 0.3, 1.5, 1.5, 1.5,
    1.5, 1.5, 1.5, 1.5, 1.5,
    0.3, 0.2, 0.3, 0.3, 0.3,
    0.3, 0.3, 1.0, 1.0]
class Football:
    def __init__(self):
        self.hp = 1670
        self.hp_lim = 90
        self.v = random.uniform(0.66, 0.68)
        self.x = random.randint(780, 819) + (0 if (not is_flag) else 40) # t=0
        self.completing_rate = 0

        self.frozen_cd = 0
        self.slow_cd = 0

        self.eating = False

    # end of every tick
    def update(self):
        if self.eating: self.completing_rate = 0
        else:
            # 位移结算
            speed_para = self.v
            speed = 0 if self.frozen_cd != 0 else (speed_para*(0.5 if self.slow_cd!=0 else 1.0))
            # 循环率更新
            self.completing_rate = self.completing_rate + 0.47/total_x * speed
            if self.completing_rate>1:   self.completing_rate = self.completing_rate-1

            dlt_x = 0.47 / total_x * (1 + N_animation) * speed * x_each_animation[int(1 + self.completing_rate * N_animation)]
            self.x = self.x - dlt_x

        # cd结算
        if self.frozen_cd >0: self.frozen_cd=self.frozen_cd-1
        if self.slow_cd >0: self.slow_cd=self.slow_cd-1

class IO:
    def __init__(self,co,ty,al):
        self.type = ty
        self.x_atk = 750
        self.list = []
        self.start = -1
        if ty == 0: #yyg
            self.x_atk = min(co*80-40+159-50 , 800-50)
            self.list = self.hit_list(al,[158,130,102,74],200,20)
        elif ty == 1: #dpg
            self.x_atk = min(co*80-40+399-50 , 800-50)
            self.list = self.hit_list(al,[49],150,20)
        elif ty == 2: # 冰瓜 melon set space
            self.x_atk = min(int(eat_col*80+70+co) ,800-50)
            self.list = self.hit_list(al,[150],300,26)
        elif ty == 3:  # 西瓜溅射
            self.x_atk = min(int(eat_col * 80 + 70 + co), 800 - 50)
            self.list = self.hit_list(al, [150], 300, 26)
        elif ty == 4: # 冰瓜 melon direct co[0]溅射 co[1]直接
            self.x_atk = min(int(eat_col*80+70+co[0]) ,800-50)
            self.x_dir_atk = min(int(eat_col*80+70+co[1]) ,800-50)
            self.list = self.hit_list(al,[150],300,26)
        elif ty == 5:  # 西瓜直射 melon direct co[0]溅射 co[1]直接
            self.x_atk = min(int(eat_col * 80 + 70 + co[0]), 800 - 50)
            self.x_dir_atk = min(int(eat_col * 80 + 70 + co[1]), 800 - 50)
            self.list = self.hit_list(al, [150], 300, 26)
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

class StopPlant:
    def __init__(self,co,ty):
        self.hp = 300
        self.put_tick = -1
        self.type = ty
        self.x = co*80-40
        if ty==1:
            # 小喷偏移 -5~+4
            self.x = self.x + random.randint(-5,+4)
        if ty==-1:
            self.hp = 0
            self.x = self.x + (30 if eat_type==1 else 0)

def test_dmg():
    # 生成植物
    plt = []
    for p in plant_list:
        plt.append(IO(p[0],p[1],p[2]))
    stp_plt = []
    for sp in stop_list:
        stp_plt.append(StopPlant(sp[0],sp[1]))
    stp_plt.append(StopPlant(eat_col,-1))

    # 模拟自然出怪
    global num_football
    check = num_football==0
    if check:
        m = np.random.binomial(41 if is_flag else 50,2000.0/total_weight)
        num_football = np.random.binomial(m,1.0/land_num)

    # 生成橄榄
    footballs = []
    for _ in range(num_football):
        footballs.append(Football())
    if check:   num_football=0

    '''
    # 结算顺序
        # 植物伤害、用冰、减速
        # 僵尸死亡
        # 僵尸啃食
        # update
            # 僵尸移动（移动到下cs）
            # cd自然更新（移动到下cs）    
    '''

    for i in range(0,M):
        # end
        if len(footballs)==0:
            return -stp_plt[-1].hp

        # 结算冻结
        if i in ice_t:
            for j in range(len(footballs)):
                footballs[j].hp = footballs[j].hp -20
                if footballs[j].slow_cd!=0:
                    footballs[j].frozen_cd = random.randint(300,400)
                else:
                    footballs[j].frozen_cd = random.randint(400,600)
                footballs[j].slow_cd = 2000
        # 植物伤害
        x_min = min([int(footballs[j].x) for j in range(len(footballs))])
        for p in plt:
            if p.start!=-1:
                # 存在伤害
                if p.list[i-p.start]!=0:
                    for j in range(len(footballs)):
                        # 可以攻击
                        x_now = int(footballs[j].x)
                        if x_now<=p.x_atk:
                            # 直伤
                            if j==0 and (p.type==4 or p.type==5):
                                footballs[j].hp = footballs[j].hp - 80
                            # aoe
                            else:
                                footballs[j].hp = footballs[j].hp - p.list[i-p.start]
                            #减速倒计时
                            if p.type==2 or p.type==4:
                                if footballs[j].slow_cd<1000:
                                    footballs[j].slow_cd = 1000

            # 植物启动
            elif x_min<=p.x_atk+1:
                p.start = i

        # 结算死亡
        footballs = [football for football in footballs if football.hp>0]

        # 结算啃食
        if i % 4 == 0:
            for j in range(len(footballs)):
                if footballs[j].frozen_cd!=0:   continue
                if footballs[j].slow_cd!=0 and i%8!=0: continue
                if footballs[j].hp<=footballs[j].hp_lim: continue
                x_now = int(footballs[j].x)
                x_eat = stp_plt[0].x
                if x_now > x_eat and footballs[j].eating:
                    footballs[j].eating = False
                if x_now <= x_eat:
                    footballs[j].eating = True
                    if stp_plt[0].put_tick==-1:
                        stp_plt[0].put_tick=i
                    if not(stp_plt[0].type==2 and (i-stp_plt[0].put_tick)<=100):
                        stp_plt[0].hp = stp_plt[0].hp - 4
            if stp_plt[0].type!=-1 and stp_plt[0].hp<=0:
                stp_plt.remove(stp_plt[0])

        # update
        for j in range(len(footballs)):
            footballs[j].update()

    print('excel')

def my_tst(N):
    ans = []
    tmp = 0
    for i in range(0,N):
        ans.append(test_dmg())
        if i == N/100+tmp and show_pro:
            print(f"{int(100*i/N)}% {np.mean(ans)}")
            tmp = i
    print(f"ice_t:{ice_t}")
    print(f"stop_list:{stop_list}")
    print(f"mean dmg:{np.mean(ans):.3f}")

    # 作图
    ans_list = pd.Series(ans)
    plt.figure(figsize=(30, 10))
    value_counts = ans_list.value_counts().sort_index()
    value_counts.plot(kind='bar')
    plt.xlabel('dmg')
    plt.ylabel('frequency')
    plt.show()

my_tst(test_N)

if trans_exe:
    print("回车结束程序",end='')
    end_exe = input()
