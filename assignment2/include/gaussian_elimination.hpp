#ifndef GAUSSIAN_ELIMINATION_HPP
#define GAUSSIAN_ELIMINATION_HPP

#include <Eigen/Dense>
#include <string>
#include <stdexcept>

namespace GaussianSolver {

/**
 * Custom exception for singular matrices or systems with no unique solution
 */
class SingularMatrixException : public std::runtime_error {
public:
    explicit SingularMatrixException(const std::string& message)
        : std::runtime_error(message) {}
};

/**
 * @brief Reads an augmented matrix [A|b] from a CSV file.
 * 
 * @param filename Path to the CSV file
 * @return Eigen::MatrixXd The augmented matrix
 * @throws std::runtime_error if file cannot be opened or format is invalid
 */
Eigen::MatrixXd readAugmentedMatrixFromCSV(const std::string& filename);

/**
 * @brief Solves a system of linear equations Ax = b using Gaussian elimination
 *        with partial pivoting.
 * 
 * @param augmentedMatrix An N x (N+1) Eigen matrix representing [A|b]
 * @param epsilon A small value to check for near-zero pivots
 * @return Eigen::VectorXd The solution vector x
 * @throws SingularMatrixException if the system is singular or has no unique solution
 */
Eigen::VectorXd solve(const Eigen::MatrixXd& augmentedMatrix, double epsilon = 1e-10);

/**
 * @brief Writes a solution vector to a CSV file.
 * 
 * @param filename Path to the output CSV file
 * @param solution The solution vector to write
 * @throws std::runtime_error if the file cannot be opened/written
 */
void writeSolutionToCSV(const std::string& filename, const Eigen::VectorXd& solution);

/**
 * @brief Generates a random augmented matrix [A|b].
 * 
 * @param num_variables Number of variables (and equations)
 * @param min_val Minimum value for random coefficients
 * @param max_val Maximum value for random coefficients
 * @param seed Seed for the random number generator
 * @return Eigen::MatrixXd The randomly generated augmented matrix
 */
Eigen::MatrixXd generateRandomSystem(int num_variables, 
                                     double min_val = -10.0, 
                                     double max_val = 10.0,
                                     unsigned int seed = 0);

/**
 * @brief Writes an augmented matrix to a CSV file
 * 
 * @param filename Path to the output CSV file
 * @param matrix The matrix to write
 * @throws std::runtime_error if the file cannot be opened/written
 */
void writeMatrixToCSV(const std::string& filename, const Eigen::MatrixXd& matrix);

} // namespace GaussianSolver

#endif // GAUSSIAN_ELIMINATION_HPP
