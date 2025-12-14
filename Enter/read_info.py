# -*- coding: utf-8 -*-
import pandas as pd
import json

file_name = 'enter_test_info.xlsx'

###
info_read = pd.read_excel(file_name, usecols='B', skiprows=0, nrows=21,header=None)
info_list = info_read.iloc[:,0].tolist()

tmp = info_list[5]
if tmp == "通常波":   not_flag = True
else:               not_flag = False

extra_dmg = int(info_list[6])
melon = int(info_list[7])

tmp = info_list[8]
if tmp == '杆': zombie_type = 0
elif tmp == '报': zombie_type = 1
elif tmp == '门': zombie_type = 2
elif tmp == '橄': zombie_type = 3
elif tmp == '潜': zombie_type = 4
elif tmp == '车': zombie_type = 5
elif tmp == '豚': zombie_type = 6
elif tmp == '丑': zombie_type = 7
elif tmp == '气': zombie_type = 8
elif tmp == '矿': zombie_type = 9
elif tmp == '跳': zombie_type = 10
elif tmp == '梯': zombie_type = 11
elif tmp == '篮': zombie_type = 12
elif tmp == '白': zombie_type = 13
elif tmp == '红': zombie_type = 14
elif tmp == '篮球投率': zombie_type = 15


test_N = int(info_list[9])  # a * 10**n
show_pro = info_list[10] == "是"

# vulnerable_time = int(info_list[12])
M_sup = int(info_list[13])

tmp = info_list[14]
if tmp == "随机":   test_type_zombie = 0
elif tmp == "最快": test_type_zombie = 1
else:               test_type_zombie = 2

tmp = info_list[15]
if tmp == "随机":   test_type_plant = 0
elif tmp == "最快": test_type_plant = 1
else:               test_type_plant = 2



###

plant_list = []
info_read = pd.read_excel(file_name, usecols='D:G', skiprows=2,header=None,na_values='')
info_read.dropna(inplace=True)
info_list = info_read.apply(lambda row: row.tolist(), axis=1).tolist()
for tmp in info_list:
    for _ in range(int(tmp[3])):
        a = int(tmp[0])
        b = 0 if tmp[1]=="曾" else 1
        c = tmp[2]=="永动"
        plant_list.append([a,b,c])

ice_t = []
info_read = pd.read_excel(file_name, usecols='I', skiprows=2,header=None,na_values='')
info_read.dropna(inplace=True)
info_list = info_read.iloc[:,0].tolist()
for info in info_list:
    ice_t.append(int(info))

splash_t = []
info_read = pd.read_excel(file_name, usecols='K', skiprows=2,header=None,na_values='')
info_read.dropna(inplace=True)
info_list = info_read.iloc[:,0].tolist()
for info in info_list:
    splash_t.append(int(info))

ash_infos = []
info_read = pd.read_excel(file_name, usecols='M:P', skiprows=2,header=None,na_values='')
info_read.dropna(inplace=True)
info_list = info_read.apply(lambda row: row.tolist(), axis=1).tolist()
for tmp in info_list:
    ash_time = int(tmp[0])
    ash_type_card = 1 if tmp[1]=="卡" else 0
    ash_range_left = int(tmp[2])
    ash_range_right = int(tmp[3])
    ash_infos.append([ash_time, ash_type_card, ash_range_left, ash_range_right])

config_dict = {
  "num_test": test_N, 
  "show_progress": show_pro,
  "type": zombie_type,
  "test_type_zombie": test_type_zombie,
  "test_type_plant": test_type_plant,
  "hugewave": not not_flag,
  "M": M_sup,

  "ice_t": ice_t,
  "splash_t": splash_t,
  "melon": melon,
  "extra_dmg": extra_dmg,

  "ash_infos": ash_infos,

  "plant_list": [[v[0], v[1], 1 if v[2] else 0] for v in plant_list]
}

with open("enter_config.json", "w", encoding="utf-8") as f:
    json.dump(config_dict, f, ensure_ascii=False, indent=2)

