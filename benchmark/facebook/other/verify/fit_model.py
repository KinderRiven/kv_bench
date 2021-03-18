import matplotlib.pyplot as plt
import pdb

def cut_interval(total_keys = 50000000, range_num = 30):
    ret = []
    range_size = total_keys / range_num
    start = 0
    cur_range = 0
    while cur_range < range_num:
        ret.append(start)
        start += range_size
        cur_range += 1
    ret.append(total_keys+1)
    return ret;

def find_range(key, cutted_range):
    i = 0
    sz = len(cutted_range)
    while i < sz-1:
        if cutted_range[i] <= key and key < cutted_range[i+1]:
            return i
        else:
            i += 1

def get_statics(filename, cut, ans_mode):
    statics_ans = [0 for i in range(len(cut)-1)]
    idx_ans = [i for i in range(len(cut)-1)]
    count = 0
    with open(filename, "r") as file:
        while True:
            key = file.readline()
            #print(key)
            count += 1
            if count % 100000 == 0:
                print(count)
            if not key:
                break
            else:
                key = key.replace('\n', '')
                key = int(key)
                if ans_mode == "scan":
                    idx = find_range(key, cut)
                    #print(idx)
                    statics_ans[idx] += 1
                elif ans_mode == "add":
                    statics_ans[key] += 1
    file.close()
    return idx_ans, statics_ans

def fit_key_model(filename, total_keys, keyrange_num, sort):
    cut = cut_interval(total_keys, keyrange_num)
    print(cut)
    x, y = get_statics(filename, cut, "scan")
    if sort:
        y.sort()
    print(y)
    plt.plot(x, y)
    plt.ylim(ymin=0)
    plt.show()

def fit_value_size_model(filename, sort):
    cut = [i for i in range(1,1001)]
    x, y= get_statics(filename, cut, "add")
    print(y)
    if sort:
        y.sort()
    plt.plot(x, y)
    plt.show()

def fit_model(type="key"):
    if type == "key":
        fit_key_model("../test/key.txt", 50000000, 30, True)
    elif type == "value_size":
        fit_value_size_model("value_size.txt", False)

if __name__ == "__main__":
   fit_model("key")



