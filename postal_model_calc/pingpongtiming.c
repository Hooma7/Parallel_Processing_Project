#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>


double timesendrecv(int N, double* send_array, double* recv_array){
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Status status;

    //Synchronize the process before timing
    MPI_Barrier(MPI_COMM_WORLD);
    
    double start = MPI_Wtime();
    for(int i=0; i<1000; i++){
        if(rank ==0){
            MPI_Send(send_array, N, MPI_DOUBLE, 1, 0, MPI_COMM_WORLD);
            MPI_Recv(recv_array, N, MPI_DOUBLE, 1, 0, MPI_COMM_WORLD, &status);
        }
        if(rank ==1){
            MPI_Recv(send_array, N, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, &status);
            MPI_Send(recv_array, N, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
        }
    }
    double total = MPI_Wtime() - start;
    double cost = total / 2000;
    return cost;
}

int main(int argc, char* argv[]){
    MPI_Init(&argc, &argv);
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    srand(time(NULL)*(rank+1));


    double cost;
    
    for(int i=0; i<16; i++){
        long N= 1 << i;
        double* send_array = (double*)malloc(N*sizeof(double));
        double* recv_array = (double*)malloc(N*sizeof(double));

        for(int j=0; j<N; j++){
            send_array[j] = rand()/RAND_MAX;
        }

        cost = timesendrecv(N, send_array, recv_array);
        if(rank == 0){
            printf("i: %d,\t Size: %ld, \tCost of message: %.9f seconds\n", i, N, cost);
        }

        free(send_array);
        free(recv_array);
    }


    MPI_Finalize();
    return 0;
}