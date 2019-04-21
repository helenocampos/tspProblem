OBJS	= main.o mtwister.o TSPutil.o
SOURCE	= main.c mtwister.c TSPutil.c
HEADER	= TSPutil.h mtwister.h
OUT	= tspProblem
CC	 = gcc
FLAGS	 = -g -c -Wall
LFLAGS	 = 

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS)

main.o: main.c
	$(CC) $(FLAGS) main.c -std=c99

mtwister.o: mtwister.c
	$(CC) $(FLAGS) mtwister.c -std=c99

TSPutil.o: TSPutil.c
	$(CC) $(FLAGS) TSPutil.c -std=c99


clean:
	rm -f $(OBJS) $(OUT)