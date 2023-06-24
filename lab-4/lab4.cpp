#include <omp.h>
#include <stdio.h>
#include <math.h>

#define SIZE 1000

int main() {
	double a[SIZE];
	int i, j, nthreads, threads_before, threads_after;
	double itime, ftime, exec_time;

	omp_set_dynamic(0);
	omp_set_num_threads(8);
	a[0] = 1.0;
	
	itime = omp_get_wtime();		// get start time of parallelization

	#pragma omp parallel for ordered schedule(static)
		for(i = 2; i < SIZE; i++) {
			#pragma omp ordered
			a[i-1] = a[i-2] + ((pow(-1.0, i+1)) / i);
			nthreads = omp_get_num_threads();
		}
	

	#pragma omp barrier
	ftime = omp_get_wtime();		// get end time of parallelization

	// report results
	for(i = 0; i < SIZE-1; i++) {
		printf("%f ", a[i]);
		threads_after = omp_get_num_threads();
	}

	
	exec_time = ftime - itime;
	printf("\nThreads during parallelization: %d\nThreads during report: %d\nTime: %f\n", nthreads, threads_after, exec_time);
	return 0;	
}
