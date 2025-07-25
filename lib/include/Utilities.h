#pragma once
#include <iostream>
#include <string>
#include <cstdint>

// random generator of 64-bit unsigned integers
uint64_t rand64();

// integer power
int IntPow(int x, unsigned int p);

// relevant positions
const std::string starting_position_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 0";

// pieces values
const int WHITE_ROOK_VALUE = 500;
const int BLACK_ROOK_VALUE = -500;
const int WHITE_BISHOP_VALUE = 300;
const int BLACK_BISHOP_VALUE = -300;
const int WHITE_KNIGHT_VALUE = 300;
const int BLACK_KNIGHT_VALUE = -300;
const int WHITE_QUEEN_VALUE = 900;
const int BLACK_QUEEN_VALUE = -900;
const int WHITE_KING_VALUE = 100000;
const int BLACK_KING_VALUE = -100000;
const int WHITE_PAWN_VALUE = +100;
const int BLACK_PAWN_VALUE = -100;

// for each square we store the number of squares where one can put a blocker for bishop
const int n_squares_for_bishop_blockers[64] = {
    6, 5, 5, 5, 5, 5, 5, 6,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    6, 5, 5, 5, 5, 5, 5, 6
};

// bit-wise operations for bitboards
void bit_set(uint64_t& bitboard, int i, int j);
void bit_clear(uint64_t& bitboard, int i, int j);
bool bit_get(uint64_t bitboard, int i, int j);
bool bit_get(const uint64_t& bitboard, const unsigned long& square);
void clear_last_active_bit(uint64_t& bitboard); // set to 0 the last bit which is 1
void PrintBitboard(uint64_t bitboard);

// pawn promotions
const char pieces_white_pawn_becomes[4] = {'Q', 'R', 'B', 'N'};
const char pieces_black_pawn_becomes[4] = {'q', 'r', 'b', 'n'};
// pieces deltas
const int knight_deltas[8][2] = {
    {1, 2}, {2, 1}, {2, -1}, {1, -2}, {-1, -2}, {-2, -1}, {-2, 1}, {-1, 2}
};
const int king_deltas[8][2] = {
    {1, 1}, {0, 1}, {-1, 1}, {1, 0}, {-1, 0}, {1, -1}, {0, -1}, {-1, -1}
};
const int white_pawn_deltas[2][2] = {
    {-1, 1}, {-1, -1}
};
const int black_pawn_deltas[2][2] = {
    {1, 1}, {1, -1}
};

// Transform a square index to alphabetic notation
std::string SquareToAlphabet(uint8_t& square);
std::string PieceToAlphabet(uint8_t& piece);

// relevant constants
const int negative_infinity = std::numeric_limits<int>::min();
const int positive_infinity = std::numeric_limits<int>::max();

// Pieces Square Tables
const int knightPST[64] = {
    -50, -40, -30, -30, -30, -30, -40, -50,
    -40, -20,   0,   5,   5,   0, -20, -40,
    -30,   5,  10,  15,  15,  10,   5, -30,
    -30,   0,  15,  20,  20,  15,   0, -30,
    -30,   5,  15,  20,  20,  15,   5, -30,
    -30,   0,  10,  15,  15,  10,   0, -30,
    -40, -20,   0,   0,   0,   0, -20, -40,
    -50, -40, -30, -30, -30, -30, -40, -50
};
const int pawnPST[64] = {
      0,   0,   0,   0,   0,   0,   0,   0,
     50,  50,  50,  50,  50,  50,  50,  50,
     10,  10,  20,  30,  30,  20,  10,  10,
      5,   5,  10,  25,  25,  10,   5,   5,
      0,   0,   0,  20,  20,   0,   0,   0,
      5,  -5, -10,   0,   0, -10,  -5,   5,
      5,  10,  10, -20, -20,  10,  10,   5,
      0,   0,   0,   0,   0,   0,   0,   0
};
const int bishopPST[64] = {
    -20, -10, -10, -10, -10, -10, -10, -20,
    -10,   5,   0,   0,   0,   0,   5, -10,
    -10,  10,  10,  10,  10,  10,  10, -10,
    -10,   5,   5,  10,  10,   5,   0, -10,
    -10,   5,   5,  10,  10,   5,   5, -10,
    -10,  10,  10,  10,  10,  10,  10, -10,
    -10,   5,   0,   0,   0,   0,   5, -10,
    -20, -10, -10, -10, -10, -10, -10, -20
};
const int rookPST[64] = {
     0,   0,   0,   5,   5,   0,   0,  0,
    10,   20,  20,  20,  20,  20,  20, 10,
    -5,   0,   0,   0,   0,   0,   0,  -5,
    -5,   0,   0,   0,   0,   0,   0,  -5,
    -5,   0,   0,   0,   0,   0,   0,  -5,
    -5,   0,   0,   0,   0,   0,   0,  -5,
    -5,   0,   0,   0,   0,   0,   0,  -5,
     0,   0,   0,   0,   0,   0,   0,   0
};
const int queenPST[64] = {
    -20, -10, -10,  -5,  -5, -10, -10, -20,
    -10,   0,   0,   0,   0,   0,   0, -10,
    -10,   0,   5,   5,   5,   5,   0, -10,
     -5,   0,   5,   5,   5,   5,   0,  -5,
      0,   0,   5,   5,   5,   5,   0,  -5,
    -10,   0,   5,   5,   5,   5,   0, -10,
    -10,   0,   0,   0,   0,   0,   0, -10,
    -20, -10, -10,  -5,  -5, -10, -10, -20
};
const int kingPST_Middlegame[64] = {
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -20, -30, -30, -40, -40, -30, -30, -20,
    -10, -20, -20, -20, -20, -20, -20, -10,
     20,  20,   0,   0,   0,   0,  20,  20,
     20,  30,  10,   0,   0,  10,  30,  20
};
const int kingPST_Endgame[64] = {
    -50, -40, -30, -20, -20, -30, -40, -50,
    -30, -20, -10,   0,   0, -10, -20, -30,
    -30, -10,  20,  30,  30,  20, -10, -30,
    -30, -10,  30,  40,  40,  30, -10, -30,
    -30, -10,  30,  40,  40,  30, -10, -30,
    -30, -10,  20,  30,  30,  20, -10, -30,
    -30, -30,   0,   0,   0,   0, -30, -30,
    -50, -30, -30, -30, -30, -30, -30, -50
};