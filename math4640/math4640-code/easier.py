from typing import Tuple
import numpy as np

def gaussian_elimination(A: np.array) -> Tuple[np.array, np.array]:
    n = A.shape[0]

    for k in range(0, n - 1): #Loop over cols
        assert A[k][k]
        for i in range(k + 1, n): # compute multipliers
            A[i][k] = A[i][k] / A[k][k]
        for j in range(k + 1, n):
            for i in range(k + 1, n):
                A[i][j] = A[i][j] - A[i][k]*A[k][j]
    L = A - np.triu(A) + np.eye(n,n)
    return L, np.triu(A)

def forward_substitution(L: np.array, b: np.array) -> np.array:
    n = L.shape[0]
    x = np.zeros((n, 1))
    for j in range(0, n):
        assert L[j][j]
        x[j] = b[j] / L[j][j]
        for i in range(j + 1, n):
            b[i] = b[i] - L[i][j] * x[j]
    return x

def backward_substitution(U: np.array, y: np.array) -> np.array:
    n = U.shape[0]
    x = np.zeros((n, 1))
    for j in range(n - 1, -1, -1):
        assert U[j][j]
        x[j] = y[j] / U[j][j]
        for i in range(0, j):
            y[i] = y[i] - U[i][j] * x[j]
    return x

def main():
    A = np.array([[1, 1/2, 1/3],
                    [1/2, 1/3, 1/4],
                    [1/3, 1/4, 1/5]])
    b = np.array([7/6, 5/6, 13/20])
    L, U = gaussian_elimination(A.copy())
    print(L @ U)
    y = forward_substitution(L, b)
    x = backward_substitution(U, y)
    print(x)
    breakpoint()
    r = b.reshape(3,1) - (A @ x)
    print(r)

if __name__ == "__main__":
    main()