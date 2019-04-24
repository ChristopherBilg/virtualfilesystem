# The final executable file to be run
output: main.o
	gcc -o output main.o -g

# The only c file needed to implement/test the virtual disk
main.o: main.c
	gcc -o main.o main.c -c -g
