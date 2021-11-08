CC = g++
CFLAGS = -Wall -g

execute_ground: execute_ground.o
	$(CC) $(CFLAGS) -o execute_ground execute_ground.o -lssh

execute.o: execute.cpp
	$(CC) $(CFLAGS) -c execute.cpp

clean: 
	rm -rf execute_ground execute_ground.o