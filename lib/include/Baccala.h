#pragma once;
#include <SDL.h>
#include <iostream>
#include <string>
#include <vector>

// useful types and operations
typedef std::vector<std::vector<bool>> Mask;
typedef std::vector<std::vector<char>> Board;
struct Square { int i; int j; };
inline Mask operator+(const Mask& m1, const Mask& m2){
    Mask m = m1;
    for(int i=0; i<8; i++){
        for(int j=0; j<8; j++){
            m[i][j] = m1[i][j] || m2[i][j];
        }
    }
    return m;
};
inline Mask& operator+=(Mask& m1, const Mask& m2){
    m1 = m1 + m2;
    return m1;
}

// useful constants
const Mask empty_mask = {
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0}
};
const Board empty_board = {
    {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
    {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
    {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
    {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
    {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
    {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
    {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
    {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}
};
const std::string starting_position_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
const int knight_deltas[8][2] = {
    {1, 2}, {2, 1}, {2, -1}, {1, -2}, {-1, -2}, {-2, -1}, {-2, 1}, {-1, 2}
};
const int king_deltas[8][2] = {
    {1, 1}, {0, 1}, {-1, 1}, {1, 0}, {-1, 0}, {1, -1}, {0, -1}, {-1, -1}
};
const char pieces_white_pawn_becomes[4] = {'Q', 'R', 'B', 'N'};
const char pieces_black_pawn_becomes[4] = {'q', 'r', 'b', 'n'};
const int negative_infinity = std::numeric_limits<int>::min();
const int positive_infinity = std::numeric_limits<int>::max();

// useful functions
void PrintMask(Mask mask);
void PrintMoves(std::vector<Square> moves);
int PieceValue(char piece);
std::string SquareToAlphabet(Square square);
Square AlphabetToSquare(std::basic_string<char> square_string);

// Class for chess move
class Move
{
private:
public:
    Square current_square;
    Square target_square;
    char piece;
    bool is_capture = false;
    bool is_check = false;
    bool is_castling = false;
    char new_piece; // used for pawn promotion
    char castling_side; // 'K': white castles kingside, 'Q' white castles queenside, 'k' and 'q' are analogues for black
    int rank = 0; // move rank for smart move sorting
    //bool is_check_mate;

    // constructor for a normal move
    Move(Square current_square, Square target_square, char piece, bool is_capture);
    // constructor for pawn promotion
    Move(Square target_square, char new_piece);
    // constructor for castling
    Move(char castling_side);
    //Move(std::string special_move);

    // methods
    void PrintMove();
    std::string AlgebraicNotation();

    ~Move();
};


// declare functions for pieces movement
Mask KnightMovesMask(Square current_square, Mask& your_pieces_mask);
Mask KnightCoveredSquaresMask(Square current_square);
std::vector<Square> KnightTargetSquares(Square current_square, Mask& your_pieces_mask);

Mask RookMovesMask(Square current_square, Mask& your_pieces_mask, Mask& opponent_pieces_mask);
Mask RookCoveredSquaresMask(Square current_square, Mask& all_pieces_mask);
std::vector<Square> RookTargetSquares(Square current_square, Mask& your_pieces_mask, Mask& opponent_pieces_mask);

Mask BishopMovesMask(Square current_square, Mask& your_pieces_mask, Mask& opponent_pieces_mask);
Mask BishopCoveredSquaresMask(Square current_square, Mask& all_pieces_mask);
std::vector<Square> BishopTargetSquares(Square current_square, Mask& your_pieces_mask, Mask& opponent_pieces_mask);

Mask QueenMovesMask(Square current_square, Mask& your_pieces_mask, Mask& opponent_pieces_mask);
Mask QueenCoveredSquaresMask(Square current_square, Mask& all_pieces_mask);
std::vector<Square> QueenTargetSquares(Square current_square, Mask& your_pieces_mask, Mask& opponent_pieces_mask);

Mask KingMovesMask(Square current_square, Mask& your_pieces_mask);
Mask KingCoveredSquaresMask(Square current_square);
std::vector<Square> KingTargetSquares(Square current_square, Mask& your_pieces_mask);

Mask WhitePawnMovesMask(Square current_square, Mask& your_pieces_mask, Mask& opponent_pieces_mask, Square en_passant_target_square);
Mask WhitePawnCoveredSquaresMask(Square current_square);
std::vector<Square> WhitePawnTargetSquares(Square current_square, Mask& your_pieces_mask, Mask& opponent_pieces_mask, Square en_passant_target_square);

Mask BlackPawnMovesMask(Square current_square, Mask& your_pieces_mask, Mask& opponent_pieces_mask);
Mask BlackPawnCoveredSquaresMask(Square current_square);
std::vector<Square> BlackPawnTargetSquares(Square current_square, Mask& your_pieces_mask, Mask& opponent_pieces_mask);

Mask WhitePiecesMask(Board board);
Mask BlackPiecesMask(Board board);
Mask AllPiecesMask(Board board);
Mask WhiteCoveredSquaresMask(Board board);
Mask BlackCoveredSquaresMask(Board board);

void SortMoves(std::vector<Move>& moves);