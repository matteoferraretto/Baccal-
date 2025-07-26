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

// printing the move
inline void PrintMove(const Move& m){
    uint8_t from = MoveFrom(m);
    uint8_t to = MoveTo(m);
    uint8_t piece = MovePiece(m);
    uint8_t captured = MoveCaptured(m);
    uint8_t promotion = MovePromotion(m);
    uint8_t flags = MoveFlags(m);
    std::string move_str; 
    move_str = PieceToAlphabet(piece);
    move_str += SquareToAlphabet(from);
    if(captured != 15){ move_str += "x"; } // manage capture
    move_str += SquareToAlphabet(to);
    std::cout << move_str << "\n";
}