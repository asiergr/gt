use ndarrary::prelude::*;

fn gaussian_elimination(A: ndarray::arr2 , b: ndarrary::arr1) -> Vec<f64> {
    let n = A.Dim().0; // We assume A is square
    let mut M = Array::zeros((n,n)); // Multiplier matrix
    
    for k in (1..n) {
        if A[k][k] == 0 {
            panic!("Zero pivot encountered!");
        }
        for i in (k+1..n) {
            M[i][k] = A[i][k] / A[k][k];
        }
        for j in (k=1..n) {
            for i in (k=1..n) {
                A[i][j] = A[i][j] - M[i][k] * A[k][j];
            }
        }
    }
    println!("{A}");
    println!("{M}");
    vec!(0.0)
}

#[cfg(test)]
mod tests {
    use super::*

    #[test]
    fn test_gaussian_elimination() {
        let A = array![[1.0, 1/2, 1/3],
                        [1/2, 1/3, 1/4],
                        [1/3, 1/4, 1/5]];
        let b = array![7/6, 5/6, 13/20];
        
        gaussian_elimination(A, b);

    }
}