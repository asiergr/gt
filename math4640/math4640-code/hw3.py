import numpy as np
from typing import Callable, Tuple, List
import matplotlib.pyplot as plt


def bisection_method(
    interval: Tuple[float, float], fn: Callable[[float], float], real_solution: float
) -> List[Tuple[int, float]]:
    i = 0
    iter_error = []
    while interval[1] - interval[0] > 10e-6:
        a, b = interval
        m = a + (b - a) / 2
        if np.sign(fn(a)) == np.sign(fn(m)):
            a = m
        else:
            b = m
        interval = (a, b)
        iter_error.append((i, abs(real_solution - m)))

    return interval


def main():
    fn_a = lambda x: np.exp(x) - 2
    int_a = (0, 1)

    fn_b = lambda x: np.power(x, 3) - x - 5
    int_b = (0, 3)

    fn_c = lambda x: np.power(x, 2) - np.sin(x)
    int_c = [0.1, np.pi]

    fn_tup = [(fn_a, int_a), (fn_b, int_b), (fn_c, int_c)]

    for fn, interval in fn_tup:
        pass
