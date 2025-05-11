#include "../include/gaussian_elimination.hpp"
#include <gtest/gtest.h>
#include <fstream>
#include <cmath>
#include <random>
#include <filesystem>

namespace fs = std::filesystem;

// Utility functions for tests
namespace {

// Check if two vectors are approximately equal
bool areVectorsClose(const Eigen::VectorXd& v1, const Eigen::VectorXd& v2, double tolerance = 1e-8) {
    if (v1.size() != v2.size()) {
        return false;
    }
    return (v1 - v2).cwiseAbs().maxCoeff() < tolerance;
}

// Create a temporary CSV file with the specified content
std::string createTempCSVFile(const std::vector<std::vector<double>>& data) {
    // Create a temporary filename
    std::string filename = "test_matrix_" + std::to_string(
        std::chrono::system_clock::now().time_since_epoch().count()) + ".csv";
    
    std::ofstream file(filename);
    if (!file) {
        throw std::runtime_error("Failed to create temporary file: " + filename);
    }
    
    // Add a dynamic dummy header for lazycsv
    if (!data.empty() && !data[0].empty()) {
        for (size_t i = 0; i < data[0].size(); ++i) {
            file << "col" << (i + 1);
            if (i < data[0].size() - 1) {
                file << ",";
            }
        }
        file << '\n';
    } else {
        // If data is empty or first row is empty, no header is written.
    }

    for (const auto& row : data) {
        for (size_t i = 0; i < row.size(); ++i) {
            file << row[i];
            if (i < row.size() - 1) {
                file << ",";
            }
        }
        file << '\n';
    }
    
    file.close();
    return filename;
}

// Delete a temporary file
void deleteTempFile(const std::string& filename) {
    try {
        fs::remove(filename);
    } catch (...) {
        // Ignore deletion errors in tests
    }
}

} // anonymous namespace

class GaussianEliminationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Nothing special to set up
    }
    
    void TearDown() override {
        // Clean up temp files if any
    }
    
    // Helper to create a simple 2x2 system with known solution
    Eigen::MatrixXd createSimpleSystem() {
        // 2x + y = 5
        // x + 3y = 10
        // Solution: x = 1, y = 3
        Eigen::MatrixXd matrix(2, 3);
        matrix << 2, 1, 5,
                  1, 3, 10;
        return matrix;
    }
    
    // Helper to create a system that requires pivoting
    Eigen::MatrixXd createPivotingSystem() {
        // 0.001x + y = 1
        // x + y = 2
        // Solution: x = 1, y = 1
        Eigen::MatrixXd matrix(2, 3);
        matrix << 0.001, 1, 1,
                  1, 1, 2;
        return matrix;
    }
    
    // Helper to create a singular matrix
    Eigen::MatrixXd createSingularSystem() {
        // x + y = 2
        // x + y = 3
        // No solution (inconsistent) or infinitely many solutions if both = 2
        Eigen::MatrixXd matrix(2, 3);
        matrix << 1, 1, 2,
                  1, 1, 3;
        return matrix;
    }
};

// Test reading a valid CSV file
TEST_F(GaussianEliminationTest, ReadValidCSV) {
    std::vector<std::vector<double>> data = {
        {1.0, 2.0, 3.0},
        {4.0, 5.0, 6.0}
    };
    
    std::string filename = createTempCSVFile(data);
    
    try {
        Eigen::MatrixXd matrix = GaussianSolver::readAugmentedMatrixFromCSV(filename);
        
        EXPECT_EQ(matrix.rows(), 2);
        EXPECT_EQ(matrix.cols(), 3);
        EXPECT_DOUBLE_EQ(matrix(0, 0), 1.0);
        EXPECT_DOUBLE_EQ(matrix(0, 1), 2.0);
        EXPECT_DOUBLE_EQ(matrix(0, 2), 3.0);
        EXPECT_DOUBLE_EQ(matrix(1, 0), 4.0);
        EXPECT_DOUBLE_EQ(matrix(1, 1), 5.0);
        EXPECT_DOUBLE_EQ(matrix(1, 2), 6.0);
    } catch(const std::exception& e) {
        deleteTempFile(filename);
        FAIL() << "Exception thrown: " << e.what();
    }
    
    deleteTempFile(filename);
}

// Test reading an invalid CSV file
TEST_F(GaussianEliminationTest, ReadInvalidCSV) {
    // File doesn't exist
    EXPECT_THROW(GaussianSolver::readAugmentedMatrixFromCSV("nonexistent_file.csv"), std::runtime_error);
    
    // Inconsistent columns
    std::vector<std::vector<double>> data = {
        {1.0, 2.0, 3.0},
        {4.0, 5.0} // Missing a column
    };
    
    std::string filename = createTempCSVFile(data);
    
    EXPECT_THROW(GaussianSolver::readAugmentedMatrixFromCSV(filename), std::runtime_error);
    
    deleteTempFile(filename);
}

// Test solving a simple system
TEST_F(GaussianEliminationTest, SolveSimpleSystem) {
    Eigen::MatrixXd matrix = createSimpleSystem();
    Eigen::VectorXd expected_solution(2);
    expected_solution << 1.0, 3.0;
    
    Eigen::VectorXd solution = GaussianSolver::solve(matrix);
    
    EXPECT_TRUE(areVectorsClose(solution, expected_solution));
}

// Test solving a system that requires pivoting
TEST_F(GaussianEliminationTest, SolvePivotingSystem) {
    Eigen::MatrixXd matrix = createPivotingSystem();
    Eigen::VectorXd expected_solution(2);
    // Using more precise expected values for the pivoting system
    // x = 1000/999, y = 998/999
    expected_solution << 1000.0/999.0, 998.0/999.0;
    
    Eigen::VectorXd solution = GaussianSolver::solve(matrix);
    
    EXPECT_TRUE(areVectorsClose(solution, expected_solution));
}

// Test solving a singular system - should throw an exception
TEST_F(GaussianEliminationTest, SolveSingularSystem) {
    Eigen::MatrixXd matrix = createSingularSystem();
    
    EXPECT_THROW(GaussianSolver::solve(matrix), GaussianSolver::SingularMatrixException);
}

// Test writing and reading solution vector
TEST_F(GaussianEliminationTest, WriteSolutionToCSV) {
    Eigen::VectorXd solution(3);
    solution << 1.0, 2.0, 3.0;
    
    std::string filename = "test_solution.csv";
    
    try {
        GaussianSolver::writeSolutionToCSV(filename, solution);
        
        // Read the CSV back manually and verify
        std::ifstream file(filename);
        ASSERT_TRUE(file.is_open());
        
        std::vector<double> values;
        std::string line;

        // Skip the header line
        std::getline(file, line);
        
        while (std::getline(file, line)) {
            values.push_back(std::stod(line));
        }
        
        ASSERT_EQ(values.size(), 3);
        EXPECT_DOUBLE_EQ(values[0], 1.0);
        EXPECT_DOUBLE_EQ(values[1], 2.0);
        EXPECT_DOUBLE_EQ(values[2], 3.0);
        
        file.close();
    } catch(const std::exception& e) {
        deleteTempFile(filename);
        FAIL() << "Exception thrown: " << e.what();
    }
    
    deleteTempFile(filename);
}

// Test generating a random system
TEST_F(GaussianEliminationTest, GenerateRandomSystem) {
    int size = 5;
    unsigned int seed = 42;
    
    Eigen::MatrixXd matrix = GaussianSolver::generateRandomSystem(size, -1.0, 1.0, seed);
    
    EXPECT_EQ(matrix.rows(), size);
    EXPECT_EQ(matrix.cols(), size + 1);
    
    // Verify reproducibility with the same seed
    Eigen::MatrixXd matrix2 = GaussianSolver::generateRandomSystem(size, -1.0, 1.0, seed);
    
    // Both matrices should be identical with the same seed
    EXPECT_TRUE((matrix - matrix2).cwiseAbs().maxCoeff() < 1e-10);
}

// Test end-to-end: generate, solve, and verify
TEST_F(GaussianEliminationTest, RandomSystemSolveVerify) {
    int size = 10;
    unsigned int seed = 123;
    
    // Generate random system
    Eigen::MatrixXd augmentedMatrix = GaussianSolver::generateRandomSystem(size, -10.0, 10.0, seed);
    
    // Solve the system
    Eigen::VectorXd solution = GaussianSolver::solve(augmentedMatrix);
    
    // Verify Ax = b
    Eigen::MatrixXd A = augmentedMatrix.leftCols(size);
    Eigen::VectorXd b = augmentedMatrix.col(size);
    Eigen::VectorXd b_calculated = A * solution;
    
    // Check that the calculated b is close to the original b
    EXPECT_TRUE(areVectorsClose(b, b_calculated, 1e-6));
}

// Test writing a matrix to CSV
TEST_F(GaussianEliminationTest, WriteMatrixToCSV) {
    Eigen::MatrixXd matrix(2, 3);
    matrix << 1.0, 2.0, 3.0,
              4.0, 5.0, 6.0;
              
    std::string filename = "test_matrix.csv";
    
    try {
        GaussianSolver::writeMatrixToCSV(filename, matrix);
        
        // Read back and verify
        Eigen::MatrixXd readMatrix = GaussianSolver::readAugmentedMatrixFromCSV(filename);
        
        EXPECT_EQ(readMatrix.rows(), matrix.rows());
        EXPECT_EQ(readMatrix.cols(), matrix.cols());
        
        for (int i = 0; i < matrix.rows(); ++i) {
            for (int j = 0; j < matrix.cols(); ++j) {
                EXPECT_NEAR(readMatrix(i, j), matrix(i, j), 1e-8);
            }
        }
    } catch(const std::exception& e) {
        deleteTempFile(filename);
        FAIL() << "Exception thrown: " << e.what();
    }
    
    deleteTempFile(filename);
}
