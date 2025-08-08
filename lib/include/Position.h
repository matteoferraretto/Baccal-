#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <Move.h>

// Class Position is made as follows:
// - 12 bitboards storing the position of the 12 types of pieces: K Q R B N P k q r b n p
// - 3 bitboards with mask of white pieces, black pieces and all pieces (meant to avoid repeated calculations in move generation)
// - 1 bitboard with mask of en passant target square
// - 1 uint8_t with relevant info:
//      bits      _            _                   _                  _                _                   _               _                _
//      info  [nothing] [black in check?] [white in check?] [can black O-O-O?] [can black O-O?] [can white O-O-O?] [can white O-O?] [white to move?]
// - 1 uint8_t half_move_counter to account for the 50 moves rule 
//      from 0 to 50 and we have 2 extra bits
// - 1 uint8_t n_legal_moves representing the number of legal moves
//      from 0 to 2^8 - 1 = 255 (max allowed size is 256)
// TOTAL MEMORY REQUIRED 
// 64 x 12 + 64 x 3 + 64 x 1 + 8 x 1 + 8 x 1 + 8 x 1 = 1048 bits = 131 bytes
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
    uint8_t half_move_counter = 0;
//    unsigned int move_counter = 0;
    uint64_t en_passant_target_square = 0ULL;
//    int white_material_value = 0;
//    int black_material_value = 0;
    uint64_t white_pieces = 0ULL;
    uint64_t black_pieces = 0ULL;
    uint64_t all_pieces = 0ULL;
    uint64_t white_covered_squares = 0ULL;
    uint64_t black_covered_squares = 0ULL;
    uint8_t n_legal_moves = 0;
};

Position PositionFromFen(std::string fen);

void PrintBoard(Position pos);

int PositionScore(Position& pos);

// structure that packs move and position
struct MoveAndPosition
{
    Move move;
    Position position;
    int score;
};

// Generate all the PSEUDOLEGAL moves, which means:
//  - move a piece from a square to another square following the rules
//  - if the square is occupied by a friendly piece, don't consider the move
//  - ignore if the move leaves the king in danger (whence PSEUDOlegal)
void PseudoLegalMoves(const Position& pos, MoveNew* moves);

void MakeMove(Position& pos, const MoveNew& move, StateMemory& state);

bool IsLegal(Position& pos, const Move& move);

void UnmakeMove(Position& pos, const MoveNew& move, const StateMemory& state);

// Generate all the possible moves following the rules, 
// but without checking if the king is left in danger by that move.
// This is done later! So these moves are technically NOT the legal moves 
void LegalMoves(Position& pos, MoveAndPosition* legal_moves);

// Consider all the moves, filter out illegal moves that leave the king in check and generate the new position
// This function is optimized for the engine purposes:
// if we check the legality of a move first and then use it to update the position, we are forced to generate 
// the new bitboards of covered squares twice and check the king's position twice! With this approach we do it only ONCE
// We can later write another function for user purposes that only filters out illegal moves
//std::vector<Position> MakeLegalMoves(const Position& pos, const std::vector<Move>& move);