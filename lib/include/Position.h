#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <Move.h>

struct Position
{
    uint64_t pieces[12] = {
        0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL
    }; // K Q R B N P k q r b n p
    bool white_to_move;
    bool can_white_castle_kingside = false;
    bool can_white_castle_queenside = false;
    bool can_black_castle_kingside = false;
    bool can_black_castle_queenside = false;
    unsigned int half_move_counter = 0;
    unsigned int move_counter = 0;
    unsigned int en_passant_target_file; 
    int white_material_value = 0;
    int black_material_value = 0;
    uint64_t white_pieces = 0ULL;
    uint64_t black_pieces = 0ULL;
    uint64_t all_pieces = 0ULL;
    uint64_t white_covered_squares = 0ULL;
    uint64_t black_covered_squares = 0ULL;
};

Position PositionFromFen(std::string fen);

void PrintBoard(Position pos);

int Score(Position pos);

std::vector<Move> AllMoves(const Position& pos);