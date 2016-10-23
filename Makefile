all: copie

copie: copie.o
	mpicc  -o copie copie.o
copie.o: copie.c
	mpicc -c copie.c
clean:
	rm -f  copie.o 
run:
	mpirun -np 4 ./copie