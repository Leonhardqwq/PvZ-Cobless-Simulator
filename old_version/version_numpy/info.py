import numpy as np
import pandas as pd
import copy
import matplotlib.pyplot as plt

class AnimationZombieInfo:
    def __init__(self, N_animation, total_x, x_each_animation, v_range=(0.66, 0.68), x_range=(780, 820)):
        self.N_animation = N_animation
        self.total_x = total_x
        self.x_each_animation = x_each_animation
        self.v_range = v_range
        self.x_range = x_range
    def get_info(self):
        return self.N_animation, self.total_x, self.x_each_animation, self.v_range, self.x_range

ANIMATION_ZOMBIE_INFO = {
    'jack': AnimationZombieInfo(
        N_animation=18,
        total_x=20.9,
        x_each_animation=np.array([
            0. , 0.6, 0.6, 0.7, 3.6, 3.6, 1. , 1. , 0. , 0.9, 
            1. , 1. , 1.8, 1.9, 0. , 0. , 1.6, 1.6
        ], dtype=np.float32),
        v_range=(0.66000003, 0.68000001),
        x_range=(780, 820)
    ),
    'ladder': AnimationZombieInfo(
        N_animation=46,
        total_x=65.9,
        x_each_animation=np.array([
            0.8, 0.9, 0.8, 0.8, 2.9, 2.9, 2.8, 2.9, 3.9, 3.9, 
            4. , 3.9, 3.9, 0.8, 0.7, 0.8, 0.7, 0.8, 0. , 0. , 
            0. , 0. , 1. , 0.8, 1. , 0.9, 0.9, 2.2, 2.1, 2.3, 
            2.1, 2.2, 2. , 2.1, 2.1, 2.1, 2. , 0.5, 0.5, 0.4, 
            0.5, 0. , 0. , 0. , 0. , 0. 
        ], dtype=np.float32),
        v_range=(0.79000002, 0.81),
        x_range=(780, 820)
    ),
    'football': AnimationZombieInfo(
        N_animation=29,
        total_x=30.0,
        x_each_animation=np.array([
            2.4, 2.4, 2.4, 2.4, 2.4, 0.3, 0.2, 0.3, 0.3, 0.3, 
            0.3, 0.3, 1.5, 1.5, 1.5, 1.5, 1.5, 1.5, 1.5, 1.5, 
            0.3, 0.2, 0.3, 0.3, 0.3, 0.3, 0.3, 1. , 1. 
        ], dtype=np.float32),
        v_range=(0.66000003, 0.68000001),
        x_range=(780, 820)
    ),
    'giga': AnimationZombieInfo(
        N_animation=48,
        total_x=124.1,
        x_each_animation=np.array([
            4.5, 4.5, 4.5, 4.4, 4.5, 2.9, 3. , 3. , 2.9, 3. , 
            4.4, 4.4, 4.4, 4.4, 0.4, 0.5, 0.4, 0.5, 0.4, 1.7, 
            1.7, 1.7, 1.7, 1.7, 2.4, 2.4, 2.4, 2.4, 2.4, 5.3, 
            5.4, 5.4, 5.4, 5.4, 4.5, 4.4, 4.5, 4.5, 0. , 0. , 
            0. , 0. , 0. , 0.4, 0.4, 0.2, 0.4, 0.4
        ], dtype=np.float32),
        v_range=(0.23, 0.37),
        x_range=(845, 855)
    ),
}


class PlantInfo:
    def __init__(self, max_cd, dmg, hit_list, atk_range, trigger_range):
        self.max_cd = max_cd
        self.dmg = dmg
        self.hit_list = hit_list
        self.atk_range = atk_range
        self.trigger_range = trigger_range
        self.t0_dist = np.array([15*i for i in range(max_cd -14 + 1)] + [15*(max_cd+i-14) - i*(i+1)//2 for i in range(1, 15)])
        
    def get_info(self):
        return self.max_cd, self.dmg, self.hit_list, self.atk_range, self.trigger_range

PLANT_INFO = {
    'gloom': PlantInfo(
        max_cd=200,
        dmg=20,
        hit_list=np.array([74, 102, 130, 158], dtype=np.int16),
        atk_range=159,
        trigger_range=160,
    ),
    'fume': PlantInfo(
        max_cd=150,
        dmg=20,
        hit_list=np.array([49], dtype=np.int16),
        atk_range=399,
        trigger_range=400,
    ),
    'melon': PlantInfo(
        max_cd=300,
        dmg=26,
        hit_list=np.array([150], dtype=np.int16),
        atk_range=160,
        trigger_range=1000,
    ),
}


def generate_animation_x(
    N, M, 
    N_animation, total_x, x_each_animation, v_range=(0.66,0.68), x_range=(780, 820), 
    not_flag=True,
    ice_t=[], splash_t=[],
    is_jack=False, jack_type=0,  

    speed_type='normal',  # 'normal' or 'fastest' or 'slowest'
    v_dis='random',  # 'random' or 'average'
):
    if v_dis == 'random':
        v = np.random.uniform(v_range[0],v_range[1], size=N).astype(np.float32)
    elif v_dis == 'average':
        v = np.linspace(v_range[0], v_range[1], N, dtype=np.float32) # 测试用

    ###
    if is_jack:
        # jack_type 全局变量
        explode_t = np.zeros(N, dtype=np.int16)
        if jack_type == 1:
            explode_t[:] = 2 * (((np.random.randint(0, 300, size=N, dtype=np.int16) + 450) / 3.0).astype(np.int16) / v).astype(np.int16)
        elif jack_type == 2:
            explode_t[:] = 2 * (((np.random.randint(0, 300, size=N, dtype=np.int16) + 450) / v).astype(np.int16))
        else:
            mask_early = (np.random.randint(0, 20, size=N, dtype=np.int16)==0)
            if mask_early.any():
                explode_t[mask_early] = 2 * (((np.random.randint(0, 300, size=mask_early.sum(), dtype=np.int16) + 450) / 3.0).astype(np.int16) / v[mask_early]).astype(np.int16)
            if (~mask_early).any():                   
                explode_t[~mask_early] = 2 * (((np.random.randint(0, 300, size=(~mask_early).sum(), dtype=np.int16) + 450) / v[~mask_early]).astype(np.int16))

    s = np.full(shape=(N,M),fill_value=2,dtype=np.int8) # tcs末的状态
    
    for t in splash_t:    s[:, min(t+1,M):min(t+1000,M)] = 1
    
    frozen_times = np.zeros(N, dtype=np.int16)
    frozen_end_times = None
    for t in ice_t:
        mask_normal = (s[:, t-1] == 2) & (s[:, t] == 2)
        if mask_normal.any():  
            if speed_type == 'normal':
                frozen_times[mask_normal] = np.random.randint(399, 600, size=mask_normal.sum(), dtype=np.int16)
            elif speed_type == 'fastest':
                frozen_times[mask_normal] = np.random.randint(399, 400, size=mask_normal.sum(), dtype=np.int16)
            elif speed_type == 'slowest':
                frozen_times[mask_normal] = np.random.randint(599, 600, size=mask_normal.sum(), dtype=np.int16)   # 测试用
        if (~mask_normal).any():  
            if speed_type == 'normal':
                frozen_times[~mask_normal] = np.random.randint(299, 400, size=(~mask_normal).sum(), dtype=np.int16)
            elif speed_type == 'fastest':
                frozen_times[~mask_normal] = np.random.randint(299, 300, size=(~mask_normal).sum(), dtype=np.int16)
            elif speed_type == 'slowest':
                frozen_times[~mask_normal] = np.random.randint(399, 400, size=(~mask_normal).sum(), dtype=np.int16)   # 测试用
        
        ###
        if is_jack:
            mask_delay = (t <= explode_t)
            explode_t[mask_delay] += frozen_times[mask_delay] + 1

        frozen_end_times = np.clip(t + frozen_times, 0 ,M)
        slowdown_end_times = min(t+1999, M)
        for j in range(N):
            s[j, t:frozen_end_times[j]] = 0
            mask = s[j, frozen_end_times[j]:slowdown_end_times] != 0    # 关键错误
            s[j, frozen_end_times[j]:slowdown_end_times][mask] = 1
    del frozen_times, frozen_end_times
    
    ###
    if is_jack:
        if explode_t.max() >= M:    raise ValueError(f"M is too small\n")
    
    speed = s*v[:, np.newaxis] * np.float32(0.5)
    del s
    
    delta_completing_rate = 47*0.00999999977648258 /total_x *speed
    # delta_completing_rate[:, 0] = 47*0.00999999977648258/total_x * v
    del v
    delta_completing_rate = np.concatenate([np.zeros((N, 1)), delta_completing_rate[:, :-1]], axis=1)

    cum_completing_rate = np.cumsum(delta_completing_rate, axis=1)
    del delta_completing_rate
    completing_rate, _ = np.modf(cum_completing_rate)
    del cum_completing_rate
    
    x = np.float32(- 47*0.009999999776482582 / total_x*(1+N_animation)) * speed * x_each_animation[(completing_rate*N_animation).astype(np.int16)]
    del completing_rate, speed

    if not not_flag and x_range[1]-x_range[0] == 40:
        if speed_type == 'normal':
            x[:,0] = np.random.randint(x_range[0]+40, x_range[1]+40, size=N, dtype=np.int16)
        elif speed_type == 'fastest':
            x[:,0] = np.random.randint(x_range[0]+40, x_range[0]+40+1, size=N, dtype=np.int16)
        elif speed_type == 'slowest':
            x[:,0] = np.random.randint(x_range[1]+40-1, x_range[1]+40, size=N, dtype=np.int16)
    else:
        if speed_type == 'normal':
            x[:,0] = np.random.randint(x_range[0], x_range[1], size=N, dtype=np.int16)
        elif speed_type == 'fastest':
            x[:,0] = np.random.randint(x_range[0], x_range[0]+1, size=N, dtype=np.int16)
        elif speed_type == 'slowest':
            x[:,0] = np.random.randint(x_range[1]-1, x_range[1], size=N, dtype=np.int16)

    x = np.cumsum(x, axis=1, dtype=np.float32)
    
    ### return completing_rate
    if is_jack: return x, explode_t
    else:       return x

def generate_zomboni_x(M):
    x = np.zeros(M,dtype=np.float32)
    x[0] = np.float32(800.0)
    for i in range(1,M):
        tmp = x[i-1].astype(int)
        if tmp>700:     dlt_x = np.float32(0.25)
        elif tmp<=400:  dlt_x = np.float32(0.10)
        else:           dlt_x = np.float32(0.0005*tmp - 0.10)
        x[i] = x[i-1] - dlt_x
    return x

class IO:
    def __init__(
        self, co, ty, al, 
        N, M, z_def
    ):
        self.x_atk = 800 - z_def
        self.x_trigger = 800 - z_def

        self.idle = not al

        if ty == 0:     self.type = 'gloom'
        elif ty == 1:   self.type = 'fume'
        elif ty == 2:   self.type = 'melon'
         
        if ty!=2:
            self.x_trigger = min(co*80-40+ PLANT_INFO[self.type].trigger_range - z_def , self.x_atk)
            self.x_atk = min(co*80-40+ PLANT_INFO[self.type].atk_range - z_def , self.x_atk)   
        
        self.t_trigger = self._generate_t_trigger(N, M)
        self.t_atk = self._generate_t_atk(N)
        
        self.dmg = PLANT_INFO[self.type].dmg

    def fix_melon_x_atk(self, splash_s, crush_col, crush_type): # splash_s/px
        self.x_atk = min(
            splash_s + crush_col*80 + (40 if crush_type!=2 else (-40))
            , self.x_atk)

    def _generate_t_trigger(self, N, M):
        max_cd = PLANT_INFO[self.type].max_cd
        
        t0_dist = PLANT_INFO[self.type].t0_dist
        t0_rand = np.random.randint(0, t0_dist[-1], size=N)
        t0_trigger = np.searchsorted(t0_dist, t0_rand, side='right')
        t0_trigger = t0_trigger[:, np.newaxis]   
              
        num_rand = M//max_cd + 3
        t_trigger = np.random.randint(max_cd-14, max_cd+1, size=(N, num_rand), dtype=np.int16)
        
        t_trigger = np.concatenate([t0_trigger-max_cd, t_trigger], axis=1, dtype=np.int16) 
        t_trigger = np.cumsum(t_trigger, axis=1, dtype=np.int16) 
        
        return t_trigger

    def _generate_t_atk(self, N):
        hit_list = PLANT_INFO[self.type].hit_list
        t_atk = self.t_trigger[:,:, np.newaxis] + hit_list 
        t_atk = t_atk.reshape(N, -1)
        return t_atk.astype(np.int16)
