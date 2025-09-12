import pandas as pd
import json

file_name = 'zomboni_test_info.xlsx'

###
info_read = pd.read_excel(file_name, usecols='B', skiprows=0, nrows=6, header=None)
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

tmp = info_list[5]
if tmp == "随机":   test_type_plant = 0
elif tmp == "最快": test_type_plant = 1
else:               test_type_plant = 2


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

melon_list = []
info_read = pd.read_excel(file_name, usecols='I:K', skiprows=2, header=None, na_values='')
info_read.dropna(inplace=True)
info_list = info_read.apply(lambda row: row.tolist(), axis=1).tolist()
for tmp in info_list:
    for _ in range(int(tmp[2])):
        a = int(80*(tmp[0]))
        b = 2
        c = tmp[1] == "永动"
        melon_list.append([a, b, c])
###

config_dict = {
    "num_test": test_N,
    "show_progress": show_pro,
    "test_type_plant": test_type_plant,

    "crush_col": crush_col,
    "crush_type": crush_type,

    "shroom_list": [[v[0], v[1], 1 if v[2] else 0] for v in plant_list],
    "melon_list": [[v[0], v[1], 1 if v[2] else 0] for v in melon_list]
}

with open("zomboni_config.json", "w", encoding="utf-8") as f:
    json.dump(config_dict, f, ensure_ascii=False, indent=2)


# t_crush = np.searchsorted(-x_int, -x_crush, side='left') + 1
