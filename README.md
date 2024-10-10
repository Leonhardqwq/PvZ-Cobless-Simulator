# Jack-in-box-Simulator

# Pop CDF
gene_prob.py可以生成小丑开盒倒计时的离散概率，原理是对小丑开盒倒计时公式，分别遍历随机数0~299、以步频step遍历速度参数0.66~0.68，视频率为概率
jack_in_box_ep7.xlsx是上述程序以step=1e-7得出的概率表，并加入了求和功能，可以计算小丑开盒倒计时在(t1,t2]内的概率
