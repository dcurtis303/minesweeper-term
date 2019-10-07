#include <cstdlib>
#include <ctime>
#include <ncurses.h>

#include "game.h"

int main(int argc, char *argv[])
{
    initscr();
    cbreak();
    curs_set(0);
    noecho();
    keypad(stdscr, TRUE);
    mousemask(BUTTON1_CLICKED, NULL);
    start_color();

    Game game(16, 30, 99, time(NULL));
    game.run();

    endwin();
    return 0;
}