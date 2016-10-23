#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>

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
	int source = 0;
	int tag = 0;

	/*Start MPI*/
	MPI_Init(&argc, &argv);

	/*Get the rank of the current process*/
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	/*Get the total number of processes*/
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	if(rank == source)
	{
		double **mat;
		double *vector;
		int *columns_to_send;
		int k = 0;
		int i;
		int j;
		int base;
		int remainder;
		int nb_columns;
		int columns_length;

                            /*
		mat = malloc(sizeof(double*)*(ROW*COLUMN));


		for(i = 0; i < (ROW*COLUMN); i++)
		{
			mat[i] = malloc(sizeof(double)*(ROW*COLUMN));
		}
                            */
                          mat = alloc_2d_double(ROW,COLUMN);

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
		columns_to_send = malloc(sizeof(int)*size);

		base= ROW/size;
		columns_length=ROW*COLUMN;

		for(i = 0; i < size; i++)
		{
			columns_to_send[i] = base;
		}

		//k = k + columns_to_send[0];
                           k =  0;

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
                                    MPI_Send(&base, 1, MPI_INT, i, tag, MPI_COMM_WORLD);
                                    MPI_Send(&(mat[0][0]), size*base, MPI_INT, i, 0, MPI_COMM_WORLD);
                                }

                          /*
		for(i = 1; i < size; i++)
		{
			nb_columns = columns_to_send[i];
                                          printf("This is COLUMN : %d and nb_columns: %d\n",i,columns_to_send[i] );
			MPI_Send(&nb_columns, 1, MPI_INT, i, tag, MPI_COMM_WORLD);
			MPI_Send(&columns_length, 1, MPI_INT, i, tag, MPI_COMM_WORLD);

			for(j = 0; j < columns_to_send[i]; j++)
			{
				MPI_Send(&mat[k][0], ROW*COLUMN, MPI_DOUBLE, i, tag, MPI_COMM_WORLD);
				k++;
			}

		}
                           */
		for (i = 1; i < size; ++i)
		{
			MPI_Send(&(mat[0][0]), ROW*COLUMN, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);        
		}


	}

	else
	{
		int i;
		int j;
		int nb_columns;
		int columns_length;
		double **local_columns;
                           int base_copie;
                           int my_work;

                            MPI_Recv(&my_work, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
                            MPI_Recv(&base_copie, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);


                            int **toCompute_copie;
                            toCompute_copie = alloc_2d_int(size,base_copie);


                           MPI_Recv(&(toCompute_copie[0][0]), size*base_copie, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
                         
		double **A_copie = alloc_2d_double(ROW,COLUMN);

		MPI_Recv(&(A_copie[0][0]), ROW*COLUMN, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, &status);

		for (i = 0; i < ROW; ++i)
		{
			for ( j = 0; j < COLUMN; ++j)
			{
				printf("%.1lf ",A_copie[i][j]); 
			}
			printf("\n");
		}

	}
	printf("END OK\n");	
	MPI_Finalize();
	return 0;
}
