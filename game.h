#include <cstdlib>

#ifndef GAME_H_
#define GAME_H_

class Tile
{
public:
    bool is_mine;
    bool is_flagged;
    bool is_revealed;
    bool is_ender;
    int adjacent_mine_count;
};

enum operation
{
    E_MINE = -4,
    E_FLAG,
    E_UNCOVER,
    E_ENDER
};

enum ColorPairs
{
    C_FLAG = 1,
    C_UNRVLD,
    C_ENDER,
    C_MINE,
    C_NUM0,
    C_NUM1,
    C_NUM2,
    C_NUM3,
    C_NUM4,
    C_NUM5,
    C_NUM6,
    C_NUM7,
    C_NUM8
};

#define CBKGND 246
#define CBKGND2 242

struct CPDef
{
    ColorPairs cp;
    int fg;
    int bg;
}; 

#define PRINT_OFFSET_X 4 // must be even number
#define PRINT_OFFSET_Y 2

class Game
{
    Tile *grid;
    int rows, irow;
    int cols, icol;
    int mines;
    int seed;
    int changes;
    int clicks;
    int lau1[8][2];
    int lau2[8][2];

    int CountAdjacentMines(int, int);
    void MatchBlank();
    void MatchUnrevealed();
    void MatchFlagged();
    void MatchPatterns1();
    void MatchPatterns2();
    int ListAdjacentUnrevealed(int, int, int);
    bool InLAUList1(int, int, int);
    int CompareLAULists(int, int);
    void RevealLAUList2(int, int);
    int IsMissingFlags(int, int);
    void RevealAdjacentTiles(int, int);
    int CountAdjacentUnrevealed(int, int);
    int CountAdjacentFlagged(int, int);
    void FlagAdjacentMines(int, int);
    void SetTile(int, int, char, const char *);
    void RandomPress(int &, int &);
    bool AllClear();
    int CountAllFlagged();

    int index(int x, int y) { return x * cols + y; };
    int rrng(int r) { return (float)rand() / RAND_MAX * r; }

    bool IsMine(int i, int j) { return grid[index(i, j)].is_mine; }
    bool IsNumber(int i, int j) { return !IsMine(i, j); }
    bool IsZero(int i, int j) { return grid[index(i, j)].adjacent_mine_count == 0; }
    bool IsRevealed(int i, int j) { return grid[index(i, j)].is_revealed; }
    bool IsUnrevealed(int i, int j) { return !IsRevealed(i, j); }
    bool IsFlagged(int i, int j) { return grid[index(i, j)].is_flagged; }

    int &a(int i, int j) { return grid[index(i, j)].adjacent_mine_count; }

    bool isValid(int, int, int &, int &);
    void print();

public:
    Game(int, int, int, int);
    ~Game();

    void init_colors();
    bool run();

};

#endif // GAME_H_