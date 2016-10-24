#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <time.h>

#define ROW 8
#define COLUMN 8

// create contiguous memory for the array sharing
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

int main(int argc, char **argv)
{

	int size;
	int rank;
	MPI_Status status;
	// this will be used to send columns by columns
	MPI_Datatype columns_type;
             double starttime, endtime; 
	/*Start MPI*/
	MPI_Init(&argc, &argv);

	/*Get the rank of the current process*/
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	/*Get the total number of processes*/
	MPI_Comm_size(MPI_COMM_WORLD, &size);


	//vector will allow us to send  columns when the processes are finished
	MPI_Type_vector(ROW, 1, ROW, MPI_DOUBLE,&columns_type);
	MPI_Type_commit(&columns_type);


	if(rank == 0)
	{
		double **mat;
		double *vector;
		int i;
		int j;
		int base;


		mat = alloc_2d_double(ROW,COLUMN);
		vector = malloc(sizeof(double)*ROW);

		//random vector to make input easier
		srand((time(NULL)));

		for(i = 0; i < ROW; i++)
		{
			vector[i] = ((rand()%(9))+1)/10.0;  
		}


		FILE *file;
		file=fopen("test8_8.txt", "r");

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
		// we assume even, to be  improved...
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

                            // beginning of //
		/*
		We are going to sen to all the other processes 
			The matrix 
			Their rank
			The base ( <---->)
		*/

                           starttime = MPI_Wtime(); 

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

		//printf("rank : %d , j=0, j<%d\n",rank,base );

		// we use a similar algorithm to the sequential code
		/*
		Since process 0 -> j =0 always
		*/
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
	
		// We are waiiting for the other jobs to finish
		// This allow to ordinate the matrix in the right order of i 
		// This is done with the modulo 
		k=1;
		i=base;
		while (i < ROW)
		{

			MPI_Recv(&mat[0][i],1,columns_type,k,i,MPI_COMM_WORLD,&status);
			printf("RECEIVED FROM MAT: %d \n",i );
			i++;
			if (i%base==0)
			{
				k++;
			}
		}

		// receiving the vector
		//same structure
		k=1;
		i=base;
		while (i < ROW)
		{

			MPI_Recv(&vector[i],1,MPI_DOUBLE,k,i,MPI_COMM_WORLD,&status);
			printf("RECEIVED FROM VECTOR %d \n",i );
			i++;
			if (i%base==0)
			{
				k++;
			}
		}

		printf("\nUpper triangular matrix: \n");
		for(i = 0; i < ROW; i++)
		{
			for(j = 0; j < COLUMN; j++) 
			{
				printf("%.1lf ",mat[i][j]); 
			}
			printf("\n");
		}
                        
                            printf("---------VECTOR-------\n");
                            for (int i = 0; i < ROW; ++i)
                            {
                                printf("%lf ",vector[i]);
                            }
                            printf("\n");
                  

		double *solution = malloc(sizeof(double)*ROW);

                            if(solution == NULL)
                            {
                                printf("Error: unable to allocate memory in process 0 for solution storage!\n");
                                MPI_Finalize();
                                exit(EXIT_FAILURE);
                            }
                            
		solution[ROW] = vector[ROW]-1/mat[ROW-1][ROW-1];
                            

                            double sum;
                            int x=0;
		for(i = (ROW-1); i >= 0; i--)
		{
			sum = 0;

			for(j=i; j < ROW; j++)
			{
				sum=sum+mat[i][j]*solution[j];
			}
                                        if (mat[i][i] != 0)
                                        {  
                                           solution[i]=(vector[i]-sum)/mat[i][i];
                                        }

                          // end of algorithm, the rest is just printing              
                         endtime   = MPI_Wtime();          

                            			
		}
		printf("%d\n",x);
		for(i = 0; i < ROW; i++)
		{
			printf("Solution[%d]: %3.2f\n",i, solution[i]);
		}
                        
                           printf("That took %f seconds\n",endtime-starttime); 
	}

		else
		{
			int i;
			int j;
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

			// algorithm in //
			double c = 0;
			int k;
			//printf("i : %d , my_work*base_copie=%d, (my_work*base_copie)+base_copie=%d\n",my_work,my_work*base_copie,(my_work*base_copie)+base_copie );

			// the range to me make sure that each process only compute the assignated  columns
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
		
			// now we need the send the COLUMNS back 

			//printf("I am gonna send it from :i : %d , my_work*base_copie=%d, (my_work*base_copie)+base_copie=%d\n",my_work,my_work*base_copie,(my_work*base_copie)+base_copie );
			for(j = my_work*base_copie ; j < ((my_work*base_copie)+base_copie ); j++)
			{
				printf("increment\n");
				MPI_Send(&A_copie[0][j], 1, columns_type,0,j,MPI_COMM_WORLD);
			}

			// now we send  back the vector
			//printf(" worker :%d is sending his vector\n", my_work);
                                       
			for(j = my_work*base_copie ; j < ((my_work*base_copie)+base_copie ); j++)
			{
				printf("sending\n");
				MPI_Send(&local_vector[j], 1, MPI_DOUBLE, 0, j, MPI_COMM_WORLD);
			}


		}



		printf("END OK proc number %d\n",rank);	
		MPI_Finalize();
		return 0;
	}
