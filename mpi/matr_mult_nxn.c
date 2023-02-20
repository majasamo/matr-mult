#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <mpi.h>

#define ROOT 0
#define n 8

int main(int argc, char **argv) {
    double *A, *B, *C, *A_sendbuf, *B_sendbuf;
    double local_start, local_finish, local_elapsed, elapsed;
    int N, id, i, j, k;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &N);

    // Rivien tai sarakkeiden määrä yhtä tulomatriisin lohkoa kohti.
    const int RC_PER_BLOCK = (int) (n / sqrt((double) N));
    // Lohkojen määrä matriisin sivua kohti.
    const int BLOCKS_PER_LEN = n / RC_PER_BLOCK; 

    double A_local[RC_PER_BLOCK*n], B_local[RC_PER_BLOCK*n], 
        C_local[RC_PER_BLOCK*RC_PER_BLOCK];
    
    if (id == ROOT) {
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

        
        /*******************/
        /* Lähetyspuskurit */
        /*******************/
        // Koska Scatter-rutiini ei salli päällekkäisen datan
        // lähettämistä eri prosesseille, eri prosessien tarvitsema
        // data laitetaan lähetyspuskuriin yksinkertaisesti peräkkäin.
        A_sendbuf = (double *)malloc(N
                                     *RC_PER_BLOCK*n*sizeof(double));
        B_sendbuf = (double *)malloc(N
                                     *RC_PER_BLOCK*n*sizeof(double));
        // Tallennetaan A lähetyspuskuriin.
        for (i = 0; i < N; i++) {
            // Yhdessä lohkossa on RC_PER_BLOCK*n alkiota.
            for (j = 0; j < RC_PER_BLOCK*n; j++) {
                A_sendbuf[i*RC_PER_BLOCK*n+j] = 
                    A[(i/BLOCKS_PER_LEN)*RC_PER_BLOCK*n+j];
            }
        }
        // Tallennetaan B lähetyspuskuriin.
        for (i = 0; i < N; i++) {
            // Yhdessä lohkossa on RC_PER_BLOCK*n alkiota.
            for (j = 0; j < RC_PER_BLOCK*n; j++) {
                B_sendbuf[i*RC_PER_BLOCK*n+j] = 
                    B[(i%BLOCKS_PER_LEN)*RC_PER_BLOCK*n+j];
            }
        }
    }

    /******************/
    /* Ajanotto alkaa */
    /******************/
    MPI_Barrier(MPI_COMM_WORLD);
    local_start = MPI_Wtime();

    
    /************************/
    /* Jaetaan prosesseille */
    /************************/
    MPI_Scatter(A_sendbuf, RC_PER_BLOCK*n, MPI_DOUBLE,
                A_local, RC_PER_BLOCK*n, MPI_DOUBLE,
                ROOT, MPI_COMM_WORLD);
    MPI_Scatter(B_sendbuf, RC_PER_BLOCK*n, MPI_DOUBLE,
                B_local, RC_PER_BLOCK*n, MPI_DOUBLE,
                ROOT, MPI_COMM_WORLD);

    
    /************/
    /* Laskenta */
    /************/
    for (i = 0; i < RC_PER_BLOCK; i++) {
        for (j = 0; j < RC_PER_BLOCK; j++) {
            C_local[i*RC_PER_BLOCK+j] = 0;
            for (k = 0; k < n; k++) {
                C_local[i*RC_PER_BLOCK+j] += 
                    A_local[i*n+k] * B_local[j*n+k];
            }
        }
    }


    /******************/
    /* Tulosten keruu */
    /******************/
    MPI_Gather(C_local, RC_PER_BLOCK*RC_PER_BLOCK, MPI_DOUBLE,
               C, RC_PER_BLOCK*RC_PER_BLOCK, MPI_DOUBLE,
               ROOT, MPI_COMM_WORLD);

    
    /********************/
    /* Ajanotto päättyy */
    /********************/
    local_finish = MPI_Wtime();
    local_elapsed = (local_finish - local_start) * 1000;
    MPI_Reduce(&local_elapsed, &elapsed, 1, MPI_DOUBLE,
               MPI_MAX, 0, MPI_COMM_WORLD);
    // Aika millisekunteina.
    if (id == ROOT)
        printf("%f\n", elapsed);
    
    if (id == ROOT) {
        free(A); free(B); free(C); free(A_sendbuf); free(B_sendbuf);
    }
    MPI_Finalize();
    return(0);
}
