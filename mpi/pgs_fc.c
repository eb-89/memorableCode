/* 
* Program for Gram-Schmidt orthogonalization using MPI
* Optimized Version
* 
* Parallell contiguous implementation: 
*   - Each proc treats its vectors as one big block.
*         The function pgs() takes a (float *) as parameter to matrices
*         Rank 0 can send and receive a custom blocktype during initiation and finalization
*
*   - Given a set of k vectors in R^n as a matrix of row-vectors, M, (k <= n) the program returns:
*         k x m - matrix Q   - containing the vectors of M, orthonormalized
*         k x k - matrix R   - A lower triangular matrix R such that RQ = M
*
* Erik Bertse
* Parallell & distributed programming
* Uppsala University 2018
* 
*/

#include <mpi.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include <float.h>


double timer() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  double seconds = tv.tv_sec + (double)tv.tv_usec / 1000000;
  return seconds;
}

float dot(float * v_1, float * v_2, int n) {
  float dot = 0;
  int i;
  for (i=0;i<n;i++) {
    dot += v_1[i]*v_2[i];
  }
  return dot;
}

void normalize(float * vec, int n) {
    float scale = dot(vec,vec,n);
    scale = sqrt(scale);

    int j;
    for (j = 0;j<n;j++) {
      vec[j] =vec[j]/scale; 
    }
}



void pgs(float * lmtx, float * lrmtx, MPI_Comm comm, MPI_Datatype vector, int k, int n) { 

    int i,j,ii,jj;

    int size, rank;
    MPI_Comm_size(comm, &size);
    MPI_Comm_rank(comm, &rank);

    int lk = (int) k/size;
    int count = 0; //Keeps track of many vectors we have already processed (per cook).

    MPI_Request requests[size];

    float * rv = (float *) malloc(n*sizeof(float));
    float * cpy = (float *) malloc(n*sizeof(float));


    // Chief cook normalizes and sends out his first vector
    if (rank == 0) {
      
      memcpy(cpy, &lmtx[count*n], n*sizeof(float));
      normalize(&lmtx[count*n], n); 
      
      for (j = 0; j< size; j++ ) {         
        MPI_Isend(&lmtx[count*n], 1, vector, j, 0, comm, &requests[j]);
      }
      
      // Compute the dot with the un-normalized vector, to compute <a_i , e_i> . 
      lrmtx[count*k] = dot(&lmtx[count*n], cpy, n);
      count++;
      
    }


    for (i = 1; i<k; i++) {

      // Receive the vector from the cook
      // that finished in the previous iteration
      MPI_Recv(rv, 1, vector, (i-1) % size, i-1, comm, MPI_STATUS_IGNORE);
      
      // Chief cook waits for the sends to finish on first iteration. 
      if (i == 1) {
        if (rank == 0) {
          fflush(stdout);
          MPI_Waitall(size, requests, MPI_STATUSES_IGNORE);
        }
      }

      // If its your turn to send...
      if (i % size == rank) {

        // Then immediately compute your current vector
        // which is the next finished vector...
        float dotpr = dot(rv, &lmtx[count*n], n);
        lrmtx[count*k + (i-1)] = dotpr;

        for (jj = 0; jj<n;jj++) {
          lmtx[n*count + jj] = lmtx[n*count + jj] - (dotpr)*rv[jj]; 
        }
        
        memcpy(cpy, &lmtx[count*n], n*sizeof(float));
        normalize(&lmtx[count*n], n); 

        if (i!=k-1) {
          // Post a send for your current vector...
          for (j = 0; j< size; j++ ) {     
            MPI_Isend(&lmtx[count*n], 1, vector, j, i, comm, &requests[j]);
          }
        }

        lrmtx[count*k + i] = dot(&lmtx[count*n], cpy, n);
        count++;        
      }

      // Compute all remaining projections for this incoming vector
      for (j = count; j<lk; j++) {

        // Compute the dot
        float dotpr = dot(rv, &lmtx[j*n], n);
        lrmtx[j*k + (i-1)] = dotpr;

        // Subtract projections
        for (jj = 0;jj<n;jj++) {
          lmtx[n*j + jj] = lmtx[n*j + jj] - (dotpr)*rv[jj]; 
        }

      }

      // Make sure we sent
      if (i % size == rank && (i != k-1)) {
        MPI_Waitall(size, requests, MPI_STATUSES_IGNORE);
      }
    }

    free(rv);
    free(cpy);

    // End of PGS
}


int main(int argc, char *argv[]) {


    MPI_Init(&argc, &argv);

    //looping variables
    int i,j,ii,jj;

    int rank, size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    //  k# vectors in R^n 
    int n; // Can be arbitraty
    int k; // Size must divide k

    if (argc != 3) {
      if (rank == 0) {
       printf("Please provide n and k, where k is the #of vectors in R^n \n    Pass:   filename k n \n");
      } 
      MPI_Finalize();
      exit(0);

    } else {
      k = atoi(argv[1]);
      n = atoi(argv[2]);

    }



    double tmr; //timer
    short unsigned int seed = 30;

    if ((k % size) != 0) {
      while ((k % size) != 0) {
        k--;
      }
    }


    //OUTPUT MATRICES
    // mtx contains Q
    // rmtx contains R
    float * mtx;
    float * rmtx;


    //lk# of vectors per proc
    int lk = (int) k/size;

    // Local mtx and Local R-mtx
    float * lmtx = (float *) malloc(lk*n*sizeof(float));
    float * lrmtx = (float *) calloc(lk*k, sizeof(float *));


    // The vector types
    MPI_Datatype vector;
    MPI_Datatype kvector;
    MPI_Datatype blocktype; // Only relevant at rank 0 for sending and recieving.  
    MPI_Datatype kblocktype;


    // kvectors are used to send home vectors of R.
    MPI_Type_vector(1, n, 0, MPI_FLOAT, &vector);
    MPI_Type_vector(1, k, 0, MPI_FLOAT, &kvector);
    MPI_Type_vector(lk, n, n*size, MPI_FLOAT, &blocktype);
    MPI_Type_vector(lk, k, k*size, MPI_FLOAT, &kblocktype);

    MPI_Type_commit(&vector); 
    MPI_Type_commit(&kvector); 
    MPI_Type_commit(&blocktype); 
    MPI_Type_commit(&kblocktype); 

    // Post nonblocking recieves.
    MPI_Request init_request; 
    MPI_Irecv(lmtx, lk, vector, 0, rank, MPI_COMM_WORLD, &init_request);


    if (rank == 0) {
      printf("Dims: k = %d, n = %d \n",k,n );

      // Allocate the initial matrices
      mtx = (float *) malloc(k*n*sizeof(float));
      rmtx = (float *) malloc(k*k*sizeof(float));

      seed48(&seed);
      for (i = 0; i < k; i++) {
        for (j = 0; j < n; j++) {
            mtx[n*i + j] = 10*drand48();
            //mtx[n*i + j] = (1.0/((i+1)+(j+1)-1)); //hilbert Matrix
        }
      }


      printf("Timing PGS... ");
      fflush(stdout);
      tmr = timer();

      // Rank 0 sends his data out
      for (i = 0; i< size; i++  ) {
          MPI_Send(&mtx[i*n], 1, blocktype, i, i, MPI_COMM_WORLD);
      }
    }



    // Make sure we recieved our block
    MPI_Wait(&init_request, MPI_STATUSES_IGNORE);
    
    // ---- HERE WE PERFORM PARALLELL GS
    pgs(lmtx, lrmtx, MPI_COMM_WORLD, vector, k, n);

    // Send everything home
    MPI_Request home_requests[size];
    MPI_Request rhome_requests[size];

    if (rank == 0) {
      for (i = 0; i < size; i++) {
          MPI_Irecv(&mtx[i*n], 1, blocktype, i, i, MPI_COMM_WORLD, &home_requests[i]);
          MPI_Irecv(&rmtx[i*k], 1, kblocktype, i, i, MPI_COMM_WORLD, &rhome_requests[i]);
      }
    }

    MPI_Send(lmtx, lk, vector, 0, rank, MPI_COMM_WORLD);
    MPI_Send(lrmtx, lk, kvector, 0, rank, MPI_COMM_WORLD);


    if (rank == 0) {
      MPI_Waitall(size, home_requests, MPI_STATUSES_IGNORE);
      MPI_Waitall(size, rhome_requests, MPI_STATUSES_IGNORE);
    }

    
    free(lrmtx);
    free(lmtx);

    MPI_Barrier(MPI_COMM_WORLD);


    MPI_Type_free(&vector); 
    MPI_Type_free(&kvector); 
    MPI_Type_free(&blocktype); 
    MPI_Type_free(&kblocktype); 


    // Various tests
    if (rank == 0) {
      double pgs_time = timer() - tmr;
      printf("Done \n");
      printf(" - PGS Time : %f s \n", pgs_time);
      
      float * tmtx = (float *) malloc(k*n*sizeof(float));
      float * rqmtx = (float *) malloc(k*n*sizeof(float));


      seed48(&seed);
      for (i = 0; i < k; i++) {
        for (j = 0; j < n; j++) {
            tmtx[i*n + j] = 10*drand48(); 
            //tmtx[n*i + j] = (1.0/((i+1)+(j+1)-1)); //hilbert matrix
        }
      }


      printf("Computing RQ as returned by PGS...  ");
      fflush(stdout);
      
      for (i = 0; i < k; i++) {
        for (j = 0; j< n; j++) {
          float sum = 0;
          for (ii = 0; ii< k; ii++) {
              sum = sum + rmtx[i*k + ii]*mtx[ii*n  + j];
          }
          rqmtx[i*n + j] = sum;

        }
      }
      printf("Done \n \n");
      fflush(stdout);

      float normerror = 0;
      float current;
      
      
      printf("TEST: Validating equality of RQ with original matrix... ");
      fflush(stdout);
      for (i = 0; i<k; i++) {
        for (j = 0; j<n; j++) {

          current = fabs(rqmtx[i*n + j] - tmtx[i*n + j]);
          normerror += current*current;

        }
      }
      normerror = sqrt(normerror);
      printf("  Done, error in |RQ - M|: %f \n", normerror);
      

      printf("TEST: Validating orthogonality of PGS output... ");
      normerror = 0;

      for (i = 0; i<k; i++) {
        for (j = 0; j<k; j++) {

           float d = dot(&mtx[i*n], &mtx[j*n], n);

           //if (j % k == 0) {printf(" \n");}
           //printf("%7.2f  ", d );
          
           if (i == j) { current = fabs(d - 1.); }
           if (i != j) { current = fabs(d); }
           normerror += current*current;

         }
      }
      normerror = sqrt(normerror);
      printf("  Done, error in |Q^tQ - I|: %f \n", normerror);
      fflush(stdout);

      free(tmtx);
      free(rqmtx);
      free(mtx);
      free(rmtx);
    }
    

    MPI_Finalize();
    return 0;
}


