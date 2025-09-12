import copy
import random
import numpy as np
# 你需要修改的地方会用 ### 和 ### 作提示
###
show_pro = True     # 是否展示测试过程

a = 1
n = 5               # 测试集大小 a*10^n
not_flag = True     # 是否通常波

scene = 1           # 0前院 1后院/屋顶平面
boom_type = 3       # 0上炸下 1下炸上 2随机测炸 3正炸
boom_col = 7        # 被炸列数
boom_ngt = False    # 是否炸南瓜

ice_t = []       # 冰时机列表
slow_t = [400]         # 减速时机列表
slow_with_melon = True      # 减速伴随溅射伤害
melon = 0           # 固定溅射数量

stop_xt = [[710,100]]
# 啃食信息列表，其中每个元素为[啃食坐标,啃食时长]，且从左到右啃食坐标需要递减
# 例如  [[750,100],[600,100]]
# n列植物啃食坐标=80*n-10，南瓜+30，高坚果+20，炮-10
###

M_sup = 4000
M = M_sup
N_animation = 18
total_x = 20.9
x_each_animation = [0,
	0, 0.6, 0.6, 0.7, 3.6, 3.6, 1, 1, 0, 0.9,
	1, 1,   1.8, 1.9,   0,   0, 1.6, 1.6]
class Jack:
    def __init__(self,ty):
        self.hp = 334 - 26*melon
        self.v = random.uniform(0.66, 0.68)
        self.timing = self.rnd_xc_countdown(ty)
        self.x = self.x_list_xc()
    def x_list_xc(self):
        global M
        global stop_xt
        M = M_sup
        speed_para = self.v
        x = np.zeros(M)
        s = np.ones(2*M)
        x[0] = random.randint(780, 819) + (0 if not_flag else 40)
        completing_rate = 0
        for t in slow_t:
            s[t:t+1000] = 0.5
        for t in ice_t:
            frozen = random.randint(400,600) if s[t]==1 else random.randint(300,400)
            s[t:t+frozen] = 0
            s[t+frozen:t+2000] = 0.5
            if self.timing > t:
                self.timing = self.timing + frozen
            M = self.timing + 10

        ori_xt = copy.deepcopy(stop_xt)
        for i in range(1,M):
            if len(stop_xt)>0 and int(x[i-1])<=stop_xt[0][0]:
                stop_xt[0][1] = stop_xt[0][1]-1
                x[i] = x[i-1]
                if stop_xt[0][1] == 0:
                    tmp = stop_xt[0]
                    stop_xt.remove(tmp)
                    temp = ori_xt[0]
                    if max(s[temp[0]:(temp[0]+temp[1])])>0:
                        completing_rate = 0
                    ori_xt.remove(temp)
                    # print(i,'rate',completing_rate)
                continue
            speed = speed_para*s[i]
            completing_rate = completing_rate+0.47/total_x*speed
            if completing_rate>1:
                completing_rate = completing_rate-1
            dlt_x = 0.47/total_x*(1+N_animation)*speed*x_each_animation[int(1+completing_rate*N_animation)]
            x[i]=x[i-1]-dlt_x
            if x[i]<=-100:
                break
        return x
    def rnd_xc_countdown(self,ty):
        if ty==0: #no constraint
            tmp = random.randint(0,19)
            if tmp == 0:
                return 2 * int(int((random.randint(0, 299) + 450) / 3) / self.v)
            else:
                return 2*int((random.randint(0,299)+450)/self.v)
        if ty==1: #early
            return 2*int(int((random.randint(0,299)+450)/3)/self.v)
        if ty==2: #late
            return 2*int((random.randint(0,299)+450)/self.v)
class IO:
    def __init__(self,co,ty,al):
        self.x_atk = 764
        self.list = []
        self.start = -1
        if ty == 0: #yyg
            self.x_atk = min(co*80-40+159-36 , 764)
            self.list = self.hit_list(al,[158,130,102,74],200,20)
        elif ty == 1: #dpg
            self.x_atk = min(co*80-40+399-36 , 764)
            self.list = self.hit_list(al,[49],150,20)
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
def is_boom():
    ###
    #Jack参数: 0无限制,1锁早爆,2锁晚爆
    jack = Jack(1)
    plt = [
        #IO(7,0,True),
        IO(7,0,True),
        #IO(8,0,True),
        #IO(6, 0, True),
        #IO(5, 0, True),

        #IO(4,1,True),
    ]
    #添加输出植物格式:
    # IO(植物所在列,植物类型 曾哥0/大喷1,是否永动True/False),
    ###
    x_boom = boom_col*80-10+(30 if boom_ngt else 0)
    if boom_type == 3:
        x_boom = x_boom + 70
    elif scene ==0:
        if boom_type == 0:
            x_boom = x_boom +36
        elif boom_type == 1:
            x_boom = x_boom +54
        elif boom_type == 2:
            x_boom = x_boom +random.choice([36,54])
    elif scene == 1:
        if boom_type == 0:
            x_boom = x_boom +51
        elif boom_type == 1:
            x_boom = x_boom +62
        elif boom_type == 2:
            x_boom = x_boom +random.choice([51,62])
    for i in range(1,M):
        x_now = int(jack.x[i])
        if i == jack.timing:
            return x_now<=x_boom
        if jack.hp<=0:
            return False
        if i in ice_t:
            jack.hp = jack.hp - 20
        if slow_with_melon and i in slow_t:
            jack.hp = jack.hp - 26
        for p in plt:
            if p.start!=-1:
                if x_now <= p.x_atk and p.list[i-p.start] != 0:
                    jack.hp = jack.hp - p.list[i-p.start]
            elif x_now <= p.x_atk+1:
                p.start = i
    print('excel')
def my_tst(N):
    cnt = 0
    tmp = 0
    for i in range(0,N):
        if is_boom():
            cnt=cnt+1
        if i == N/100+tmp and show_pro:
            print(f"{int(100*i/N)}% {100.0*cnt/i}%")
            tmp = i
    print(f"ice_t:{ice_t}")
    print(f"slow_t:{slow_t}")
    print(f"rate:{100.0*cnt/N:.3f}%")
def quick_tst(s,b):
    print(s,b)
    global scene
    global boom_type
    scene = s
    boom_type = b
    my_tst(a* 10**n)
    print('\n')

#quick_tst(1,1)
#quick_tst(1,0)
#quick_tst(0,0)
#quick_tst(0,1)
my_tst(a * 10 ** n)
