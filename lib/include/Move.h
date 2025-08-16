#pragma once
#include <cstdint>
#include <Utilities.h>

// [4 bits] [6 bits] [6 bits]
//  flags      to      from
//      flags = 0 -> quiet move
//      flags = 1 -> double pawn push
//      flags = 2 -> kingside castling
//      flags = 3 -> queenside castling
//      flags = 4 -> standard capture
//      flags = 5 -> en-passant capture 
//      flags = 6 -> nothing 
//      flags = 7 -> nothing
//      flags = 8 -> promotion to knight
//      flags = 9 -> promotion to bishop
//      flags = 10 -> promotion to rook
//      flags = 11 -> promotion to queen
//      flags = 12 -> capture and promotion to knight
//      flags = 13 -> capture and promotion to bishop
//      flags = 14 -> capture and promotion to rook
//      flags = 15 -> capture and promotion to queen
typedef uint16_t Move;

struct ScoredMove{
    Move move;
    int score;
};

struct StateMemory{
    uint64_t en_passant_target_square;
    uint8_t moved_piece_index = 0;
    uint8_t captured_piece_index = 0;
    uint8_t promoted_piece_index = 0;
    uint8_t half_move_counter = 0;
    bool can_white_castle_kingside = false;
    bool can_white_castle_queenside = false;
    bool can_black_castle_kingside = false;
    bool can_black_castle_queenside = false;
};

// theoretical maximum number of moves in a given position (this is an overestimate, however we consider eating the king as a move, so this might be reasonable)
// for 99.99% of positions this is definitely fine, maybe edge cases could exceed this value
const int MAX_NUMBER_OF_MOVES = 256;

inline Move EncodeMove(unsigned long from, unsigned long to, uint16_t flags){
    Move move = 0;
    // progress of move bits:       // 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
    move = (uint16_t)from;          // 0 0 0 0 0 0 0 0 0 0 s s s s s s 
    move |= ((uint16_t)to << 6);    // 0 0 0 0 t t t t t t s s s s s s
    move |= flags << 12;            // f f f f t t t t t t s s s s s s
    return move;
}


inline void PrintMove(const Move& move){
    uint8_t from, to, flags;
    std::string move_str;
    from = move & 0b00111111;
    to = (move >> 6) & 0b00111111;
    flags = move >> 12;
    move_str = SquareToAlphabet(from);
    move_str += SquareToAlphabet(to);
    if(flags == 15 || flags == 11){ move_str += "Q"; }
    else if(flags == 14 || flags == 10){ move_str += "R"; }
    else if(flags == 13 || flags == 9){ move_str += "B"; }
    else if(flags == 12 || flags == 8){ move_str += "N"; }
    std::cout << move_str << "\n";
}

