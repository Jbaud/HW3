#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>

#define ROW 4
#define COLUMN 4

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


		mat = malloc(sizeof(double*)*(ROW*COLUMN));


		for(i = 0; i < (ROW*COLUMN); i++)
		{
			mat[i] = malloc(sizeof(double)*(ROW*COLUMN));
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
		columns_to_send = malloc(sizeof(int)*size);

                            base= ROW/size;
                            columns_length=ROW*COLUMN;

                            for(i = 0; i < size; i++)
                            {
                                columns_to_send[i] = base;
                            }

		k = k + columns_to_send[0];

		for(i = 1; i < size; i++)
		{
			nb_columns = columns_to_send[i];
			MPI_Send(&nb_columns, 1, MPI_INT, i, tag, MPI_COMM_WORLD);
			MPI_Send(&columns_length, 1, MPI_INT, i, tag, MPI_COMM_WORLD);

			for(j = 0; j < columns_to_send[i]; j++)
			{
				MPI_Send(&mat[k][0], ROW*COLUMN, MPI_DOUBLE, i, tag, MPI_COMM_WORLD);
				k++;
			}

		}

                        // ALGORITHM
                        double sum = 0;
                        double c= 0;

                        for(j=0; j<=ROW; j++)  
                        {
                            for(i= 0; i<=ROW; i++)
                            {
                                c=mat[i][j]/mat[j][j];
                                for(k=1; k<=n+1; k++)
                                {
                                    mat[i][k]=mat[i][k]-c*mat[j][k];
                                }

                            }
                        }

	}

	else
	{
		int i;
		int j;
		int nb_columns;
		int columns_length;
		double **local_columns;

		MPI_Recv(&nb_columns, 1, MPI_INT, source, tag, MPI_COMM_WORLD, &status);
		MPI_Recv(&columns_length, 1, MPI_INT, source, tag, MPI_COMM_WORLD, &status);
	

		local_columns = malloc(sizeof(double*)*nb_columns);

		for(i = 0; i < nb_columns; i++)
		{
			local_columns[i] = malloc(sizeof(double)*columns_length);
		}

		for(i = 0; i < nb_columns; i++)
		{
			MPI_Recv(&local_columns[i][0], columns_length, MPI_DOUBLE, source, tag, MPI_COMM_WORLD, &status);
		}
             }
	              printf("END OK\n");	
		MPI_Finalize();
		return 0;
	}
