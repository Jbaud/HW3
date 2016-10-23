#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <time.h>

#define ROW 4
#define COLUMN 4

double **alloc_2d_double(int rows, int cols) {
	double *data = (double *)malloc(rows*cols*sizeof(double));
	double **array= (double **)malloc(rows*sizeof(double*));
	for (int i=0; i<rows; i++)
		array[i] = &(data[cols*i]);

	return array;
}

int **alloc_2d_int(int rows, int cols) {
	int *data = (int *)malloc(rows*cols*sizeof(int));
	int **array= (int **)malloc(rows*sizeof(int*));
	for (int i=0; i<rows; i++)
		array[i] = &(data[cols*i]);

	return array;
}

void printArray_int(int rows, int cols, int array[rows][cols]){

	int i,j;

	for(i = 0; i < rows; i++)
	{
		for(j = 0; j < cols; j++) 
		{
			array[i][j]=0;
			printf("%d ", array[i][j]); 
		}
		printf("\n");
	}
}

void printArray_double(int rows, int cols, double array[rows][cols]){

	int i,j;

	for(i = 0; i < rows; i++)
	{
		for(j = 0; j < cols; j++) 
		{
			array[i][j]=0;
			printf("%lf ", array[i][j]); 
		}
		printf("\n");
	}
}

int main(int argc, char **argv)
{

	int size;
	int rank;
	MPI_Status status;

	/*Start MPI*/
	MPI_Init(&argc, &argv);

	/*Get the rank of the current process*/
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	/*Get the total number of processes*/
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	if(rank == 0)
	{
		double **mat;
		double *vector;
		int i;
		int j;
		int base;
		int nb_columns;


		mat = alloc_2d_double(ROW,COLUMN);
		vector = malloc(sizeof(double)*ROW);
		//random vector to make input easier
		srand((time(NULL)));

		for(i = 0; i < ROW; i++)
		{
			vector[i] = ((rand()%(9))+1)/10.0;  
		}


		FILE *file;
		file=fopen("test4_4.txt", "r");

		for(i = 0; i < ROW; i++)
		{
			for(j = 0; j < COLUMN; j++) 
			{
				if (!fscanf(file, "%lf", &mat[i][j])) 
					break;

				printf("%.1lf ",mat[i][j]); 
			}
			printf("\n");
		}
		fclose(file);
		//:=========================== END OF READING FILE =============================


		// sending infos to other processes

		base= ROW/size;
		int k =  0;

		int **toCompute;
		toCompute = alloc_2d_int(size,base);

		for (i = 0; i < size; ++i)
		{
			for ( j = 0; j < base; ++j)
			{
				toCompute[i][j]= k;   
				k++;
			}

		}
		printf("\n");

		for(i = 0; i < size; i++)
		{
			for(j = 0; j < base; j++) 
			{
				printf("%.d ",toCompute[i][j]); 
			}
			printf("\n");
		}

		for (i = 1; i < size; ++i)
		{
			MPI_Send(&i, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
			MPI_Send(&base, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
			MPI_Send(&(mat[0][0]), size*base, MPI_INT, i, 0, MPI_COMM_WORLD);
		}

		for (i = 1; i < size; ++i)
		{
			MPI_Send(&(mat[0][0]), ROW*COLUMN, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);        
		}
		// now we send the vector
		for (i = 1; i < size; ++i)
		{
			MPI_Send(&(vector[0]), ROW, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);        
		}

		double c =0;


		printf("rank : %d , j=0, j<%d\n",rank,base );
		// we use a similar algorithm to the sequential code
		for(j = 0 ; j < base; j++)
		{
			for(i = 0; i<ROW; i++)
			{
				if(i > j)
				{
					c = mat[i][j]/mat[j][j];

					for(k = 0; k<ROW; k++)
					{

						mat[i][k] = mat[i][k] - c*mat[j][k];
					}
					vector[i] = vector[i] - c*vector[j];
				}
			}
		}


		printf("this is the output for rank 0:\n");
		for(i = 0; i < ROW; i++)
		{
			for(j = 0; j < COLUMN; j++) 
			{
				printf("%.1lf ",mat[i][j]); 
			}
			printf("\n");
		}

	}

	else
	{
		int i;
		int j;
		int nb_columns;
		double **local_columns;
		double *local_vector;
		int base_copie;
		int my_work;


		MPI_Recv(&my_work, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
		MPI_Recv(&base_copie, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);


		int **toCompute_copie;
		toCompute_copie = alloc_2d_int(size,base_copie);

		local_vector  = malloc(sizeof(double)*ROW);


		MPI_Recv(&(toCompute_copie[0][0]), size*base_copie, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);

		double **A_copie = alloc_2d_double(ROW,COLUMN);

		MPI_Recv(&(A_copie[0][0]), ROW*COLUMN, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, &status);

		MPI_Recv(&(local_vector[0]), COLUMN, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, &status);

		/*
		   for (i = 0; i < ROW; ++i)
		   {
		   for ( j = 0; j < COLUMN; ++j)
		   {
		   printf("%.1lf ",A_copie[i][j]); 
		   }
		   printf("\n");
		   }
		 */
		// algorithm in //
		double c = 0;
		int k;
		printf("i : %d , my_work*base_copie=%d, (my_work*base_copie)+base_copie=%d\n",my_work,my_work*base_copie,(my_work*base_copie)+base_copie );
		for(j = my_work*base_copie ; j < ((my_work*base_copie)+base_copie); j++)
		{

			for(i = 0; i<ROW; i++)
			{
				if(i > j)
				{
					c = A_copie[i][j]/A_copie[j][j];

					for(k = 0; k<ROW; k++)
					{

						A_copie[i][k] = A_copie[i][k] - c*A_copie[j][k];
					}

					local_vector[i] = local_vector[i] - (c*local_vector[j]);
				}
			}
		}
		// The computation is finished we send back our response
		if (rank==1)
		{
			printf("this is the output for rank 1:\n");
			for(i = 0; i < ROW; i++)
			{
				for(j = 0; j < COLUMN; j++) 
				{
					printf("%.1lf ",A_copie[i][j]); 
				}
				printf("\n");
			}
		}

	}
	printf("END OK\n");	
	MPI_Finalize();
	return 0;
}
