import numpy as np

def print_arr(name, arr):
    print(f"{name}=[{', '.join([f'{r}' for r in arr])}]")

def print_test(func):
    def wrapper(*args, **kwargs):
        print("-------------------------")
        print(func.__name__)
        print_arr('inp', args[0])
        res = func(np.array(args[0]))
        print_arr('res', res)
        print("-------------------------")
    return wrapper

@print_test
def MovingAverageTests_simple(inp):
    window_size = 6
    weights = np.ones(window_size) / window_size
    return np.convolve(inp, weights, mode='valid')

@print_test
def MovingAverageTests_cumul(inp):
    return np.cumsum(inp) / np.arange(1, len(inp) + 1)

@print_test
def MovingAverageTests_exp(inp):
    alpha = 0.5
    res = np.zeros(len(inp))
    res[0] = inp[0]
    for i in range(1, len(inp)):
        res[i] = alpha * inp[i] + (1 - alpha) * res[i-1]
    return res

inp = [10, 15, 10, 30, 20, 45, 70, 50, 40, 60]
MovingAverageTests_simple(inp)
MovingAverageTests_cumul(inp)
MovingAverageTests_exp(inp)
