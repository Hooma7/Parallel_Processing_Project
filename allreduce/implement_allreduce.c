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

    double* tmp = (double*)malloc(size*sizeof(double));
    for(int i=0; i<num_iters; i++){
        int group = rank / distance;
        int partner_proc;
        
        if(group % 2 == 0){
            partner_proc = rank + distance;
        }
        else{
            partner_proc = rank - distance;
        }
        MPI_Sendrecv(recvbuf, size, MPI_DOUBLE, partner_proc, tag, 
            tmp, size, MPI_DOUBLE, partner_proc, tag, 
            MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
        for(int j=0; j<size; j++) recvbuf[j] += tmp[j];
        distance = distance << 1;

    }
}

void binom_tree(double* sendbuf, double* recvbuf, int size){
    int rank, num_procs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    int tag = 1234;
    
    int num_iters = log2(num_procs);

    memcpy(&recvbuf[0], &sendbuf[0], size*sizeof(double));
    double* tmp = (double*)malloc(size*sizeof(double));

    int distance = 1;
    for(int i=0; i<num_iters; i++){
        int sending = rank & distance;
        int receiving = (rank & distance) == 0 && (rank+distance) < num_procs;

        MPI_Request send_req, recv_req;
        if(sending){
            int send_proc = rank & ~distance;
            MPI_Isend(recvbuf, size, MPI_DOUBLE, send_proc, tag, MPI_COMM_WORLD, &send_req);
        }
        if(receiving){
            int recv_proc = rank | distance;
            MPI_Irecv(tmp, size, MPI_DOUBLE, recv_proc, tag, MPI_COMM_WORLD, &recv_req);
        }

        if(sending) MPI_Wait(&send_req, MPI_STATUS_IGNORE);
        if(receiving){
            MPI_Wait(&recv_req, MPI_STATUS_IGNORE);
            for(int j=0; j<size; j++) recvbuf[j] += tmp[j];
        }
        distance = distance << 1;
    }

    distance = 1;
    for(int i=0; i<num_iters; i++){
        int offset = num_procs/(1<<(i+1));
        int send_proc = rank + offset;
        int recv_proc = rank - offset;

        if(send_proc < num_procs && rank % (offset * 2) == 0){
            MPI_Send(recvbuf, size, MPI_DOUBLE, send_proc, tag, MPI_COMM_WORLD);
        }
        else if(rank % (offset * 2) == offset){
            MPI_Recv(recvbuf, size, MPI_DOUBLE, recv_proc, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
    }

    free(tmp);
}

void rabenseifner(double* sendbuf, double* recvbuf, int size){
    int rank, num_procs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    int tag = 1234;

    int num_iters = log2(num_procs);

    memcpy(&recvbuf[0], &sendbuf[0], size*sizeof(double));
    double* tmp = (double*)malloc(size*sizeof(double));

    int recv_offset = 0;
    int sendSize = size;
    for(int i=0; i<num_iters; i++){
        int distance = 2 << (num_iters-1-i);
        int partner_proc = rank+distance % num_procs;

        int sendSize = sendSize/2;
        int send_offset, keep_offset;

        if(rank < partner_proc){
            send_offset = recv_offset + sendSize;
            keep_offset = recv_offset;
        }
        else{
            keep_offset = recv_offset + sendSize;
            send_offset = recv_offset;
        }
        MPI_Sendrecv(&recvbuf[send_offset], sendSize, MPI_DOUBLE, partner_proc, tag, 
            tmp, sendSize, MPI_DOUBLE, partner_proc, tag, 
            MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        for(int j=0; j<sendSize; j++) recvbuf[keep_offset + j] += tmp[j];

        recv_offset = keep_offset;
    }

    int distance = 1;
    int send_offset = recv_offset;

    for(int i=0; i<num_iters; i++){
        int group = rank / distance;
        int partner_proc;
        
        if(group % 2 == 0){
            partner_proc = rank + distance;
        }
        else{
            partner_proc = rank - distance;
        }

        int partner_offset;
        if(rank < partner_proc){
            partner_offset = recv_offset + sendSize;
        }
        else{
            partner_offset = recv_offset - sendSize;
        }

        MPI_Sendrecv(&recvbuf[recv_offset], sendSize, MPI_DOUBLE, partner_proc, tag, 
            &recvbuf[partner_offset], sendSize, MPI_DOUBLE, partner_proc, tag, 
            MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
        if(rank >= partner_proc){
            recv_offset = partner_offset;
        }
        distance = distance << 1;
        sendSize *= 2;
    }
}


int main(int argc, char* argv[]){
    MPI_Init(&argc, &argv);

    int rank, num_procs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    srand(time(NULL)*(rank+1));
    for(int i=0; i<21; i++){
        int size = 1 << i;
        double* sendbuf = (double*)malloc(size*sizeof(double));
        double* recvbuf = (double*)malloc(size*sizeof(double));

        for(int j=0; j<size; j++){
            sendbuf[j] = (double)rand() / (double)RAND_MAX;
        }

        MPI_Barrier(MPI_COMM_WORLD);
        double recursive_start = MPI_Wtime();
        recursive_doubling(sendbuf, recvbuf, size);
        double recursive_time = MPI_Wtime() - recursive_start;


        MPI_Barrier(MPI_COMM_WORLD);
        double binom_tree_start = MPI_Wtime();
        binom_tree(sendbuf, recvbuf, size);
        double binom_tree_time = MPI_Wtime() - binom_tree_start;


        MPI_Barrier(MPI_COMM_WORLD);        
        double rabenseifner_start = MPI_Wtime();
        rabenseifner(sendbuf, recvbuf, size);
        double rabenseifner_time = MPI_Wtime() - rabenseifner_start;

        if(rank == 0)
            printf("Message size: 2^%d\t\tRecursive Doubling Time: %E\t\tBinomial Tree Time: %E\t\tRabenseifner's Time: %E\n",i,recursive_time,binom_tree_time,rabenseifner_time);

	    free(sendbuf);
    	free(recvbuf);        
    }

    MPI_Finalize();
    return 0;
}
