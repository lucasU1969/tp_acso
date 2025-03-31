# sim: shell.c sim.c 
# 	gcc -g -O0 $^ -o $@

# .PHONY: clean
# clean:
# 	rm -rf *.o *~ sim
CC = gcc
CFLAGS = -g -O0
SRC = sim.c functions.c shell.c
OBJ = sim.o functions.o shell.o

sim: $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o sim

sim.o: sim.c functions.h shell.h
	$(CC) $(CFLAGS) -c sim.c

functions.o: functions.c functions.h
	$(CC) $(CFLAGS) -c functions.c

shell.o: shell.c shell.h
	$(CC) $(CFLAGS) -c shell.c

.PHONY: clean
clean:
	rm -rf *.o *~ sim
