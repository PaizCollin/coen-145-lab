#include <stdio.h>
#include <stdlib.h>

#define SIZE 100

int main() {
	int a[SIZE][SIZE], b[SIZE][SIZE], c[SIZE][SIZE], d[SIZE][SIZE];
	int i,j,k,temp;

	// randomize matrices A and B, init C and D to 0
	for(i = 0; i < SIZE; i++) {
		for(j = 0; i < SIZE; i++) {
			a[i][j] = rand()%10;
			b[i][j] = rand()%10;
			c[i][j] = 0;
			d[i][j] = 0;
		}
	}

	// multiply A*B into matrix C using OpenACC
	#pragma acc data copyin(a[0:SIZE][0:SIZE], b[0:SIZE][0:SIZE]) copyout(c[0:SIZE][0:SIZE])
	#pragma acc kernels loop 
    for(i = 0; i < SIZE; i++) {
		for(j = 0; j < SIZE; j++) {
			temp = 0;
			#pragma acc loop reduction(+:temp)
			for(k = 0; k < SIZE; k++) {
				temp += a[i][k] * b[k][j];
			}
			c[i][j] = temp;
		}
	}

	// multiply A*B into matrix D on the host
    for(i = 0; i < SIZE; i++) {
		for(j = 0; j < SIZE; j++) {
			for(k = 0; k < SIZE; k++) {
				d[i][j] += a[i][k] * b[k][j];
			}
		}
	}

	// compare result matrices C and D
	for (i = 0; i < SIZE; i++) {
		for (j = 0; j < SIZE; j++) {
			if(c[i][j] != d[i][j]) {
				printf("%d != %d\n", c[i][j], d[i][j]);
			}
		}
	}
	return 0;	
}