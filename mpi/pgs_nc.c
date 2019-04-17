/* 
* Program for Gram-Schmidt orthogonalization using MPI
* Parallell non-contiguous implementation: 
*   - Each proc treats its vectors as located separately in memory.
*         The function pgs() takes a (float **) as parameter to matrices
*         Rank 0 sends and receives separate vectors during initiation and finalization
*
*   - Given a set of k vectors in R^n as a matrix of row-vectors, M, the program returns:
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



void pgs(float ** lmtx, float ** lrmtx, MPI_Comm comm, MPI_Datatype vector, int k, int n) { 

    int i,j,ii,jj;

    int size, rank;
    MPI_Comm_size(comm, &size);
    MPI_Comm_rank(comm, &rank);

    int lk = (int) k/size;
    int count = 0; //Keeps track of many vectors we have already processed (per cook).

    MPI_Request requests[size];

    float * rv = (float *) malloc(n*sizeof(float));
    float * cpy = (float *) malloc(n*sizeof(float));

    for (i = 0; i<k; i++) {

      // If you just finished vector i..
      if (i % size == rank) {
        
        //Make a copy, since normalize() overwrites the vector.
        memcpy(cpy, lmtx[count], n*sizeof(float));

        // Normalize it and send it out non-blocking
        normalize(lmtx[count], n); 
        for (j = 0; j< size; j++ ) {         
          MPI_Isend(lmtx[count], 1, vector, j, i, comm, &requests[j]);
        }
        
        // Compute the dot with the un-normalized vector, to compute <a_i , e_i> . 
        lrmtx[count][i] = dot(lmtx[count], cpy, n);
        count++;
        
      }


      MPI_Recv(rv, 1, vector, i % size, i, comm,MPI_STATUS_IGNORE);

      // For all your vectors that are yet not done...
      for (j = count; j<lk; j++) {

        // Compute the dot
        float dotpr = dot(rv, lmtx[j], n);

        // Save the dot 
        lrmtx[j][i] = dotpr;
        
        // Subtract projections
        for (jj = 0;jj<n;jj++) {
          lmtx[j][jj] = lmtx[j][jj] - (dotpr)*rv[jj]; 
        }

      }

      if (i % size == rank) {
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
    int n; // Can be arbitrary
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
      if (rank == 0) {
        printf("The number of vectors is not divisible by number of procs, existing... \n");
      }
      MPI_Finalize();
      exit(0);
    }

    //OUTPUT MATRICES
    // mtx contains Q
    // rmtx contains R
    float ** mtx;
    float ** rmtx;


    //lk# of vectors per proc
    int lk = (int) k/size;


    // Local mtx and Local R-mtx
    float ** lmtx = (float **) malloc(lk*sizeof(float *));
    float ** lrmtx = (float **) calloc(lk, sizeof(float *));

    for (i = 0; i<lk; i++) {
      lmtx[i] = (float *) malloc(n*sizeof(float));
      lrmtx[i] = (float *) calloc(k, sizeof(float));
    }

    // The vector types
    MPI_Datatype vector;
    MPI_Datatype kvector;


    // kvectors are used to send home vectors of R.
    MPI_Type_vector(1, n, 0, MPI_FLOAT, &vector);
    MPI_Type_vector(1, k, 0, MPI_FLOAT, &kvector);
    
    MPI_Type_commit(&vector); 
    MPI_Type_commit(&kvector); 


    // Post nonblocking recieves.
    MPI_Request init_requests[lk]; 
    for (i = 0; i<lk; i++) {
      MPI_Irecv(lmtx[i], 1, vector, 0, i, MPI_COMM_WORLD, &init_requests[i]);
    }


    if (rank == 0) {

      // Allocate the initial matrices
      mtx = (float **) malloc(k*sizeof(float *));
      rmtx = (float **) malloc(k*sizeof(float *));
      for (i = 0; i<k; i++) {
        mtx[i] = (float *) malloc(n*sizeof(float));
        rmtx[i] = (float *) malloc(k*sizeof(float));
      }

      seed48(&seed);
      for (i = 0; i < k; i++) {
        for (j = 0; j < n; j++) {
            mtx[i][j] = 10*drand48(); 
        }
      }



      printf("Timing PGS... ");
      fflush(stdout);
      tmr = timer();

      // Rank 0 sends his data out
      int to;
      for (i = 0; i< k; i++  ) {
          to = i % size;
          MPI_Send(mtx[i], 1, vector, to, i/size, MPI_COMM_WORLD);
      }


    }

    // Make sure we recieved
    MPI_Waitall(lk, init_requests, MPI_STATUSES_IGNORE);
    
    // ---- HERE WE PERFORM PARALLELL GS    
    pgs(lmtx, lrmtx, MPI_COMM_WORLD, vector, k, n);

    // Send everything home
    MPI_Request home_requests[k];
    MPI_Request rhome_requests[k];
    

    if (rank == 0) {
      for (i = 0; i<k; i++) {
        MPI_Irecv(mtx[i], 1, vector, i%size, i, MPI_COMM_WORLD, &home_requests[i]);
        MPI_Irecv(rmtx[i], 1, kvector, i%size, i, MPI_COMM_WORLD, &rhome_requests[i]);
      }
    }

    for (i = 0; i<lk;i++) {
        MPI_Send(lmtx[i], 1, vector, 0, rank + size*i, MPI_COMM_WORLD);
        MPI_Send(lrmtx[i], 1, kvector, 0, rank + size*i, MPI_COMM_WORLD);
    }

    if (rank == 0) {
      MPI_Waitall(k, home_requests, MPI_STATUSES_IGNORE);
      MPI_Waitall(k, rhome_requests, MPI_STATUSES_IGNORE);
    }
    
    // Free local matrix
    for (i = 0; i<lk; i++) {
      free(lrmtx[i]);
      free(lmtx[i]); 
    }
    free(lrmtx);
    free(lmtx);

    MPI_Type_free(&vector); 
    MPI_Type_free(&kvector); 


    // Various tests
    if (rank == 0) {
      double pgs_time = timer() - tmr;
      printf("Done \n");
      printf(" - PGS Time : %f s \n", pgs_time);
      
      float ** tmtx = (float **) malloc(k*sizeof(float *));
      float ** rqmtx = (float **) malloc(k*sizeof(float *));

      for (i = 0; i<k; i++) {
        tmtx[i] = (float *) malloc(n*sizeof(float));
        rqmtx[i] = (float *) malloc(n*sizeof(float));
      }

      seed48(&seed);
      for (i = 0; i < k; i++) {
        for (j = 0; j < n; j++) {
            tmtx[i][j] = 10*drand48(); 
        }
      }

      printf("Computing RQ as returned by PGS...  ");
      fflush(stdout);
      
      for (i = 0; i < k; i++) {
        for (j = 0; j< n; j++) {
          float sum = 0;
          for (ii = 0; ii< k; ii++) {
              sum = sum + rmtx[i][ii]*mtx[ii][j];
          }
        rqmtx[i][j] = sum;

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

          current = fabs(rqmtx[i][j] - tmtx[i][j]);
          normerror += current*current;

        }
      }
      normerror = sqrt(normerror);
      printf("  Done, error in |RQ - M|: %f \n", normerror);
      

      printf("TEST: Validating orthogonality of PGS output... ");
      fflush(stdout);
      normerror = 0;
      fflush(stdout);
      for (i = 0; i<k; i++) {
        for (j = 0; j<k; j++) {

           float d = dot(mtx[i], mtx[j], n);

           //if (i % k == 0) {printf(" \n");}
           //printf("%7.2f  ", d );
           if (i == j) { current = fabs(d - 1.); }
           if (i != j) { current = fabs(d); }
           normerror += current*current;

         }
      }
      normerror = sqrt(normerror);
      printf("  Done, error in |Q^tQ - I|: %f \n", normerror);
      fflush(stdout);



      // free matrix
      for (i = 0; i<k; i++) {
        free(mtx[i]);
        free(rmtx[i]); 

        free(tmtx[i]); 
        free(rqmtx[i]);
      }
      free(mtx);
      free(rmtx);

      free(tmtx);
      free(rqmtx);
    }
    

    MPI_Finalize();
    return 0;
}


