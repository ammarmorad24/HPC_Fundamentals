#include <mpi.h>
#include <stdio.h>
int main(int argc, char** argv) {
    int message;
    scanf("%d", &message);
    MPI_Init(&argc, &argv);
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(processor_name, &name_len);
    if(world_rank == 0) {
        printf("Hello from process %d of %d on %s\n", world_rank, world_size, processor_name);
    }
    
    if(world_rank == 0){
        message *= 2;
        MPI_Send(&message,1,MPI_INT,3,0,MPI_COMM_WORLD);

    }
    if(world_rank == 3){
        MPI_Recv(&message,1,MPI_INT,0,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        printf("Process 3 received message which is the number %d from process 0\n", message);
    }
    
    
    MPI_Finalize();
    return 0;
}