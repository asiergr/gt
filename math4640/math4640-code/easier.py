import numpy as np

def cond_num(A: np.array):
    return np.linalg.cond(A)

def main():
    A = np.array([[0.0001, 1],[1,1]])
    print(cond_num(A))

if __name__ == "__main__":
    main()