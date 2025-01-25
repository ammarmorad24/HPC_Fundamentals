#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

struct circle {
    int *radius;
    int *diameter;
    double *circumference;
    double *area;
};

// Function to initialize circle params
void init_circle(struct circle *param, int size) {
    param->radius = (int *)malloc(size * sizeof(int));
    param->diameter = (int *)malloc(size * sizeof(int));
    param->circumference = (double *)malloc(size * sizeof(double));
    param->area = (double *)malloc(size * sizeof(double));
}

// Function to free circle params
void free_circle(struct circle *param) {
    free(param->radius);
    free(param->diameter);
    free(param->circumference);
    free(param->area);
}

int main(int argc, char **argv) {
    int world_size, world_rank;
    double start_time, end_time;
    const int num_circles = 1e8;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    // Initialize circle param
    struct circle circles;
    init_circle(&circles, num_circles);

    if (world_rank == 0) {
        // Populate radius values
        for (int i = 0; i < num_circles; i++) {
            circles.radius[i] = i % 100;
        }
    }

    // Broadcast radius values to all ranks
    MPI_Bcast(circles.radius, num_circles, MPI_INT, 0, MPI_COMM_WORLD);

    // Start timing
    start_time = MPI_Wtime();

    

    if (world_rank == 0) {
        // Calculate the diameter
        for (int i = 0; i < num_circles; i++) {
            circles.diameter[i] = 2 * circles.radius[i];
        }
    } else if (world_rank == 1) {
        // Calculate the circumference
        for (int i = 0; i < num_circles; i++) {
            circles.circumference[i] = 2 * 3.14 * circles.radius[i];
        }
    } else if (world_rank == 2) {
        // Calculate the area
        for (int i = 0; i < num_circles; i++) {
            circles.area[i] = 3.14 * circles.radius[i] * circles.radius[i];
        }
    }

    // End timing
    end_time = MPI_Wtime();
    printf("Time taken by process %d is %f seconds\n", world_rank, end_time - start_time);

    // Free circle param
    free_circle(&circles);

    MPI_Finalize();
    return 0;
}
