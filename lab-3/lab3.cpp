#include <omp.h>
#include <stdio.h>

#define SIZE 100000

int main() {
        int a[SIZE], b[SIZE], c[SIZE];
        int i, j, nthreads, threads_before, threads_after;
        double itime, ftime, exec_time;

        for(i = 0; i < SIZE; i++) {
                a[i] = i;
                b[i] = SIZE - i;
                threads_before = omp_get_num_threads();
        }

        omp_set_dynamic(0);
        omp_set_num_threads(16);

        itime = omp_get_wtime();

        #pragma omp parallel for
        for(i = 0; i < SIZE; i++) {
                c[i] = a[i] + b[i];
                nthreads= omp_get_num_threads();
        }

        #pragma omp barrier
        for(i = 0; i < SIZE; i++) {
                printf("%d ", c[i]);
                threads_after = omp_get_num_threads();
        }

        ftime = omp_get_wtime();
        exec_time = ftime - itime;
        printf("\nArray Init Threads: %d\nArray Addition Threads: %d\nReport Threads: %d\nTime: %f\n", threads_before, nthreads, threads_after, exec_time);
        return 0;
}
