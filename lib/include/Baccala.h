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
const std::string starting_position_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w Kk - 0 1";
const int knight_deltas[8][2] = {
    {1, 2}, {2, 1}, {2, -1}, {1, -2}, {-1, -2}, {-2, -1}, {-2, 1}, {-1, 2}
};
const int king_deltas[8][2] = {
    {1, 1}, {0, 1}, {-1, 1}, {1, 0}, {-1, 0}, {1, -1}, {0, -1}, {-1, -1}
};

// useful functions
void PrintMask(Mask mask);
void PrintMoves(std::vector<Square> moves);
int PieceValue(char piece);

// Class for chess move
class Move
{
private:
public:
    Square current_square;
    Square target_square;
    char piece;
    char side_to_move;
    bool is_special_move;
    bool is_capture;
    bool is_check;
    //bool is_check_mate;

    // constructors
    Move(Square current_square, Square target_square, char piece, bool is_capture);
    //Move(std::string special_move);

    // methods
    void PrintMove();
    std::string AlgebraicNotation();

    ~Move();
};


// Class for chess position
class Position
{
private:
public:
    std::string fen;
    bool white_to_move; // 1 = white; 0 = black
    Board board = empty_board;
    char castling_rights;
    std::string en_passant_target_square;
    bool can_white_castle_kingside = false;
    bool can_white_castle_queenside = false;
    bool can_black_castle_kingside = false;
    bool can_black_castle_queenside = false;
    Mask white_pieces_mask = empty_mask;
    Mask black_pieces_mask = empty_mask;
    Mask all_pieces_mask = empty_mask;
    Mask white_covered_squares_mask = empty_mask;
    Mask black_covered_squares_mask = empty_mask;
    Square white_king_square;
    Square black_king_square;
    bool is_check;
    int material_value = 0;

    // constructors
    Position(std::string fen);

    // methods
    void PrintBoard(void);
    bool IsLegal(Move move);
    std::vector<Move> LegalMoves();

    // destructor
    ~Position();
};

// declare functions for pieces movement
Mask KnightMovesMask(Square current_square, Mask your_pieces_mask);
Mask KnightCoveredSquaresMask(Square current_square);
std::vector<Square> KnightTargetSquares(Square current_square, Mask your_pieces_mask);

Mask RookMovesMask(Square current_square, Mask your_pieces_mask, Mask opponent_pieces_mask);
Mask RookCoveredSquaresMask(Square current_square, Mask all_pieces_mask);
std::vector<Square> RookTargetSquares(Square current_square, Mask your_pieces_mask, Mask opponent_pieces_mask);

Mask BishopMovesMask(Square current_square, Mask your_pieces_mask, Mask opponent_pieces_mask);
Mask BishopCoveredSquaresMask(Square current_square, Mask all_pieces_mask);
std::vector<Square> BishopTargetSquares(Square current_square, Mask your_pieces_mask, Mask opponent_pieces_mask);

Mask QueenMovesMask(Square current_square, Mask your_pieces_mask, Mask opponent_pieces_mask);
Mask QueenCoveredSquaresMask(Square current_square, Mask all_pieces_mask);
std::vector<Square> QueenTargetSquares(Square current_square, Mask your_pieces_mask, Mask opponent_pieces_mask);

Mask KingMovesMask(Square current_square, Mask your_pieces_mask);
Mask KingCoveredSquaresMask(Square current_square);
std::vector<Square> KingTargetSquares(Square current_square, Mask your_pieces_mask);

Mask WhitePawnMovesMask(Square current_square, Mask your_pieces_mask, Mask opponent_pieces_mask);
Mask WhitePawnCoveredSquaresMask(Square current_square);
std::vector<Square> WhitePawnTargetSquares(Square current_square, Mask your_pieces_mask, Mask opponent_pieces_mask);

Mask BlackPawnMovesMask(Square current_square, Mask your_pieces_mask, Mask opponent_pieces_mask);
Mask BlackPawnCoveredSquaresMask(Square current_square);
std::vector<Square> BlackPawnTargetSquares(Square current_square, Mask your_pieces_mask, Mask opponent_pieces_mask);

Mask WhitePiecesMask(Board board);
Mask BlackPiecesMask(Board board);
Mask AllPiecesMask(Board board);
Mask WhiteCoveredSquaresMask(Board board);
Mask BlackCoveredSquaresMask(Board board);