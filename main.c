// Barbero
// by Jadiel Zuñiga Rodriguez
// GNU/GPL License
// 20230512

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "myvar.h"

double f(double x){
	return x * x * x;
}


int main(int argn, char **argc){
	int miproc, numproc;
	MPI_Status status;
	MPI_Status status2;
	int data;
	double a, b, dx, sum, F;
	MPI_myvar range;
	MPI_myvar range2;
	int segments;
	int n_partition;
	double block;
	
	MPI_Init(&argn, &argc); /*Inicializar MPI*/
	MPI_Comm_rank(MPI_COMM_WORLD, &miproc);
	MPI_Comm_size(MPI_COMM_WORLD, &numproc);
	MPI_Barrier(MPI_COMM_WORLD);
	
	if (miproc == 0){ //master    
		if(argn < 4){
		  printf("Faltan parametros [a] [b] [range] [n]\n");
		  printf("     dx = (b - a) / n * range \n");
		  return 0;
		}
		if(sscanf(argc[1],"%lf",&a) != 1){
		  printf("Error al convertir a.\n");
		  return 0;
		}
		if(sscanf(argc[2],"%lf",&b) != 1){
		  printf("Error al convertir b.\n");
		  return 0;
		}
		if(sscanf(argc[3],"%i",&segments) != 1){
		  printf("Error al convertir segments\n");
		  return 0;
		}
		if(sscanf(argc[4],"%i",&n_partition) != 1){
		  printf("Error al convertir n_partition\n");
		  return 0;
		}
		dx = (b - a) / ((double)(segments * n_partition));
		block = (b - a) / ((double)segments);
		printf("[%lf, %lf] range=%i n=%i,  dx=%lf\n", a, b, segments, n_partition, dx);
	} //master reading command line
	
	MPI_Barrier(MPI_COMM_WORLD);
	
	if (miproc != 0) { //slaves
		//int flag_start = 1;
		double F = 0.0; // result
		data = 1;
		range.F = 0.0;
		while(1) {
			MPI_Send(&range, sizeof(range), MPI_CHARACTER, 0, 0, MPI_COMM_WORLD);
			MPI_Recv(&range, sizeof(range), MPI_CHARACTER, 0, 1, MPI_COMM_WORLD, &status);
			// integral de Riemman
			double x;
			int i = 0;
			F = 0.0;
			do {
				x = range.a + ((double)i) * range.dx;
				F += f(x) * range.dx;
				i++;
			} while(x <= range.b);
			
			range.F = F;
			//printf("%i:[%lf, %lf] dx = %lf  F = %lf\n", miproc, range.a, range.b, range.dx, range.F);
			MPI_Send(&range, sizeof(range), MPI_CHARACTER, 0, 2, MPI_COMM_WORLD);
			// Parallel processing
		}
	} /*slaves*/ else { //Master
		int flag = -1;
		int flag2 = -1;
		MPI_Request request;
		MPI_Request request2;
		double sum = 0;
		int m = 0;
		int n = 0;
		while(1) {
			if(flag == -1) {
				MPI_Irecv(&range, sizeof(range), MPI_CHARACTER, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &request);
				flag = 0;
			}
			if(flag2 == -1) {
				MPI_Irecv(&range2, sizeof(range2), MPI_CHARACTER, MPI_ANY_SOURCE, 2, MPI_COMM_WORLD, &request2);
				flag2 = 0;
			}
			MPI_Test(&request, &flag, &status);
			MPI_Test(&request2, &flag2, &status2);
			if(flag == 1) {
				if (status.MPI_SOURCE != -1){
					// sending information
					// segmentar la información para enviarla al nodo disponibles
					//printf("%i: %lf\n", miproc, range.F);
					range.a = a + m * block;
					range.b = a + (m + 1) * block;
					range.dx = dx;
					range.F = 0.0;
					m++;
					
					MPI_Send(&range, sizeof(range), MPI_CHARACTER, status.MPI_SOURCE, 1, MPI_COMM_WORLD);
				}
				flag = -1;
			}
			if (flag2 == 1) {
				if (status.MPI_SOURCE != -1){
                    sum += range2.F;
                    n++;
                    //printf("N:%d, sum:%lf, df:%lf\n", n,sum, range2.F);
                }
                flag2 = -1;
			}
			// stop condition
			//if(a + n * dx >= b){
			if (n == segments) {
				printf("%i, F = %le\n", n, sum);
				break;
			}
		} //while(1)
	}//Master
	
	MPI_Abort(MPI_COMM_WORLD, MPI_SUCCESS);
	// MPI_Finalize();
	return 0;
}
