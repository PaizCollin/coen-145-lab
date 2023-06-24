#include <omp.h>
#include <stdio.h>
#include <algorithm>
#include <bits/stdc++.h>


#define SIZE 100
#define COUNT 5

int main() {
	int a[COUNT][SIZE], c[SIZE];
	int i, j, nthreads, threads_before, threads_after;
	double itime, ftime, exec_time;
	int curMax = INT_MIN;

	for(i = 0; i < COUNT; i++) {
		for(j = 0; j < SIZE; j++) {
			a[i][j] = j*i%10;
		}
		threads_before = omp_get_num_threads();
	}

	omp_set_dynamic(0);
	omp_set_num_threads(4);
	
	itime = omp_get_wtime();

	#pragma omp parallel for shared(a,c) private(j, curMax)
	for(i = 0; i < SIZE; i++) {
		c[i] = 0;
		curMax = a[0][i];
		#pragma omp parallel for reduction(max:curMax)
		for(j = 1; j < COUNT; j++) {
			curMax = std::max(curMax, a[j][i]);
		}
		c[i] = curMax;
		nthreads = omp_get_num_threads();
	}

	ftime = omp_get_wtime();

	for(i = 0; i < SIZE; i++) {
		for(j = 0; j < COUNT; j++) {
			std::cout<< a[j][i] << " ";
		}
		printf("%d\n", c[i]);
		threads_after = omp_get_num_threads();
	}

	exec_time = ftime - itime;
	printf("\nArray Init Threads: %d\nComparison Threads: %d\nReport Threads: %d\nTime: %f\n", threads_before, nthreads, threads_after, exec_time);
	return 0;	
}
