#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

#define ARRAY_SIZE 1000000000   
#define MASTER 0         

int main(int argc,char *argv[]){
    int nprocs;
    int pid;
    int *data = NULL;
    int *chunk = NULL;
    int i;
    int chunksize;
    int localSum = 0;
    int globalSum = 0;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &pid);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    chunksize = ARRAY_SIZE / nprocs;

    chunk = (int *) malloc(chunksize * sizeof(int));
    // handle memory allocation error
    if (chunk == NULL) {
        printf("Memory allocation failed for process %d\n", pid);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    // Prepare the data array in the master process
    if (pid == MASTER) {
        printf("Starting MapReduce simulation with %d processes\n", nprocs);
        printf("Array size: %d, Chunk size: %d\n", ARRAY_SIZE, chunksize);
        
        // Allocate and initialize the full array
        data = (int *) malloc(ARRAY_SIZE * sizeof(int));
        if (data == NULL) {
            printf("Memory allocation failed for master process\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        
        // Initialize array with random values
        srand(time(NULL));
        for (i = 0; i < ARRAY_SIZE; i++) {
            data[i] = rand() % 3;  // Random values between 0-2
        }
        
        printf("Array initialized by master process\n");
    }

    MPI_Scatter(data, chunksize, MPI_INT, chunk, chunksize, MPI_INT, MASTER, MPI_COMM_WORLD);

    for (i = 0; i < chunksize; i++) {
        localSum += chunk[i];
    }

    MPI_Reduce(&localSum, &globalSum, 1, MPI_INT, MPI_SUM, MASTER, MPI_COMM_WORLD);

    // go to the master process
    if (pid == MASTER) {
        printf("Global sum computed by master process: %d\n", globalSum);
        int serialSum = 0;
        for (i = 0; i < ARRAY_SIZE; i++) {
            serialSum += data[i];
        }
        if (globalSum == serialSum) {
            printf("Simulation Accomplished Successfully\n");
        } else {
            printf("Global sum is incorrect\n");
        }
        
    }
    free(data);
    free(chunk);


    MPI_Finalize();
    return 0;

}