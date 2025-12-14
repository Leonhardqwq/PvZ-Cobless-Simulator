import pandas as pd
import json

file_name = 'jack_test_info.xlsx'

###
info_read = pd.read_excel(file_name, usecols='B', skiprows=0, nrows=16,header=None)
info_list = info_read.iloc[:,0].tolist()

tmp = info_list[0]
if tmp == "全部":   jack_type = 0
elif tmp == "早爆": jack_type = 1
else:               jack_type = 2

tmp = info_list[1]
if tmp == "前院":     scene = 0
elif tmp == "后院":   scene = 1
else:                 scene = 2

tmp = info_list[2]
if tmp == "上炸下":     boom_type = 0
elif tmp == "下炸上":   boom_type = 1
else:                  boom_type = 2

boom_col = int(info_list[3])

tmp = info_list[4]
if tmp == "普通":     boom_plant_type = 0
elif tmp == "南瓜":   boom_plant_type = 1
else:                 boom_plant_type = 2

tmp = info_list[5]
if tmp == "通常波":   not_flag = True
else:               not_flag = False

melon = int(info_list[7])
test_N = int(info_list[9])  # a * 10**n
show_pro = info_list[10] == "是"

vulnerable_time = int(info_list[12])
# M_sup = int(info_list[13])

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
  "test_type_zombie": test_type_zombie,
  "test_type_plant": test_type_plant,
  "hugewave": not not_flag,

  "scene": scene,
  "boom_type": boom_type,
  "boom_col": boom_col,
  "defense_type": boom_plant_type,

  "jack_type": jack_type,

  "ice_t": ice_t,
  "splash_t": splash_t,
  "melon": melon,

  "ash_infos": ash_infos,
  "vulnerable_time": vulnerable_time,

  "plant_list": [[v[0], v[1], 1 if v[2] else 0] for v in plant_list]
}

with open("jack_config.json", "w", encoding="utf-8") as f:
    json.dump(config_dict, f, ensure_ascii=False, indent=2)

