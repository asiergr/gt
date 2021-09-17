use ndarray::{ArrayBase, Array1, Array2}; 

fn gaussian_elimination(mut a: Array2<f64>, b: Array1<f64>) -> Vec<f64> {
    let n = a.shape()[0]; // We assume A is square
    let mut m: Array2<f64> = ArrayBase::zeros((n,n)); // Multiplier matrix
    
    for k in 0..n {
        if a[[k,k]] == 0.0 {
            panic!("Zero pivot encountered!");
        }
        for i in k..n {
            m[[i,k]] = a[[i,k]] / a[[k,k]];
        }
        for j in k..n {
            for i in k..n {
                a[[i,j]] = a[[i,j]] - m[[i,k]] * a[[k,j]];
            }
        }
    }
    println!{"{}", m};
    println!{"{}", a};
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