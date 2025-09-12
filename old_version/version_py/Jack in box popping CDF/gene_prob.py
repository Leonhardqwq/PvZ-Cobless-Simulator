import numpy as np
import pandas as pd

m = 2270
def analyze(v):
    if v>0.68 or v<0.66:
        print("Wrong")
        return
    p = np.zeros(m)
    for i in range(0,300):
        t1 = 2*int(int((i+450)/3)/v)
        t2 = 2*int((i+450)/v)
        p[t1] = p[t1] + 1
        p[t2] = p[t2] + 19
    return p

def analyze_all(step):
    v = 0.66
    cnt = 0
    p = np.zeros(m)
    while v<=0.68:
        tmp = analyze(v)
        p = p +tmp
        cnt = cnt+1
        v= v+step
    p = p/6000/cnt
    return p

arr = analyze_all(0.0000001)

df = pd.DataFrame({
    't': [i for i in range(len(arr)) if arr[i] > 0],
    'f': [arr[i] for i in range(len(arr)) if arr[i] > 0]
})

# Save the DataFrame to an Excel file
df.to_excel('jack_in_box.xlsx', index=False)
