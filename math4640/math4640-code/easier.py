from typing import Tuple
import numpy as np

def gaussian_elimination(A: np.array) -> Tuple[np.array, np.array]:
    n = A.shape[0]
    L = np.eye(n,n)

    for k in range(0, n - 1): #Loop over cols
        if A[k][k] == 0:
            return None #if pivot is 0 return
        
        for i in range(k + 1, n): # compute multipliers
            L[i][k] = A[i][k] / A[k][k]

        for j in range(k + 1, n):
            for i in range(k + 1, n):
                A[i][j] = A[i][j] - L[i][k]*A[k][j]
    
    return L, np.triu(A)

def cond_num(A: np.array):
    return np.linalg.cond(A)

def main():
    A = np.array([[1, 1/2, 1/3],
                    [1/2, 1/3, 1/4],
                    [1/3, 1/4, 1/5]])
    b = np.array([7/6, 5/6, 13/20])
    L, U = gaussian_elimination(A)
    print(L @ U)

if __name__ == "__main__":
    main()