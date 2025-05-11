#include "gaussian_elimination.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <random>
#include <algorithm>
#include <lazycsv.hpp>

namespace GaussianSolver {

Eigen::MatrixXd readAugmentedMatrixFromCSV(const std::string& filename) {
    std::vector<std::vector<double>> rows;
    
    try {
        // Open the file manually first to see if it exists
        std::ifstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open file: " + filename);
        }
        file.close();
        
        // Use lazycsv to parse the CSV file
        lazycsv::parser parser(filename);
        
        // Find the matrix dimensions
        size_t num_rows = 0;
        size_t num_cols = 0;
        bool first_row = true;
        
        // Debug output
        //std::cout << "Reading CSV rows:" << std::endl;
        
        for (const auto& row : parser) {
            std::vector<double> current_row;
            
            for (const auto& cell : row) {
                std::string cell_str = std::string(cell.raw());
                current_row.push_back(std::stod(cell_str));
            }
            
            // Check for consistent column count
            if (first_row) {
                num_cols = current_row.size();
                first_row = false;
            } else if (current_row.size() != num_cols) {
                throw std::runtime_error("Inconsistent number of columns in CSV file: " + filename);
            }
            
            rows.push_back(current_row);
            num_rows++;
        }
        
        if (num_rows == 0 || num_cols == 0) {
            throw std::runtime_error("Empty or invalid matrix in CSV file: " + filename);
        }
        
        // Create the Eigen matrix
        Eigen::MatrixXd matrix(num_rows, num_cols);
        for (size_t i = 0; i < num_rows; ++i) {
            for (size_t j = 0; j < num_cols; ++j) {
                matrix(i, j) = rows[i][j];
            }
        }
        
        return matrix;
    } catch (const std::exception& e) {
        throw std::runtime_error("Error reading CSV file: " + std::string(e.what()));
    }
}

Eigen::VectorXd solve(const Eigen::MatrixXd& augmentedMatrix, double epsilon) {
    // Get dimensions
    int n = augmentedMatrix.rows();
    
    // Validate input matrix - should be augmented matrix [A|b]
    if (augmentedMatrix.cols() != n + 1) {
        throw std::invalid_argument("Augmented matrix should have n+1 columns for n equations");
    }
    
    // Create a mutable copy of the matrix
    Eigen::MatrixXd A = augmentedMatrix;
    
    // Forward Elimination with Partial Pivoting
    for (int k = 0; k < n - 1; ++k) {
        // Find the row with maximum absolute value in column k (partial pivoting)
        int pivot_row = k;
        double max_val = std::abs(A(k, k));
        
        for (int i = k + 1; i < n; ++i) {
            if (std::abs(A(i, k)) > max_val) {
                max_val = std::abs(A(i, k));
                pivot_row = i;
            }
        }
        
        // Swap rows if needed
        if (pivot_row != k) {
            A.row(k).swap(A.row(pivot_row));
        }
        
        // Check for singularity
        if (std::abs(A(k, k)) < epsilon) {
            throw SingularMatrixException("Matrix is singular or ill-conditioned at column " + 
                                           std::to_string(k));
        }
        
        // Perform elimination for all rows below pivot
        for (int i = k + 1; i < n; ++i) {
            double factor = A(i, k) / A(k, k);
            A.row(i) -= factor * A.row(k);
        }
    }
    
    // Check singularity of the last pivot
    if (std::abs(A(n-1, n-1)) < epsilon) {
        throw SingularMatrixException("Matrix is singular or ill-conditioned at the last column");
    }
    
    // Back Substitution
    Eigen::VectorXd solution(n);
    
    for (int i = n - 1; i >= 0; --i) {
        // Calculate the sum of known terms: sum(A(i,j) * solution(j)) for j > i
        double sum = 0.0;
        for (int j = i + 1; j < n; ++j) {
            sum += A(i, j) * solution(j);
        }
        
        // Solve for the current variable
        solution(i) = (A(i, n) - sum) / A(i, i);
    }
    
    return solution;
}

void writeSolutionToCSV(const std::string& filename, const Eigen::VectorXd& solution) {
    std::ofstream file(filename);
    if (!file) {
        throw std::runtime_error("Failed to open file for writing: " + filename);
    }

    file << "solution" << std::endl;
    
    // Set precision for floating-point output
    file << std::fixed << std::setprecision(10);
    
    // Write each element of the solution vector on a new line
    for (int i = 0; i < solution.size(); ++i) {
        file << solution(i) << std::endl;
    }
    
    file.close();
}

Eigen::MatrixXd generateRandomSystem(int num_variables, double min_val, double max_val, 
                                     unsigned int seed) {
    if (num_variables <= 0) {
        throw std::invalid_argument("Number of variables must be positive");
    }
    
    // Initialize random number generator with the provided seed or use a random device
    std::mt19937 gen;
    if (seed == 0) {
        std::random_device rd;
        gen.seed(rd());
    } else {
        gen.seed(seed);
    }
    
    // Create a uniform distribution for the values
    std::uniform_real_distribution<double> distrib(min_val, max_val);
    
    // Create the augmented matrix [A|b]
    Eigen::MatrixXd matrix(num_variables, num_variables + 1);
    
    // Fill with random values
    for (int i = 0; i < num_variables; ++i) {
        for (int j = 0; j <= num_variables; ++j) {
            matrix(i, j) = distrib(gen);
        }
    }
    
    return matrix;
}

void writeMatrixToCSV(const std::string& filename, const Eigen::MatrixXd& matrix) {
    std::ofstream file(filename);
    if (!file) {
        throw std::runtime_error("Failed to open file for writing: " + filename);
    }

    // Add a dynamic dummy header
    if (matrix.cols() > 0) {
        for (int j = 0; j < matrix.cols(); ++j) {
            file << "col" << (j + 1);
            if (j < matrix.cols() - 1) {
                file << ",";
            }
        }
        file << "\n"; // Ensure this is a newline
    }
    
    // Set precision for floating-point output
    file << std::fixed << std::setprecision(10);
    
    // Write each row of the matrix
    for (int i = 0; i < matrix.rows(); ++i) {
        for (int j = 0; j < matrix.cols(); ++j) {
            file << matrix(i, j);
            // Add comma between values, but not after the last value in a row
            if (j < matrix.cols() - 1) {
                file << ",";
            }
        }
        file << std::endl;
    }
    
    file.close();
}

} // namespace GaussianSolver
