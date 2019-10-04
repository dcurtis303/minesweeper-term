#include <iostream>
#include <ncurses.h>

#include "game.h"

using namespace std;

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

    srand(s);
    printf("Seed: %d\n", s);

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

    int mcnt = 0;
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
            if (IsMine(i, j))
                mcnt += 1;
    printf("Mine Count: %d\n", mcnt);

    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
            if (!IsMine(i, j))
                grid[index(i, j)].adjacent_mine_count = CountAdjacentMines(i, j);
}

Game::~Game()
{
    delete grid;
}

void Game::init_colors()
{

    init_pair(C_NUM1, 21, 248);
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
    const char *s;

    switch (value)
    {
    case E_UNCOVER:
        grid[index(i, j)].is_revealed = true;
        s = "Uncover";
        break;
    case E_FLAG:
        grid[index(i, j)].is_flagged = true;
        s = "Flag";
        break;
    case E_ENDER:
        grid[index(i, j)].is_ender = true;
        s = "End Mine";
        break;
    }

    changes++;
    printf("(%d, %d) = %s <- %s\n", i, j, s, who);
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

void Game::print()
{
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            if (j % cols == 0)
                printw("\n");

            int cp;
            int ch;

            if (grid[index(i, j)].is_flagged)
            {
                cp = C_FLAG;
                ch = 'F';

            }
            else if (!grid[index(i, j)].is_revealed)
            {

            }
            else if (grid[index(i, j)].is_ender)
            {
            }
            else if (grid[index(i, j)].is_mine)
            {
            }
            else
            {
                int d = grid[index(i, j)].adjacent_mine_count;
                switch (d)
                {
                    case 1:
                        cp = C_NUM1;
                        ch = '1';
                }
            }


            attron(COLOR_PAIR(cp));
            printw("%c ", ch);
            attroff(COLOR_PAIR(cp));
        }
    }
}


void Game::run()
{
    print();
    getch();
}

void Game::run_old()
{
    bool playing = true;
    int presses = 0;

    cout << "Start..." << endl;

    while (playing)
    {
        int rpi, rpj;
        RandomPress(rpi, rpj);
        presses++;

        if (IsMine(rpi, rpj))
        {
            SetTile(rpi, rpj, E_ENDER, "Pressed mine");
            playing = false;
            break;
        }

        for (int i = 1; playing; i++)
        {
            int c = changes;
            printf("Press: %d, Iteration: %d, total changes: %d\n", presses, i, c);

            MatchBlank();
            MatchUnrevealed();
            MatchFlagged();
            MatchPatterns1();
            MatchPatterns2();

            if (c == changes)
                break;

            if (AllClear())
            {
                printf("Game Won! Total changes: %d\n", changes);
                playing = false;
            }
        }
    }
    cout << "Finish..." << endl;
}
