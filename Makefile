CC=g++
CFLAGS=-g -Wall

minesweeper-ncurses: main.o game.o
	$(CC) $(CFLAGS) -o minesweeper-ncurses main.o game.o -lncurses

main.o: main.cpp
	$(CC) $(CFLAGS) -c main.cpp

game.o: game.cpp
	$(CC) $(CFLAGS) -c game.cpp


clean:
	rm -f test minesweeper-ncurses *.o 2>/dev/null