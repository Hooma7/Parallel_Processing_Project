#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>
#include <math.h>
#include <string.h>


void recursive_doubling(double* sendbuf, double* recvbuf, int size){
    int rank, num_procs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    int tag = 1234;

    int num_iters = log2(num_procs);
    int distance = 1;
    
    memcpy(&recvbuf[0], &sendbuf[0], size*sizeof(double));
    for(int i=0; i<num_iters; i++){
        int group = rank / distance;
        int partner_proc;
        
        if(group % 2 == 0){
            partner_proc = rank + distance;
        }
        else{
            partner_proc = rank - distance;
        }
        int sendsize = size * distance;
        MPI_Sendrecv(&recvbuf[0], sendsize, MPI_DOUBLE, partner_proc, tag, 
            &recvbuf[distance*size], sendsize, MPI_DOUBLE, partner_proc, tag, 
            MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            distance = distance << 1;
    }
}

void bruck(double* sendbuf, double* recvbuf, int size){
    int rank, num_procs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    int tag = 1234;
    
    int num_iters = log2(num_procs);
    int distance = 1;
    
    memcpy(&recvbuf[0], &sendbuf[0], size*sizeof(double));
    for(int i=0; i<num_iters; i++){
        int send_proc = (num_procs + rank - distance) % num_procs;
        int recv_proc = (rank + distance) % num_procs;


        int sendsize = size * distance;
        MPI_Sendrecv(&recvbuf[0], sendsize, MPI_DOUBLE, send_proc, tag, 
            &recvbuf[distance*size], sendsize, MPI_DOUBLE, recv_proc, tag, 
            MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        distance = distance << 1;
    }
}

void ring(double* sendbuf, double* recvbuf, int size){
    int rank, num_procs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    int tag = 1234;

    int send_proc = (rank+1) % num_procs;
    int recv_proc = rank-1;
    if(recv_proc < 0){
        recv_proc = num_procs-1;
    }

    double* tmp = (double*)malloc(size*num_procs*sizeof(double));
    memcpy(&recvbuf[0], &sendbuf[0], size*sizeof(double));

    for(int i=0; i<num_procs-1; i++){
        memcpy(&tmp[i*size], &recvbuf[i*size], size*sizeof(double));
        MPI_Sendrecv(&tmp[i*size], size, MPI_DOUBLE, send_proc, tag, 
            &recvbuf[(i+1)*size], size, MPI_DOUBLE, recv_proc, tag, 
            MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
}


int main(int argc, char* argv[]){
    MPI_Init(&argc, &argv);

    int rank, num_procs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    srand(time(NULL)*(rank+1));
    for(int i=0; i<26; i++){
        int size = 1 << i;
        double* sendbuf = (double*)malloc(size*sizeof(double));
        double* recvbuf = (double*)malloc(size*num_procs*sizeof(double));

        if(rank==0){
            for(int j=0; j<size; j++){
                sendbuf[j] = (double)rand() / (double)RAND_MAX;
            }
        }


        MPI_Barrier(MPI_COMM_WORLD);
        double recursive_start = MPI_Wtime();
        recursive_doubling(sendbuf, recvbuf, size);
        double recursive_time = MPI_Wtime() - recursive_start;


        MPI_Barrier(MPI_COMM_WORLD);
        double brucks_start = MPI_Wtime();
        bruck(sendbuf, recvbuf, size);
        double brucks_time = MPI_Wtime() - brucks_start;


        MPI_Barrier(MPI_COMM_WORLD);        
        double ring_start = MPI_Wtime();
        ring(sendbuf, recvbuf, size);
        double ring_time = MPI_Wtime() - ring_start;

        if(rank == 0)
            printf("Message size: 2^%d\t\tRecursive Doubling Time: %E\t\tBruck's Time: %E\t\tRing Time: %E\n",i,recursive_time,brucks_time,ring_time);
        
    }

    MPI_Finalize();
    return 0;
}