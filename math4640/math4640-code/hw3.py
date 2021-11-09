import numpy as np
from typing import Callable, Tuple, List
import matplotlib.pyplot as plt
import pandas as pd


def bisection_method(
    interval: Tuple[float, float], fn: Callable[[float], float], real_solution: float
) -> np.array:
    """Bisection method.
    @param interval: the interval considered.
    @param fn: the function we are finding the roots of.
    @param real_solution: the true solution.
    @return: numpy array with the iterations and errors."""
    # Iteration counter
    i = 0
    # where we will store the approximation error
    iter_error = []
    while interval[1] - interval[0] > 10e-6:
        a, b = interval
        m = a + (b - a) / 2
        if np.sign(fn(a)) == np.sign(fn(m)):
            a = m
        else:
            b = m
        # Update interval
        interval = (a, b)
        # Append iteration and error
        iter_error.append((i, abs(real_solution - m)))
        i += 1

    return np.array(iter_error)


def main():
    # Each of these is just the triple of params
    # that we want to pass into the bisection_method function.
    fn_a = lambda x: np.exp(x) - 2
    int_a = (0, 1)
    sol_a = np.log(2)

    fn_b = lambda x: np.power(x, 3) - x - 5
    int_b = (0, 3)
    sol_b = 2.094551482

    fn_c = lambda x: np.power(x, 2) - np.sin(x)
    int_c = [0.1, np.pi]
    sol_c = 0.876726

    # For easier iterating
    fn_tup = [(fn_a, int_a, sol_a), (fn_b, int_b, sol_b), (fn_c, int_c, sol_c)]

    for fn, interval, sol in fn_tup:
        data = bisection_method(interval, fn, sol)
        df = pd.DataFrame(data)
        df.plot(x=0, y=1, kind="scatter", logx=True, logy=True)


if __name__ == "__main__":
    main()
