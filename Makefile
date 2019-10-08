CC=g++
CFLAGS=-g -Wall

minesweeper-ncurses: main.o game.o
	$(CC) $(CFLAGS) -o minesweeper-ncurses main.o game.o -lncurses

main.o: main.cpp game.h
	$(CC) $(CFLAGS) -c main.cpp

game.o: game.cpp game.h
	$(CC) $(CFLAGS) -c game.cpp


clean:
	rm -f test minesweeper-ncurses *.o 2>/dev/null