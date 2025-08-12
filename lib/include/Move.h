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

// SORTING MOVES HEURISTICS
// -------------------------------------------------------
// 1. Captures (they go first as you might capture the king by accident and win)
// 2. Promotions 
// 3. Checks (they lead to tricky tactics)
// 4. All the other moves ...
// -------------------------------------------------------
// 1. Sorting captures with
//    MVV - LVA: Most Valuable Victim - Least Valuable Attacker
//    if captured piece is a king, assign a checkmate bonus
//    else
//      score  =  bonus for capture  +  |value of victim|  -  |value of attacker|
//    e.g. 
//      pawn takes queen:    score = bonus for capture + 800
//      pawn takes rook:     score = bonus for capture + 400
//      pawn takes pawn:     score = bonus for capture
//      rook takes pawn:     score = bonus for capture - 400
//      queen takes pawn:    score = bonus for capture - 800
//      king takes pawn:     score = bonus for capture - 9900 --> it is not important that the score is uniform or representative of how a move is good relative to anothre, it is just a sorting tool!
//
//  2. Promotion
//      score based on the promoted piece
//      promote to queen:   score = bonus for promotion  +  |piece_value|
//      if promoted piece is a queen, this should be considered first, immediately after eating the king, so it deserves a queen bonus  
//
//  3. Sorting checks
//      prefer checks with most powerful pieces:
//      score  =  bonus for checks  +  |value of attacker|
//      e.g. 
//      check with queen:    score = bonus for checks + 900
//
//  4. Sorting normal moves 
//      prefer moves of most important pieces: these moves may save a piece from an attack, or activate it to an important square etc.
//      in the future we could consider killer moves ...
//      score  =  |value of piece|
//
// Note: a move can be both a check and a capture: this move should be considered first
//
//      bonus for checks = 1000 --> checks bandwidth [1000 ; 1900]
//      bonus for promotion = 2000 
//      bonus for promotion to queen = 18000 --> promo bandwidth [2200 ; 20900]
//      bonus for capture = 20000 --> captures bandwidth [10100 (eat pawn with king) ; 29900 (eat king with pawn)]
inline int ScoreMove(Move& move){
    int score;
    /*uint8_t piece = MovePiece(move);
    uint8_t captured_piece = MoveCaptured(move);
    uint8_t promoted_piece = MovePromotion(move);
    uint8_t flags = MoveFlags(move);
    bool is_check = MoveIsCheck(move);
    // capture 
    if(captured_piece != 15){
        score += BONUS_FOR_CAPTURE + abs(PIECES_VALUES[captured_piece]) - abs(PIECES_VALUES[piece]);
    }
    // promotion
    if(promoted_piece != 15){
        score += BONUS_FOR_PROMOTION + abs(PIECES_VALUES[promoted_piece]);
    }
    // check
    if(is_check){
        score += BONUS_FOR_CHECKS + abs(PIECES_VALUES[piece]);
    }*/

    int flags = move >> 12;
    score = flags;

    return score;
}
