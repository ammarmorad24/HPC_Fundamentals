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

    // Circle param
    struct circle circles;
    init_circle(&circles, num_circles);

    if (world_rank == 0) {
        // Populate radius values
        for (int i = 0; i < num_circles; i++) {
            circles.radius[i] = i % 100;
        }

        // Send radius values to all other processes
        for (int rank = 1; rank < world_size; rank++) {
            MPI_Send(circles.radius, num_circles, MPI_INT, rank, 0, MPI_COMM_WORLD);
        }
    } else {
        // Receive radius values from process 0
        MPI_Recv(circles.radius, num_circles, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    // Start timing
    start_time = MPI_Wtime();

    if (world_rank == 1) {
        // Calculate the diameter
        for (int i = 0; i < num_circles; i++) {
            circles.diameter[i] = 2 * circles.radius[i];
        }
        // Send diameter to process 2
        MPI_Send(circles.diameter, num_circles, MPI_INT, 2, 0, MPI_COMM_WORLD);
    } else if (world_rank == 2) {
        // Receive diameter from process 1
        MPI_Recv(circles.diameter, num_circles, MPI_INT, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // Calculate the circumference
        for (int i = 0; i < num_circles; i++) {
            circles.circumference[i] = 3.14159 * circles.diameter[i];
        }
        // Send circumference to process 3
        MPI_Send(circles.circumference, num_circles, MPI_DOUBLE, 3, 0, MPI_COMM_WORLD);
    } else if (world_rank == 3) {
        // Receive circumference from process 2
        MPI_Recv(circles.circumference, num_circles, MPI_DOUBLE, 2, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // Calculate the area
        for (int i = 0; i < num_circles; i++) {
            // area = circumference / (2 * pi) * circumference / (2 * pi) * pi
            circles.area[i] = (circles.circumference[i] / (2 * M_PI)) * (circles.circumference[i] / (2 * M_PI)) * M_PI;
        }
    }

    // End timing
    end_time = MPI_Wtime();
    printf("Time taken by process %d is %f seconds\n", world_rank, end_time - start_time);

    // Free memory
    free_circle(&circles);

    MPI_Finalize();
    return 0;
}
