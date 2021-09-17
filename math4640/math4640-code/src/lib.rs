use ndarray::{ArrayBase, Array1, Array2}; 

fn gaussian_elimination(mut A: Array2<f64>, b: Array1<f64>) -> Vec<f64> {
    let n = A.shape()[0]; // We assume A is square
    let mut M: Array2<f64> = ArrayBase::zeros((n,n)); // Multiplier matrix
    
    for k in 1..n {
        if A[[k,k]] == 0.0 {
            panic!("Zero pivot encountered!");
        }
        for i in k+1..n {
            M[[i,k]] = A[[i,k]] / A[[k,k]];
        }
        for j in k+1..n {
            for i in k+1..n {
                A[[i,j]] = A[[i,j]] - M[[i,j]] * A[[k,j]];
            }
        }
    }
    println!{"{}", A};
    vec!(0.0)
}

#[cfg(test)]
mod tests {
    use super::*;
    use ndarray::array;

    #[test]
    fn test_gaussian_elimination() {
        let A = array![[1.0, 1.0/2.0, 1.0/3.0],
                        [1.0/2.0, 1.0/3.0, 1.0/4.0],
                        [1.0/3.0, 1.0/4.0, 1.0/5.0]];
        let b = array![7.0/6.0, 5.0/6.0, 13.0/20.0];
        
        gaussian_elimination(A, b);
    }
}