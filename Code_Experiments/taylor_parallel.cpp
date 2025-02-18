#include <math.h>
#include <iostream>
#include <thread>
#include <vector>
#include <random>
#include <chrono>

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
    sinx(t->n, t->terms, t->x, t->y);
}


void taylor_parallel(int n, int terms, double* x, double* y){
    thread t1;
    taylor_args tp;
    tp.n = n/2;
    tp.terms = terms;
    tp.x = x;
    tp.y = y;

    t1 = thread(my_thread_fn, &tp);
    sinx(n - tp.n, tp.terms, x + tp.n, y + tp.n);
    t1.join();

}


void taylor_serial(int n, int terms, double* x, double* y) {
    sinx(n, terms, x, y);
}

int main()
{
    int n = 100000000;
    int terms = 10;

    vector<double> x(n);

    // Generate normally distributed pi multipliers values
    random_device rd;
    mt19937 gen(rd());
    normal_distribution<> d(M_PI, 1);
    for (int i = 0; i < n; i++) {
        x[i] = d(gen);
    }

    vector<double> y_parallel(n);
    vector<double> y_serial(n);

    auto start_parallel = chrono::high_resolution_clock::now();
    taylor_parallel(n, terms, x.data(), y_parallel.data());
    auto end_parallel = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed_parallel = end_parallel - start_parallel;

    auto start_serial = chrono::high_resolution_clock::now();
    taylor_serial(n, terms, x.data(), y_serial.data());
    auto end_serial = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed_serial = end_serial - start_serial;

    cout << "Parallel version took: " << elapsed_parallel.count() << " seconds" << endl;
    cout << "Serial version took: " << elapsed_serial.count() << " seconds" << endl;

    // Check if the results are the same
    for (int i = 0; i < n; i++) {
        if (abs(y_parallel[i] - y_serial[i]) > 1e-6) {
            cout << "Results differ at index " << i << endl;
            cout << "Parallel: " << y_parallel[i] << " Serial: " << y_serial[i] << endl;
            return 1;
        }
    }

    // calculate the speedup
    cout << "Speedup: " << elapsed_serial.count() / elapsed_parallel.count() << endl;

    cout << "Results are the same" << endl;

    return 0;
}