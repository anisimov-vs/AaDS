#include "gaussian_elimination.hpp"
#include <iostream>
#include <string>
#include <cstring>
#include <chrono>

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " [options]\n"
              << "Options:\n"
              << "  --input <file>      Input CSV file containing the augmented matrix [A|b]\n"
              << "  --output <file>     Output CSV file to write the solution vector\n"
              << "  --generate <N>      Generate a random system of N equations\n"
              << "  --seed <S>          Seed for random number generator (default: current time)\n"
              << "  --min <val>         Minimum value for random coefficients (default: -10.0)\n"
              << "  --max <val>         Maximum value for random coefficients (default: 10.0)\n"
              << "  --matrix-out <file> Save the generated matrix to this file (only with --generate)\n"
              << "  --help              Display this help message\n";
}

int main(int argc, char* argv[]) {
    // Default values
    std::string inputFile;
    std::string outputFile = "solution.csv";
    std::string matrixOutputFile;
    int generateSize = 0;
    unsigned int seed = static_cast<unsigned int>(
        std::chrono::high_resolution_clock::now().time_since_epoch().count());
    double minVal = -10.0;
    double maxVal = 10.0;
    
    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--help") == 0) {
            printUsage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "--input") == 0 && i + 1 < argc) {
            inputFile = argv[++i];
        } else if (strcmp(argv[i], "--output") == 0 && i + 1 < argc) {
            outputFile = argv[++i];
        } else if (strcmp(argv[i], "--generate") == 0 && i + 1 < argc) {
            generateSize = std::stoi(argv[++i]);
        } else if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc) {
            seed = static_cast<unsigned int>(std::stoul(argv[++i]));
        } else if (strcmp(argv[i], "--min") == 0 && i + 1 < argc) {
            minVal = std::stod(argv[++i]);
        } else if (strcmp(argv[i], "--max") == 0 && i + 1 < argc) {
            maxVal = std::stod(argv[++i]);
        } else if (strcmp(argv[i], "--matrix-out") == 0 && i + 1 < argc) {
            matrixOutputFile = argv[++i];
        } else {
            std::cerr << "Unknown option: " << argv[i] << std::endl;
            printUsage(argv[0]);
            return 1;
        }
    }
    
    // Check if we have a valid mode (input file or generate)
    if (inputFile.empty() && generateSize <= 0) {
        std::cerr << "Error: Either --input or --generate must be specified." << std::endl;
        printUsage(argv[0]);
        return 1;
    }
    
    try {
        Eigen::MatrixXd augmentedMatrix;
        
        // Either read from file or generate random system
        if (!inputFile.empty()) {
            std::cout << "Reading augmented matrix from: " << inputFile << std::endl;
            augmentedMatrix = GaussianSolver::readAugmentedMatrixFromCSV(inputFile);
            
            // Debug output
            std::cout << "Raw matrix dimensions: " << augmentedMatrix.rows() << "x" 
                      << augmentedMatrix.cols() << std::endl;
            std::cout << "Matrix content:\n" << augmentedMatrix << std::endl;
        } else {
            std::cout << "Generating random " << generateSize << "x" << (generateSize + 1) 
                      << " augmented matrix with seed: " << seed << std::endl;
            augmentedMatrix = GaussianSolver::generateRandomSystem(generateSize, minVal, maxVal, seed);
            
            // Save the generated matrix if requested
            if (!matrixOutputFile.empty()) {
                std::cout << "Saving generated matrix to: " << matrixOutputFile << std::endl;
                GaussianSolver::writeMatrixToCSV(matrixOutputFile, augmentedMatrix);
            }
        }
        
        // Display matrix dimensions
        std::cout << "Matrix dimensions: " << augmentedMatrix.rows() << "x" 
                  << augmentedMatrix.cols() << std::endl;
        
        // Solve the system
        std::cout << "Solving system using Gaussian Elimination..." << std::endl;
        auto startTime = std::chrono::high_resolution_clock::now();
        
        Eigen::VectorXd solution = GaussianSolver::solve(augmentedMatrix);
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        
        std::cout << "Solution found in " << duration.count() << " ms" << std::endl;
        
        // Write solution to output file
        std::cout << "Writing solution to: " << outputFile << std::endl;
        GaussianSolver::writeSolutionToCSV(outputFile, solution);
        
        // Verify result - calculate A*x and check against b
        int n = augmentedMatrix.rows();
        Eigen::MatrixXd A = augmentedMatrix.leftCols(n);
        Eigen::VectorXd b = augmentedMatrix.col(n);
        Eigen::VectorXd b_calculated = A * solution;
        
        double maxError = (b - b_calculated).cwiseAbs().maxCoeff();
        std::cout << "Maximum residual error: " << maxError << std::endl;
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
