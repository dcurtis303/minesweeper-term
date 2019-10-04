#include <cstdlib>

class Tile
{
public:
    bool is_mine;
    bool is_flagged;
    bool is_revealed;
    bool is_ender;
    int adjacent_mine_count;
};

#define E_MINE -1
#define E_FLAG -2
#define E_UNCOVER -3
#define E_ENDER -4

enum colorpairs { C_FLAG=1, C_UNRVLD, C_NUM1, C_NUM2 };

class Game
{
    Tile *grid;
    int rows, irow;
    int cols, icol;
    int mines;
    int changes;
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

    int index(int x, int y) { return x * cols + y; };
    int rrng(int r) { return (float)rand() / RAND_MAX * r; }

    bool IsMine(int i, int j) { return grid[index(i, j)].is_mine; }
    bool IsNumber(int i, int j) { return !IsMine(i, j); }
    bool IsZero(int i, int j) { return grid[index(i, j)].adjacent_mine_count == 0; }
    bool IsRevealed(int i, int j) { return grid[index(i, j)].is_revealed; }
    bool IsUnrevealed(int i, int j) { return !IsRevealed(i, j); }
    bool IsFlagged(int i, int j) { return grid[index(i, j)].is_flagged; }

    const int &a(int i, int j) { return grid[index(i,j)].adjacent_mine_count; }

public:
    Game(int, int, int, int);
    ~Game();

    void init_colors();

    void run();
    void run_old();
    void print();
};
