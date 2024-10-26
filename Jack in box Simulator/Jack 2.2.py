import random
import numpy as np
show_pro = True

a = 1
n = 5
not_flag = True     #是否通常波

scene = 0           # 0前院 1后院 2屋顶
boom_type = 0       # 0上炸下 1下炸上 2随机测炸 3正炸
boom_col = 7        # 被炸列数
boom_ngt = False     # 炸南瓜

ice_t = [551]       # 冰时机
slow_t = []         # 减速时机
slow_with_melon = True      # 冰瓜减速
melon = 0           # 溅射数量

M = 3000
N_animation = 18
total_x = 20.9
x_each_animation = [0,
	0, 0.6, 0.6, 0.7, 3.6, 3.6, 1, 1, 0, 0.9,
	1, 1,   1.8, 1.9,   0,   0, 1.6, 1.6]
x0_left = 780 if not_flag else 780+40
x0_right = 819 if not_flag else 819+40
class Jack:
    def __init__(self,ty):
        self.hp = 334 - 26*melon
        self.timing = self.rnd_xc_countdown(ty)
        self.x = self.x_list_xc(random.uniform(0.66, 0.68))

    def x_list_xc(self,speed_para):
        x = np.zeros(M)
        s = np.ones(2*M)
        x[0] = random.uniform(x0_left, x0_right)
        completing_rate = 0

        for t in slow_t:
            s[t:t+1000] = 0.5
        for t in ice_t:
            frozen = random.randint(400,600) if s[t]==1 else random.randint(300,400)
            s[t:t+frozen] = 0
            s[t+frozen:t+2000] = 0.5
            if self.timing > t:
                self.timing = self.timing + frozen

        for i in range(1,M):
            speed = speed_para*s[i]
            completing_rate = completing_rate+0.47/total_x*speed
            if completing_rate>1:
                completing_rate = completing_rate-1
            v = 0.47/total_x*(1+N_animation)*speed*x_each_animation[int(1+completing_rate*N_animation)]
            x[i]=x[i-1]-v
            if x[i]<=-100:
                break
        return x

    @staticmethod
    def cdf(t):
        if t<=441:
            return 0
        if t<=454:
            return (0.578*(t-441)+112500*(1/t-1/441))/198
        c1 = (0.578*(454-441)+112500*(1/454-1/441))/198
        if t<=732:
            return 0.0335*(t-454)/198 + c1
        c2 = 0.0335*(732-454)/198 + c1
        if t<=754:
            return (-0.5445*(t-732)-310005*(1/t-1/732))/198 + c2
        c3 = (-0.5445*(754-732)-310005*(1/754-1/732))/198 + c2
        if t<=1323:
            return c3
        if t<=1363:
            return (10.982*(t-1323)+19237500*(1/t-1/1323))/598 + c3
        c4 = (10.982*(1363-1323)+19237500*(1/1363-1/1323))/598 + c3
        if t<=2202:
            return 0.6365*(t-1363)/598 + c4
        c5 = 0.6365*(2202-1363)/598 + c4
        if t<=2269:
            return (-10.3455*(t-2202)-53295095*(1/t-1/2202))/598 + c5

    def find_cdf_reverse(self,p):
        ep = 0.5
        lb=441
        rb=2269
        while True:
            mid = (lb+rb)/2
            fm = self.cdf(mid)
            if fm<p:
                lb = mid
            else:
                rb = mid
            if rb-lb<ep:
                break
        return round(lb)

    def rnd_xc_countdown(self,ty):
        if ty==0: #no constraint
            return self.find_cdf_reverse(random.random())
        if ty==1: #early
            return self.find_cdf_reverse(random.uniform(0,self.cdf(754)))
        if ty==2: #late
            return self.find_cdf_reverse(random.uniform(self.cdf(1323),self.cdf(2269)))
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
    jack = Jack(1)
    plt = [
            IO(7,0,True),
            #IO(6,0,True),
            IO(6,0,True),

            IO(5,1,True),
            IO(4,1,True)
    ]

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
        if i == jack.timing:
            return jack.x[i]<=x_boom
        if jack.hp<=0:
            return False
        if i in ice_t:
            jack.hp = jack.hp - 20
        if slow_with_melon and i in slow_t:
            jack.hp = jack.hp - 26
        for p in plt:
            if p.start!=-1:
                if jack.x[i] <= p.x_atk and p.list[i-p.start] != 0:
                    jack.hp = jack.hp - p.list[i-p.start]
            elif jack.x[i]<=p.x_atk+1:
                p.start = i


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
    print(100.0*cnt/N,'%')
my_tst(a* 10**n)

def tst():
     j = Jack(1)
     print(j.cdf(551))
     print(j.find_cdf_reverse(3.107/100))
#tst()
