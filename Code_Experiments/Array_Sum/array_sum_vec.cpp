#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include "array_sum_ispc.h"

// Serial sum function
float serial_sum(int N, float* array) {
    float sum = 0;
    for (int i = 0; i < N; i++) {
        sum += array[i];
    }
    return sum;
}

int main() {
    const int N = 10000000; // 10 million elements
    
    // Create and fill vector with random values
    std::vector<float> numbers(N);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);
    
    for (int i = 0; i < N; i++) {
        numbers[i] = dis(gen);
    }
    
    // Time serial implementation
    auto start = std::chrono::high_resolution_clock::now();
    float serial_result = serial_sum(numbers.size(), numbers.data());
    auto end = std::chrono::high_resolution_clock::now();
    auto serial_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    
    // Time ISPC implementation
    start = std::chrono::high_resolution_clock::now();
    float ispc_result = ispc::sum_array(numbers.size(), numbers.data());
    end = std::chrono::high_resolution_clock::now();
    auto ispc_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    
    // Print results
    std::cout << "Array size: " << N << " elements\n";
    std::cout << "Serial sum: " << serial_result << " (took " << serial_time << " microseconds)\n";
    std::cout << "ISPC sum:   " << ispc_result << " (took " << ispc_time << " microseconds)\n";
    std::cout << "Difference: " << std::abs(serial_result - ispc_result) << "\n";
    std::cout << "Speedup: " << (float)serial_time / ispc_time << "x\n";
    
    return 0;
}