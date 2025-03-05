#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <string.h>
#include <time.h>

#define WIDTH 12800  
#define HEIGHT 9600  
#define MAX_ITERATIONS 1024 

// Function to compute a single pixel's value
static inline int mandel(float c_re, float c_im, int max_iterations) {
    float z_re = c_re, z_im = c_im;
    int i;
    for (i = 0; i < max_iterations; ++i) {
        if (z_re * z_re + z_im * z_im > 4.0f)
            break;

        float new_re = z_re * z_re - z_im * z_im;
        float new_im = 2.0f * z_re * z_im;
        z_re = c_re + new_re;
        z_im = c_im + new_im;
    }
    return i;
}

// Serial implementation of Mandelbrot calculation
double mandelbrotSerial(float x0, float y0, float x1, float y1,
                       int width, int height, int maxIterations, int* output) {
    float dx = (x1 - x0) / width;
    float dy = (y1 - y0) / height;
    
    double start_time = MPI_Wtime();
    
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            float x = x0 + i * dx;
            float y = y0 + j * dy;
            output[j * width + i] = mandel(x, y, maxIterations);
        }
    }
    
    double end_time = MPI_Wtime();
    return end_time - start_time;
}

// Function to write the PPM image
void writePPMImage(int* data, int width, int height, const char* filename, int maxIterations) {
    FILE* fp = fopen(filename, "wb");
    if (!fp) {
        fprintf(stderr, "Error: Unable to open output file '%s'\n", filename);
        return;
    }

    // Write PPM header
    fprintf(fp, "P6\n%d %d\n255\n", width, height);

    // Write image data
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            int value = data[j * width + i];
            
            // Convert iteration count to color (simple mapping)
            unsigned char r, g, b;
            if (value == maxIterations) {
                r = g = b = 0;  // Black for points in the set
            } else {
                // Color gradient based on iteration count
                value = value % 16;
                r = (value * 16) % 256;
                g = (value * 8) % 256;
                b = (value * 32) % 256;
            }
            
            fputc(r, fp);
            fputc(g, fp);
            fputc(b, fp);
        }
    }
    fclose(fp);
}

// Function to verify results between serial and parallel implementations
int verifyResults(int* serial, int* parallel, int width, int height) {
    for (int i = 0; i < width * height; i++) {
        if (serial[i] != parallel[i]) {
            return 0;
        }
    }
    return 1; 
}

int main(int argc, char** argv) {
    int rank, size;
    double start_time, end_time;
    float x0 = -2.0f, y0 = -1.0f;
    float x1 = 1.0f, y1 = 1.0f;
    
    // Initialize MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    // Calculate workload distribution - each process gets a set of rows
    int rows_per_process = HEIGHT / size;
    int remainder = HEIGHT % size;
    
    // Process 0 might get a slightly larger chunk if HEIGHT is not divisible by size
    int my_start_row = rank * rows_per_process + (rank < remainder ? rank : remainder);
    int my_num_rows = rows_per_process + (rank < remainder ? 1 : 0);
    int my_end_row = my_start_row + my_num_rows;
    
    if (rank == 0) {
        printf("Mandelbrot calculation with %d MPI processes\n", size);
        printf("Image size: %d x %d, Max iterations: %d\n", WIDTH, HEIGHT, MAX_ITERATIONS);
    }
    
    // Calculate values for assigned rows
    int* local_output = (int*)malloc(my_num_rows * WIDTH * sizeof(int));
    if (local_output == NULL) {
        fprintf(stderr, "Process %d: Memory allocation failed\n", rank);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    
    // Start timing for parallel implementation
    start_time = MPI_Wtime();
    
    // Calculate parameters
    float dx = (x1 - x0) / WIDTH;
    float dy = (y1 - y0) / HEIGHT;
    
    // Compute Mandelbrot set for this process's portion
    for (int j = 0; j < my_num_rows; j++) {
        for (int i = 0; i < WIDTH; i++) {
            float x = x0 + i * dx;
            float y = y0 + (j + my_start_row) * dy;
            local_output[j * WIDTH + i] = mandel(x, y, MAX_ITERATIONS);
        }
    }
    
    // End timing calculation for parallel portion
    end_time = MPI_Wtime();
    double local_time = end_time - start_time;
    double max_parallel_time;
    
    MPI_Reduce(&local_time, &max_parallel_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    
    // Gather results to process 0
    int* gather_counts = NULL;
    int* displacements = NULL;
    int* full_output = NULL;
    
    if (rank == 0) {
        // Allocate memory for the complete output
        full_output = (int*)malloc(WIDTH * HEIGHT * sizeof(int));
        if (full_output == NULL) {
            fprintf(stderr, "Master process: Memory allocation failed\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        
        // Calculate counts and displacements for MPI_Gatherv
        gather_counts = (int*)malloc(size * sizeof(int));
        displacements = (int*)malloc(size * sizeof(int));
        
        for (int i = 0; i < size; i++) {
            int proc_rows = rows_per_process + (i < remainder ? 1 : 0);
            gather_counts[i] = proc_rows * WIDTH;
            displacements[i] = (i * rows_per_process + (i < remainder ? i : remainder)) * WIDTH;
        }
    }
    
    // Gather all results to process 0
    MPI_Gatherv(local_output, my_num_rows * WIDTH, MPI_INT,
                full_output, gather_counts, displacements, MPI_INT,
                0, MPI_COMM_WORLD);
    
    // Only process 0 runs the serial version and performs the comparison
    if (rank == 0) {
        // Write the parallel output
        writePPMImage(full_output, WIDTH, HEIGHT, "mandelbrot_mpi.ppm", MAX_ITERATIONS);
        
        // Now run the serial implementation for comparison
        int* serial_output = (int*)malloc(WIDTH * HEIGHT * sizeof(int));
        if (serial_output == NULL) {
            fprintf(stderr, "Serial implementation: Memory allocation failed\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        
        printf("Running serial implementation for comparison...\n");
        double serial_time = mandelbrotSerial(x0, y0, x1, y1, WIDTH, HEIGHT, MAX_ITERATIONS, serial_output);
        
        // Write the serial output
        writePPMImage(serial_output, WIDTH, HEIGHT, "mandelbrot_serial.ppm", MAX_ITERATIONS);
        
        // Compare the results
        int results_match = verifyResults(serial_output, full_output, WIDTH, HEIGHT);
        
        // Report timings and speedup
        printf("Parallel implementation: %.3f ms\n", max_parallel_time * 1000);
        printf("Serial implementation: %.3f ms\n", serial_time * 1000);
        printf("Speedup: %.2fx\n", serial_time / max_parallel_time);
        
        if (results_match) {
            printf("Results match between serial and parallel implementations.\n");
        } else {
            printf("WARNING: Results do not match between implementations!\n");
        }
        
        // Clean up
        free(serial_output);
        free(full_output);
        free(gather_counts);
        free(displacements);
    }
    
    free(local_output);
    MPI_Finalize();
    return 0;
}