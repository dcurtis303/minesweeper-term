#include <iostream>
#include <ncurses.h>

#include "game.h"

#define CBKGND 246
#define CBKGND2 242

CPDef cpdef[] = {
    {C_MENU, 255, CBKGND2},
    {C_FLAG, 124, CBKGND2},
    {C_UNRVLD, CBKGND, CBKGND2},
    {C_ENDER, CBKGND, 52},
    {C_MINE, 232, CBKGND},
    {C_NUM0, CBKGND, CBKGND},
    {C_NUM1, 21, CBKGND},
    {C_NUM2, 28, CBKGND},
    {C_NUM3, 88, CBKGND},
    {C_NUM4, 19, CBKGND},
    {C_NUM5, 52, CBKGND},
    {C_NUM6, 29, CBKGND},
    {C_NUM7, 16, CBKGND},
    {C_NUM8, 236, CBKGND}};
int cpdef_size = sizeof(cpdef) / sizeof(CPDef);

Game::Game(int r, int c, int m, int s)
{
    rows = r;
    irow = r - 1;
    cols = c;
    icol = c - 1;
    mines = m;

    grid = new Tile[rows * cols]();

    changes = 0;

    init_colors();

    seed = s;
    srand(seed);

    for (int i = 0; i < mines; i++)
    {
        while (true)
        {
            int mine_row = rrng(rows);
            int mine_col = rrng(cols);

            if (!IsMine(mine_row, mine_col))
            {
                grid[index(mine_row, mine_col)].is_mine = true;
                break;
            }
        }
    }

    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
            if (!IsMine(i, j))
                grid[index(i, j)].adjacent_mine_count = CountAdjacentMines(i, j);

    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
            grid[index(i, j)].is_revealed = false;
}

Game::~Game()
{
    delete grid;
}

void Game::init_colors()
{
    for (int i = 0; i < cpdef_size; i++)
        init_pair(cpdef[i].cp, cpdef[i].fg, cpdef[i].bg);
}

int Game::CountAdjacentMines(int i, int j)
{
    int c = 0;

    int nis = i == 0 ? 0 : i - 1;
    int nie = i == irow ? irow : i + 1;
    int njs = j == 0 ? 0 : j - 1;
    int nje = j == icol ? icol : j + 1;

    for (int ip = nis; ip <= nie; ip++)
        for (int jp = njs; jp <= nje; jp++)
            if (IsMine(ip, jp))
                c++;

    return c;
}

void Game::MatchBlank()
{
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
            if (IsZero(i, j) && IsRevealed(i, j))
                RevealAdjacentTiles(i, j);
}

void Game::MatchUnrevealed()
{
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
            if (IsNumber(i, j))
            {
                int c = CountAdjacentUnrevealed(i, j);
                c += CountAdjacentFlagged(i, j);
                if (IsRevealed(i, j) && (grid[index(i, j)].adjacent_mine_count == c))
                    FlagAdjacentMines(i, j);
            }
}

void Game::MatchFlagged()
{
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
            if (IsNumber(i, j) && IsRevealed(i, j))
                if (grid[index(i, j)].adjacent_mine_count == CountAdjacentFlagged(i, j))
                    RevealAdjacentTiles(i, j);
}

void Game::MatchPatterns1()
{
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            int m1_cnt = IsMissingFlags(i, j);
            if (m1_cnt != -1)
            {
                int nis = i == 0 ? 0 : i - 1;
                int nie = i == irow ? irow : i + 1;
                int njs = j == 0 ? 0 : j - 1;
                int nje = j == icol ? icol : j + 1;

                for (int ip = nis; ip <= nie; ip++)
                {
                    for (int jp = njs; jp <= nje; jp++)
                    {
                        if (!((ip == i) && (jp == j)))
                        {
                            int m2_cnt = IsMissingFlags(ip, jp);
                            if (m2_cnt != -1)
                            {
                                int t1_cnt = ListAdjacentUnrevealed(i, j, 1);
                                int t2_cnt = ListAdjacentUnrevealed(ip, jp, 2);
                                int match_cnt = CompareLAULists(t1_cnt, t2_cnt);
                                if (match_cnt == t1_cnt)
                                    if (grid[index(ip, jp)].adjacent_mine_count == m2_cnt + 1)
                                        //if (a(i, j) == m2_cnt + 1)
                                        RevealLAUList2(t1_cnt, t2_cnt);
                            }
                        }
                    }
                }
            }
        }
    }
}

void Game::MatchPatterns2()
{
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
            if (IsMissingFlags(i, j) != -1)
                if (grid[index(i, j)].adjacent_mine_count == CountAdjacentUnrevealed(i, j))
                    FlagAdjacentMines(i, j);
}

int Game::ListAdjacentUnrevealed(int i, int j, int l)
{
    int lau_cnt = 0;

    int nis = i == 0 ? 0 : i - 1;
    int nie = i == irow ? irow : i + 1;
    int njs = j == 0 ? 0 : j - 1;
    int nje = j == icol ? icol : j + 1;

    for (int ip = nis; ip <= nie; ip++)
    {
        for (int jp = njs; jp <= nje; jp++)
        {
            if (!(ip == i && jp == j))
            {
                if (IsUnrevealed(ip, jp) && !IsFlagged(ip, jp))
                {
                    if (l == 1)
                    {
                        lau1[lau_cnt][0] = ip;
                        lau1[lau_cnt][1] = jp;
                    }
                    else
                    {
                        lau2[lau_cnt][0] = ip;
                        lau2[lau_cnt][1] = jp;
                    }
                    lau_cnt += 1;
                }
            }
        }
    }

    return lau_cnt;
}

bool Game::InLAUList1(int i, int j, int c1)
{
    bool ill1 = false;

    for (int r = 0; r < c1; r++)
        if (lau1[r][0] == i && lau1[r][1] == j)
            ill1 = true;

    return ill1;
}

int Game::CompareLAULists(int c1, int c2)
{
    int cll_cnt = 0;

    for (int r = 0; r < c2; r++)
        if (InLAUList1(lau2[r][0], lau2[r][1], c1))
            cll_cnt += 1;

    return cll_cnt;
}

void Game::RevealLAUList2(int c1, int c2)
{
    for (int r = 0; r < c2; r++)
        if (!InLAUList1(lau2[r][0], lau2[r][1], c1))
            SetTile(lau2[r][0], lau2[r][1], E_UNCOVER, "RevealLAUList2");
}

int Game::IsMissingFlags(int i, int j)
{
    int imf = -1;

    if (IsNumber(i, j) && IsRevealed(i, j))
    {
        int imf_cnt = CountAdjacentFlagged(i, j);
        if (imf_cnt != grid[index(i, j)].adjacent_mine_count)
            imf = imf_cnt;
    }

    return imf;
}

int Game::CountAdjacentUnrevealed(int i, int j)
{
    int c = 0;

    int nis = i == 0 ? 0 : i - 1;
    int nie = i == irow ? irow : i + 1;
    int njs = j == 0 ? 0 : j - 1;
    int nje = j == icol ? icol : j + 1;

    for (int ip = nis; ip <= nie; ip++)
        for (int jp = njs; jp <= nje; jp++)
            if (IsUnrevealed(ip, jp))
                c++;

    return c;
}

int Game::CountAdjacentFlagged(int i, int j)
{
    int c = 0;

    int nis = i == 0 ? 0 : i - 1;
    int nie = i == irow ? irow : i + 1;
    int njs = j == 0 ? 0 : j - 1;
    int nje = j == icol ? icol : j + 1;

    for (int ip = nis; ip <= nie; ip++)
        for (int jp = njs; jp <= nje; jp++)
            if (IsFlagged(ip, jp))
                c++;

    return c;
}

void Game::FlagAdjacentMines(int i, int j)
{
    int nis = i == 0 ? 0 : i - 1;
    int nie = i == irow ? irow : i + 1;
    int njs = j == 0 ? 0 : j - 1;
    int nje = j == icol ? icol : j + 1;

    for (int ip = nis; ip <= nie; ip++)
        for (int jp = njs; jp <= nje; jp++)
            if (IsUnrevealed(ip, jp))
                SetTile(ip, jp, E_FLAG, "FlagAdjacentMines");
}

void Game::RevealAdjacentTiles(int i, int j)
{
    int nis = i == 0 ? 0 : i - 1;
    int nie = i == irow ? irow : i + 1;
    int njs = j == 0 ? 0 : j - 1;
    int nje = j == icol ? icol : j + 1;

    for (int ip = nis; ip <= nie; ip++)
        for (int jp = njs; jp <= nje; jp++)
            if (IsUnrevealed(ip, jp) && !IsFlagged(ip, jp))
                SetTile(ip, jp, E_UNCOVER, "RevealAdjacentTiles");
}

void Game::SetTile(int i, int j, char value, const char *who)
{
    //const char *s;

    switch (value)
    {
    case E_UNCOVER:
        grid[index(i, j)].is_revealed = true;
        //s = "Uncover";
        break;
    case E_FLAG:
        grid[index(i, j)].is_flagged = true;
        //s = "Flag";
        break;
    case E_ENDER:
        grid[index(i, j)].is_ender = true;
        //s = "End Mine";
        break;
    }

    changes++;
    //printf("(%d, %d) = %s <- %s\n", i, j, s, who);
}

void Game::RandomPress(int &rpi, int &rpj)
{
    int x = 0, i, j;

    while (true)
    {
        i = rrng(rows);
        j = rrng(cols);

        if (IsUnrevealed(i, j) && !IsFlagged(i, j))
        {
            SetTile(i, j, E_UNCOVER, "Random Press");
            break;
        }

        x++;
        if (x > rows * cols)
            break;
    }

    rpi = i;
    rpj = j;
}

bool Game::AllClear()
{
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
            if (IsUnrevealed(i, j) && !IsFlagged(i, j))
                return false;
    return true;
}

int Game::CountAllFlagged()
{
    int c = 0;
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
            if (IsUnrevealed(i, j) && IsFlagged(i, j))
                c++;
    return c;
}

void Game::print()
{
    move(0, 0);
    printw(" %d / %d, %d", CountAllFlagged(), mines, clicks);
    clrtoeol();

    attron(COLOR_PAIR(C_MENU));
    move(0, 50);
    printw("NEW");
    move(0, 55);
    printw("QUIT");
    attroff(COLOR_PAIR(C_MENU));

    move(PRINT_OFFSET_Y, PRINT_OFFSET_X);

    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            if (j % cols == 0)
                move(PRINT_OFFSET_Y + i, PRINT_OFFSET_X);

            int cp;
            int ch;

            if (grid[index(i, j)].is_flagged)
            {
                cp = C_FLAG;
                ch = 'F';
            }
            else if (!grid[index(i, j)].is_revealed)
            {
                cp = C_UNRVLD;
                ch = '#';
            }
            else if (grid[index(i, j)].is_ender)
            {
                cp = C_ENDER;
                ch = 'X';
            }
            else if (grid[index(i, j)].is_mine)
            {
                cp = C_MINE;
                ch = '*';
            }
            else
            {
                int d = a(i, j); //grid[index(i, j)].adjacent_mine_count;
                cp = C_NUM0 + d;
                ch = '0' + d;
            }

            attron(COLOR_PAIR(cp));
            printw("%c ", ch);
            attroff(COLOR_PAIR(cp));
        }
    }
    refresh();
}

bool Game::isValid(int x, int y, int &i, int &j)
{
    if (x < PRINT_OFFSET_X)
        return false;

    j = (x - PRINT_OFFSET_X) / 2;
    i = y - PRINT_OFFSET_Y;

    if ((i >= 0) && (j >= 0) && (i < rows) && (j < cols))
        return true;

    return false;
}

bool Game::run()
{
    bool quit = false;
    bool playing = true;
    bool ender_found = false;
    int ch;
    MEVENT event;

    clicks = 0;

    do
    {
        print();

        ch = getch();
        if (ch == KEY_MOUSE)
        {
            if (getmouse(&event) == OK)
            {
                if (event.y == 0 && event.x >= 50 && event.x <= 60)
                {
                    playing = false;
                    if (event.x >= 55)
                        quit = true;
                    else
                        quit = false;
                    break;
                }

                int i, j;
                if (!ender_found)
                {
                    if (isValid(event.x, event.y, i, j))
                    {
                        if (IsUnrevealed(i, j) && !IsFlagged(i, j))
                        {
                            SetTile(i, j, E_UNCOVER, "Select");
                            clicks++;

                            if (IsMine(i, j))
                            {
                                SetTile(i, j, E_ENDER, "Pressed mine");
                                ender_found = true;
                            }

                            print();
                        }
                    }
                    else
                    {
                        playing = false;
                        quit = false;
                    }
                }
            }
        }
        else
        {
            switch (ch)
            {
            case 'q':
            case 'Q':
                playing = false;
                quit = true;
                break;
            case 'n':
            case 'N':
                playing = false;
                quit = false;
                break;
            }
        }

        if (!ender_found)
        {
            int ct;
            do
            {
                ct = changes;
                MatchBlank();
                MatchUnrevealed();
                MatchFlagged();
                MatchPatterns1();
                MatchPatterns2();
            } while (ct != changes);
        }

    } while (playing);

    return quit;
}
