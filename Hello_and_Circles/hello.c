#include <stdio.h>
#include <mpi.h>
int main(int argc,char* argv[]){
    // 1. write your sequential code

    // 2. start your parralel section
    // > MPI_Init(&argc,&argv);

    // the whole program is parallel! from start to finish
    MPI_Init(NULL,NULL);

    // how many ranks (IDs) I can give
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD,&world_size);

    // Ranking means that I give for each process an ID
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD,&world_rank);


    // Get Processor Name
    char pname[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(pname,&name_len);
    printf("Hello World from Processor %s, rank %d out of %d processors \n",
                pname,world_rank,world_size);

    

    // 3. write the the parallel code
    MPI_Finalize();
    // 4. Finalize!


    // Essentials of Message Passing
    // we send (what?,how many?,type?,to where?)
    // > MPI_Send(buffer, count, datatype, dest, tag, comm);
    // we recieve (what?,how many?,type?,from where?)
    // > MPI_Recv(buffer, count , datatype ,src);

    // *Recv is blocking cause if it didnt recieve it will block the entire
    //  program waiting for this specific message
}