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
    uint64_t en_passant_target_square = 0ULL;
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

// structure that packs move and position
struct MoveAndPosition
{
    Move move;
    Position pos;
};

// Generate all the possible moves following the rules, 
// but without checking if the king is left in danger by that move.
// This is done later! So these moves are technically NOT the legal moves 
std::vector<Move> AllMoves(const Position& pos);

// Consider all the moves, filter out illegal moves that leave the king in check and generate the new position
// This function is optimized for the engine purposes:
// if we check the legality of a move first and then use it to update the position, we are forced to generate 
// the new bitboards of covered squares twice and check the king's position twice! With this approach we do it only ONCE
// We can later write another function for user purposes that only filters out illegal moves
//std::vector<Position> MakeLegalMoves(const Position& pos, const std::vector<Move>& move);