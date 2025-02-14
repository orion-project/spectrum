import numpy as np

def print_arr(name, arr):
    print(f"{name}=[{', '.join([f'{r}' for r in arr])}]")

def print_test(func):
    def wrapper(*args, **kwargs):
        print("-------------------------")
        print(func.__name__)
        for i, arg in enumerate(args):
            print_arr(f'inp.{i}', arg)
        res = func(*(np.array(a) for a in args))
        print_arr('res', res)
        print("-------------------------")
    return wrapper

################################################################################

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

################################################################################

# Method 1: Using numpy.gradient (recommended)
@print_test
def DerivativeTests_calc1(x, y):
    d = np.gradient(y, x)
    return d

# Method 2: Using finite differences
@print_test
def DerivativeTests_calc2(x, y):
    dx = np.diff(x)
    dy = np.diff(y)
    # Returns one element less than input
    return dy / dx

# Method 3: Central differences (more accurate)
@print_test
def DerivativeTests_calc3(x, y):
    dx = np.diff(x)
    dy = np.diff(y)
    d = np.zeros(len(x))
    d[0] = dy[0] / dx[0]  # Forward difference for first point
    d[-1] = dy[-1] / dx[-1]  # Backward difference for last point
    # Central difference for middle points
    d[1:-1] = (dy[1:] + dy[:-1]) / (dx[1:] + dx[:-1])
    return d

x = [0, 1, 2, 3, 4]
y = [0, 1, 4, 9, 16]
DerivativeTests_calc1(x, y)
DerivativeTests_calc2(x, y)
DerivativeTests_calc3(x, y)

################################################################################
