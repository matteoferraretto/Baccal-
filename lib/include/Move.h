#pragma once
#include <cstdint>
#include <Utilities.h>

// we pack the structure Move in a single 32 bit number with the following convention
// Bit index:  31             ...              0
//           [flags][promo][capt][piece][to][from]
// Bits:        6      4     4      4    6    6   = 30 total --> the last 2 bits are garbage.
//          - from = 0, ... , 63 -> exactly 6 bits required
//          - to   = 0, ... , 63 -> exactly 6 bits required
//          - piece = 0, ... , 11, 15 = K, Q, R, B, N, P, k, q, r, b, n, p, nothing -> 4 bits (numbers from 12 to 14 are garbage)
//          - capture = 0, ..., 11, 15 = K, Q, R, B, N, P, k, q, r, b, n, p, nothing -> 4 bits (numbers from 12 to 14 are garbage)
//          - promoted piece = 1, 2, 3, 4, 6, 7, 8, 9, 15 = Q, R, B, N, q, r, b, n, nothing -> 4 bits (0, 5, 10, 11, 12, 13, 14 are garbage)
//          - flags =   0 -> quiet
//                      1 -> pawn move
//                      2 -> double pawn-push
//                      4 -> castling
//                      8 -> capture
//                      16 -> is check
//                      ... <-- we have 2 more bits to use if needed
// NB: the flags are powers of two because they are not mutually exclusive! A move can be at the same time an en-passang move and a capture, so both flags are active
// we can change this convention later ...
typedef uint32_t Move;

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
typedef uint16_t MoveNew;

struct StateMemory{
    //uint64_t moved_piece = 0ULL;
    uint64_t captured_piece = 0ULL;
    uint64_t friendly_pieces = 0ULL;
    uint64_t enemy_pieces = 0ULL;
    uint64_t en_passant_target_square;
    uint8_t moved_piece_index = 0;
    uint8_t captured_piece_index = 0;
    uint8_t half_move_counter = 0;
    bool can_white_castle_kingside = false;
    bool can_white_castle_queenside = false;
    bool can_black_castle_kingside = false;
    bool can_black_castle_queenside = false;
};

// define a null move: flags = 0; promo = 15; capt = 15; piece = 15; to = 0; from = 0;
const Move NULL_MOVE = 16773120;

// theoretical maximum number of moves in a given position (this is an overestimate, however we consider eating the king as a move, so this might be reasonable)
// for 99.99% of positions this is definitely fine, maybe edge cases could exceed this value
const int MAX_NUMBER_OF_MOVES = 256;

inline MoveNew EncodeMoveNew(unsigned long from, unsigned long to, uint16_t flags){
    uint16_t move = 0;
    // progress of move bits:       // 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
    move = (uint16_t)from;          // 0 0 0 0 0 0 0 0 0 0 s s s s s s 
    move |= ((uint16_t)to << 6);    // 0 0 0 0 t t t t t t s s s s s s
    move |= flags << 12;            // f f f f t t t t t t s s s s s s
    return move;
}

// functions that help to encode a move from all the info
inline Move EncodeMove(uint8_t from, uint8_t to, uint8_t piece, uint8_t captured = 15, uint8_t promotion = 15, uint8_t flags = 0) 
{
    return  (from & 0x3F) | // 0x3F in binary is 000...000111111 <--- last 6 bits are 1
           ((to & 0x3F) << 6) |
           ((piece & 0x0F) << 12) | // 0x0F in binary is 000...0001111 <--- last 4 bits are 1
           ((captured & 0x0F) << 16) |
           ((promotion & 0x0F) << 20) |
           ((flags & 0x3F) << 24);
}

// decoding functions
inline uint8_t MoveFrom(Move m)       { return  m        & 0x3F; }
inline uint8_t MoveTo(Move m)         { return (m >> 6)  & 0x3F; }
inline uint8_t MovePiece(Move m)      { return (m >> 12) & 0x0F; }
inline uint8_t MoveCaptured(Move m)   { return (m >> 16) & 0x0F; }
inline uint8_t MovePromotion(Move m)  { return (m >> 20) & 0x0F; }
inline uint8_t MoveFlags(Move m)      { return (m >> 24) & 0x3F; }
inline bool MoveIsCheck(Move m)       { return (m >> 28) & 1ULL; }

// printing the move
inline void PrintMove(const Move& m){
    uint8_t from = MoveFrom(m);
    uint8_t to = MoveTo(m);
    uint8_t piece = MovePiece(m);
    uint8_t captured = MoveCaptured(m);
    uint8_t promotion = MovePromotion(m);
    uint8_t flags = MoveFlags(m);
    bool is_check = MoveIsCheck(m);
    std::string move_str; 
    move_str = PieceToAlphabet(piece);
    move_str += SquareToAlphabet(from);
    if(captured != 15){ move_str += "x"; } // manage capture
    move_str += SquareToAlphabet(to);
    if(promotion != 15){ move_str += PieceToAlphabet(promotion); }
    if(is_check){ move_str += "+"; }
    std::cout << move_str << "\n";
}

inline void PrintMoveNew(const MoveNew& move){
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
    int score = 0;
    uint8_t piece = MovePiece(move);
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
    }
    return score;
}
