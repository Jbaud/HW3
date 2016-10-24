all: main

main: main.o
	mpicc  -o main main.o
main.o: main.c
	mpicc -c main.c
clean:
	rm -f  main.o 
run:
	mpirun -np 4 ./main