#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <omp.h>

int main(int argc, char **argv) {
    double *A, *B, *C;
    double time_start, time_finish, time_elapsed;    
    int i, j;
    
    // Matriisin dimensio.
    const int n = strtol(argv[1], NULL, 10);
    // Säikeiden määrä, sama kuin lohkojen määrä.
    const int N = strtol(argv[2], NULL, 10);

    // Rivien tai sarakkeiden määrä yhtä tulomatriisin lohkoa kohti.
    const int RC_PER_BLOCK = (int) (n / sqrt((double) N));

    // Lohkojen määrä matriisin sivua kohti.
    const int BLOCKS_PER_LEN = n / RC_PER_BLOCK; 

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

    printf("Säikeitä %d.\n", N);
    printf("Yksi säie laskee %d riviä ja %d saraketta.",
           RC_PER_BLOCK, RC_PER_BLOCK);
    printf("\nSuoritettiin matriisien kertolasku A*B\n"
           "A = \n");
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            printf("%5.4f ", A[n*i+j]);
        }
        printf("\n");
    }
    
    printf("\nB = \n");
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            printf("%5.4f ", B[n*j+i]);
        }
        printf("\n");
    }
    printf("\n");
    
    /******************/
    /* Ajanotto alkaa */
    /******************/
    time_start = omp_get_wtime();
#   pragma omp parallel num_threads(N)
    {
        int ii, jj, kk, start, end;
        int id, row, col, row_start, col_start, row_max, col_max;
        id = omp_get_thread_num();

        // Koodilohko on merkitty kriittiseksi, koska muuten tulosteet
        // menevät sekaisin.
#       pragma omp critical
        {
            printf("\nSäie %d käsittelee seuraavia A:n alkioita:\n",
                   id);
            start = (id / BLOCKS_PER_LEN)
                * BLOCKS_PER_LEN
                * RC_PER_BLOCK*RC_PER_BLOCK;
            end = start + BLOCKS_PER_LEN*RC_PER_BLOCK*RC_PER_BLOCK;
            for (ii = start; ii < end; ii++) {
                printf("%5.4f ", A[ii]);
            }
            printf("\nSäie %d käsittelee seuraavia B:n alkioita:\n",
                   id);
            start = (id % BLOCKS_PER_LEN)
                * BLOCKS_PER_LEN
                * RC_PER_BLOCK*RC_PER_BLOCK;
            end = start + BLOCKS_PER_LEN*RC_PER_BLOCK*RC_PER_BLOCK;

            for (ii = start; ii < end; ii++) {
                printf("%5.4f ", B[ii]);
            }
            printf("\n");
        }
        
        /************/
        /* Laskenta */
        /************/
        row_start = (id / BLOCKS_PER_LEN) * RC_PER_BLOCK;
        row_max = row_start + RC_PER_BLOCK;
        col_start = (id % BLOCKS_PER_LEN) * RC_PER_BLOCK;
        col_max = col_start + RC_PER_BLOCK;
        for (row = row_start; row < row_max; row++) {
            for (col = col_start; col < col_max; col++) {
                C[row*n+col] = 0;
                for (kk = 0; kk < n; kk++) {
                    C[row*n+col] += A[row*n+kk]*B[col*n+kk];
                }
            }
        }
    }


    /********************/
    /* Ajanotto päättyy */
    /********************/
    time_finish = omp_get_wtime();
    time_elapsed = (time_finish - time_start) * 1000;
    // Aika millisekunteina.
    printf("%f\n", time_elapsed);

    free(A); free(B); free(C);
    return(0);
}
