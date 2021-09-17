import numpy as np

def gaussian_elimination(A: np.array, b: np.array) -> np.array:
    n = A.shape[0]
    M = np.zeros((n,n))

    for k in range(0, n-1):
        if A[k][k] == 0.0:
            return None
        
        for i in range(k, n):
            M[i][k] = A[i][k] / A[k][k]
        
        for j in range(k, n):
            for i in range(k, n):
                A[i][j] = A[i][j] - M[i][k]*A[k][j]
    
    print(M)
    print(A)

def cond_num(A: np.array):
    return np.linalg.cond(A)

def main():
    A = np.array([[1, 1/2, 1/3],
                    [1/2, 1/3, 1/4],
                    [1/3, 1/4, 1/5]])
    gaussian_elimination(A, np.zeros(1))

if __name__ == "__main__":
    main()