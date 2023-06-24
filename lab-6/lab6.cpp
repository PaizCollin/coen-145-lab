#include <omp.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <mpi.h>

#define SIZE 100

int main() {
	int world_size, world_rank;

	std::ofstream myfile;
	myfile.open("res.txt");

	MPI_Init(NULL, NULL);
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
	MPI_Status status;

	int a[SIZE], b[SIZE], c[SIZE], d[SIZE];
	int elements_per_proc, n_elements_recvd, index, i;
	double itime, ftime, exec_time;

	for(int i = 0; i < SIZE; i++) {
		a[i] = rand()%10;
		b[i] = rand()%10;
	}

	// add arrays a, b
	if(world_rank == 0) {
		elements_per_proc = SIZE / world_size;

		if(world_size > 1) {
			for(i = 1; i < world_size; i++) {
				index = (i-1) * elements_per_proc;

				MPI_Send(&elements_per_proc, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
				MPI_Send(&index, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
				MPI_Send(&a[index], elements_per_proc, MPI_INT, i, 0, MPI_COMM_WORLD);
				MPI_Send(&b[index], elements_per_proc, MPI_INT, i, 0, MPI_COMM_WORLD);
			}

			index = (world_size-1) * elements_per_proc;
			std::cout << "Processor " << world_rank << " out of " << world_size << " performing elements " << index << " through " << SIZE << std::endl;

			for(int k = index; k < SIZE; k++) {
				c[k] = a[k] + b[k];
			}

			for(i = 1; i < world_size; i++) {
				MPI_Recv(&index, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &status);
				MPI_Recv(&c[index], elements_per_proc, MPI_INT, i, 0, MPI_COMM_WORLD, &status);
			}

			for(int i = 0; i < SIZE; i++) {
				myfile << a[i] << " + " << b[i] << " = " << c[i] << std::endl;
			}

			myfile.close();
		}
	} else {
		MPI_Recv(&elements_per_proc, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
		MPI_Recv(&index, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
		MPI_Recv(&a[index], elements_per_proc, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
		MPI_Recv(&b[index], elements_per_proc, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
		std::cout << "Processor " << world_rank << " out of " << world_size << " performing elements " << index << " through " << index+elements_per_proc << std::endl;

		for(int k = index; k < index + elements_per_proc; k++) {
			c[k] = a[k] + b[k];
		}
		
		MPI_Send(&index, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
		MPI_Send(&c[index], elements_per_proc, MPI_INT, 0, 0, MPI_COMM_WORLD);
	}

	MPI_Finalize();

	return 0;	
}
