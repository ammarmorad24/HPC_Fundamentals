#include <math.h>
#include <iostream>
#include <thread>
#include <vector>
#include <random>
#include <chrono>
#include "taylor_vector_ispc.h"

using namespace std;

/*  
    this is an AI Naive implementation of the sinx function
    it is not used in this code
    to notice that parallelism isn't a cover for bad algorithms
    this function is very slow and will take a lot of time to finish

    void sinx_ai(int N, int terms, double* x, double* y)
    {
        for (int i = 0; i < N; i++)
        {
            double sum = 0;

            for (int j = 0; j < terms; j++)
            {
                sum += pow(-1, j) * pow(x[i], 2 * j + 1) / tgamma(2 * j + 1);
            }

            y[i] = sum;
        }
            
    } 
*/

// this is a fast implementation of the sinx function
// it is based from Standford University CS149 course - Parallel Computing

void sinx(int N , int terms, double* x, double* y)
{
    for (int i = 0; i < N; i++)
    {
        double value = x[i];
        double numerator = x[i] * x[i] * x[i];
        double denominator = 6;
        int sign = -1;
        
        for (int j = 1; j <= terms; j++)
        {
            value += sign * numerator / denominator;
            numerator *= x[i] * x[i];
            denominator *= (2 * j + 2) * (2 * j + 3);
            sign *= -1;
        }

        y[i] = value;
    }
}

/*
    Why use a struct?
    In C++, threads can only pass a single argument to their entry function. 
    The struct groups all necessary parameters into one object, s
    implifying thread creation.
*/
typedef struct{
    int n;
    int terms;
    double* x;
    double* y;

} taylor_args;


void my_thread_fn(taylor_args *t)
{
    //sinx(t->n, t->terms, t->x, t->y);
    ispc::sinx_ispc(t->n, t->terms, t->x, t->y);
}

// Parallelism with ThreadPool
void taylor_parallel(int n, int terms, double* x, double* y) {
    // Get available threads and allow override
    const int available_threads = thread::hardware_concurrency();
    const int num_threads = 2 * available_threads; // You can modify this line to use more threads
    
    cout << "Running with " << num_threads << " threads (Hardware supports: " 
         << available_threads << " threads)" << endl;
    
    vector<thread> thread_pool(num_threads);
    vector<taylor_args> args(num_threads);

    
    // Calculate chunk size for each thread
    int chunk_size = n / num_threads;
    int remainder = n % num_threads;
    
    // Start position for each thread's data
    int start_pos = 0;
    
    // Create and launch threads
    for(int i = 0; i < num_threads; i++) {
        // Handle remaining elements in last chunk
        int current_chunk = chunk_size + (i == num_threads-1 ? remainder : 0);
        
        args[i].n = current_chunk;
        args[i].terms = terms;
        args[i].x = x + start_pos;
        args[i].y = y + start_pos;
        
        thread_pool[i] = thread(my_thread_fn, &args[i]);
        start_pos += current_chunk;
    }
    
    // Wait for all threads to complete
    for(auto& t : thread_pool) {
        t.join();
    }
}


void taylor_serial(int n, int terms, double* x, double* y) {
    sinx(n, terms, x, y);
}

void taylor_vector(int n, int terms, double* x, double* y) {
    ispc::sinx_ispc(n, terms, x, y);
}

int main() {
    int n = 100000000;
    int terms = 10;

    vector<double> x(n);
    // Generate test data
    random_device rd;
    mt19937 gen(rd());
    normal_distribution<> d(M_PI, 1);
    for (int i = 0; i < n; i++) {
        x[i] = d(gen);
    }

    vector<double> y_serial(n);
    vector<double> y_vector(n);
    vector<double> y_parallel(n);

    // Test 1: Serial version
    cout << "\nRunning serial version..." << endl;
    auto start_serial = chrono::high_resolution_clock::now();
    taylor_serial(n, terms, x.data(), y_serial.data());
    auto end_serial = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed_serial = end_serial - start_serial;

    // Test 2: ISPC Vector-only version
    cout << "\nRunning vectorized version..." << endl;
    auto start_vector = chrono::high_resolution_clock::now();
    taylor_vector(n, terms, x.data(), y_vector.data());
    auto end_vector = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed_vector = end_vector - start_vector;

    // Test 3: Hybrid (Threads + ISPC) version
    cout << "\nRunning hybrid (threads + SIMD) version..." << endl;
    auto start_parallel = chrono::high_resolution_clock::now();
    taylor_parallel(n, terms, x.data(), y_parallel.data());
    auto end_parallel = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed_parallel = end_parallel - start_parallel;

    // Print results
    cout << "\nPerformance Results:" << endl;
    cout << "Serial version took:    " << elapsed_serial.count() << " seconds" << endl;
    cout << "Vector version took:    " << elapsed_vector.count() << " seconds" << endl;
    cout << "Hybrid version took:    " << elapsed_parallel.count() << " seconds" << endl;

    // Calculate speedups
    double vector_speedup = elapsed_serial.count() / elapsed_vector.count();
    double hybrid_speedup = elapsed_serial.count() / elapsed_parallel.count();
    
    cout << "\nSpeedup Results:" << endl;
    cout << "Vector speedup:         " << vector_speedup << "x" << endl;
    cout << "Hybrid speedup:         " << hybrid_speedup << "x" << endl;

    // Verify results
    cout << "\nVerifying results..." << endl;
    bool results_match = true;
    for (int i = 0; i < n; i++) {
        if (abs(y_serial[i] - y_vector[i]) > 1e-6 || 
            abs(y_serial[i] - y_parallel[i]) > 1e-6) {
            cout << "Results differ at index " << i << endl;
            cout << "Serial: " << y_serial[i] 
                 << " Vector: " << y_vector[i] 
                 << " Parallel: " << y_parallel[i] << endl;
            results_match = false;
            break;
        }
    }

    if (results_match) {
        cout << "All implementations produce matching results!" << endl;
    }

    return 0;
}