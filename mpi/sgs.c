/* 
* Program for Gram-Schmidt orthogonalization using MPI
* Serial implementation - all work on one proc
* 
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

float normalize(float * vec, int n) {
    float scale = dot(vec,vec,n);
    scale = sqrt(scale);

    int j;
    for (j = 0;j<n;j++) {
      vec[j] =vec[j]/scale; 
    }
}


// Serial Modified GS
void smgs(float ** mtx, int k, int n) {

  int i,j,ii;

  for (i = 0; i<k; i++) {

    for (ii=0; ii<i; ii++) {
      //Note that mtx[i] gets updated in each iteration
      float den = dot(mtx[i], mtx[ii],n);
      float num = dot(mtx[ii], mtx[ii],n);
      
      for (j = 0;j<n;j++) {
        mtx[i][j] = mtx[i][j] - (den/num)*mtx[ii][j]; 
      }
        
    }

    normalize(mtx[i],n);
    
  }

}


// Serial Classical GS
void sgs(float ** mtx, int k, int n) {

  int i,j,jj,ii;

  float * result = (float *) malloc(n*sizeof(float));

  for (i = 0; i<k; i++) {


    for (jj=0;jj<n;jj++) {
      result[jj] = mtx[i][jj];
    }

    for (ii=0; ii<i; ii++) {
      //Note that mtx[i] does NOT get updated in each iteration
      float den = dot(mtx[i], mtx[ii],n);
      float num = dot(mtx[ii], mtx[ii],n);
      
      for (j = 0;j<n;j++) {
        result[j] = result[j] - (den/num)*mtx[ii][j]; 
      }
    }

    for (j = 0;j<n;j++) {
      mtx[i][j] = result[j]; 
    }

    normalize(mtx[i], n);
  }

  free(result);
}


int main(int argc, char *argv[]) {

    int i,j, ii, jj;
	  int rank, size;


  	MPI_Init(&argc, &argv);

    MPI_Comm_size(MPI_COMM_WORLD, &size);
  	MPI_Comm_rank(MPI_COMM_WORLD, &rank);


    if (rank == 0) {
      printf("Starting serial GS \n");

      //  k# vectors in R^n 
      int n = 3;
      int k = 3;

      float **mtx = (float **) malloc(k*sizeof(float *));
      for (i = 0; i<k; i++) {
        mtx[i] = (float *) malloc(n*sizeof(float));
      }

      // Generate random numbers
      printf("Randomizing elements...  ");
      fflush(stdout);
      short unsigned int seed = 41;
      srand(seed);
      seed48(&seed);

      float epsilon = 0.0001;
      for (i = 0; i < k; i++) {
        for (j = 0; j < n; j++) {
          mtx[i][j]= drand48()*10;

          
          // * Some problematic matrices for testing

          //mtx[i][j]= (1.0/((i+1)+(j+1)-1));
          
          /*
          if (i == 0) {
            if (j == 0) {mtx[i][j] = 1.;}
            if (j == 1) {mtx[i][j] = epsilon;}
            if (j == 2) {mtx[i][j] = epsilon;}
          }
          if (i == 1) {
            if (j == 0) {mtx[i][j] = 1.;}
            if (j == 1) {mtx[i][j] = epsilon;}
            if (j == 2) {mtx[i][j] = 0.;}
          }
          if (i == 2) {
            if (j == 0) {mtx[i][j] = 1.;}
            if (j == 1) {mtx[i][j] = 0.;}
            if (j == 2) {mtx[i][j] = epsilon;}
          }  
          */
        }
      }
      printf("Done!\n");

      printf("In-Matrix is: \n");
      for (i = 0; i<k;i++) {
        for (j = 0; j<n; j++) {
          printf("%8.5f ", mtx[i][j]);
        }
        printf(" \n");
      }

      // GS
      smgs(mtx, k, n);


      printf("Out-Matrix is: \n");
      for (i = 0; i<k;i++) {
        for (j = 0; j<n; j++) {
          printf("%8.5f ", mtx[i][j]);
        }
        printf(" \n");
      }
      printf("A*A^t is: \n");
      for (i = 0; i<k;i++) {
        for (j = 0; j<k; j++) {
          printf("%9.1e  ", dot(mtx[i], mtx[j],n));
        }
        printf(" \n");
      }
      

      for (i = 0; i < k; i++) {
        free(mtx[i]);
      }
      free(mtx);

    }


    MPI_Finalize();
}


