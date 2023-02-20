#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

int main(int argc, char **argv) {
    double *A, *B, *C;
    int i, j, k;
    const int n = strtol(argv[1], NULL, 10);
    struct timeval start, end;
    double time;
    
    A = (double *)malloc(n*n*sizeof(double));
    B = (double *)malloc(n*n*sizeof(double));
    C = (double *)malloc(n*n*sizeof(double));


    /*************************/
    /* Matriisien kokoaminen */
    /*************************/
    // A tallennetaan riveittäin.
    for (i = 0; i < n*n; i++) {
        A[i] = drand48();
    }
    // B tallennetaan sarakkeittain.
    for (i = 0; i < n*n; i++) {
        B[i] = drand48();
    }

    
    /************/
    /* Laskenta */
    /************/
    gettimeofday(&start, NULL);
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            C[i*n+j] = 0;
            for (k = 0; k < n; k++) {
                C[i*n+j] += A[i*n+k] * B[j*n+k];
            }
        }
    }
    gettimeofday(&end, NULL);

    // Aika millisekunteina.
    time = (end.tv_sec - start.tv_sec) * 1000
        + (double) (end.tv_usec - start.tv_usec) / 1000;
    printf("%f\n", time);

    
    free(A); free(B); free(C);
    return 0;
}
