#pragma once
#include <cstdint>
#include <Utilities.h>

const uint8_t N_BITS_ROOK = 12;
const uint8_t N_BITS_BISHOP = 9;
constexpr uint16_t N_ATTACKS_ROOK = 1 << N_BITS_ROOK;
constexpr uint16_t N_ATTACKS_BISHOP = 1 << N_BITS_BISHOP;
constexpr uint8_t SHIFT_ROOK = 64 - N_BITS_ROOK;
constexpr uint8_t SHIFT_BISHOP = 64 - N_BITS_BISHOP;


// magic numbers 
typedef uint64_t Magic;

struct MaskAndMagic{
    Bitboard mask = 0ULL;
    Magic magic = 0ULL;
};

// arrays that store all the possible movements of the non-sliding pieces
// this is used to avoid on-the-fly calculation during the min-max process
extern Bitboard knight_covered_squares_bitboards[64]; 
extern Bitboard king_covered_squares_bitboards[64];
extern Bitboard white_pawn_covered_squares_bitboards[64];
extern Bitboard black_pawn_covered_squares_bitboards[64]; // when pawns are in first or last raw the array contains garbage
extern Bitboard white_pawn_advance_squares_bitboards[64];
extern Bitboard black_pawn_advance_squares_bitboards[64];

// relevant masks of sliding pieces
extern Bitboard rook_masks[64];
extern Magic rook_magics[64];
extern MaskAndMagic rook_mm[64];
extern Bitboard rook_covered_squares_bb[64][N_ATTACKS_ROOK];

extern Bitboard bishop_masks[64];
extern Magic bishop_magics[64];
extern MaskAndMagic bishop_mm[64];
extern Bitboard bishop_covered_squares_bb[64][N_ATTACKS_BISHOP];

// functions that generate covered square bitboards for non-sliding pieces when the piece is in the square (i, j)
Bitboard KnightCoveredSquares(int i, int j);
Bitboard KingCoveredSquares(int i, int j);
Bitboard WhitePawnCoveredSquares(int i, int j);
Bitboard WhitePawnAdvanceSquares(int i, int j);
Bitboard BlackPawnCoveredSquares(int i, int j);
Bitboard BlackPawnAdvanceSquares(int i, int j);


// -------------- LOGIC FOR SLIDING PIECES ------------------------
// mask of the squares where blockers to the movement of a sliding piece positioned in (i,j) can be
// for example, the blockers to a rook positioned in a1 can be positioned in these squares:
// . . . . . . . .
// x . . . . . . .
// x . . . . . . .
// x . . . . . . .
// x . . . . . . .
// x . . . . . . . 
// x . . . . . . .
// . x x x x x x .
// notice that the edges are excluded, because if a blocker is on the edge, the rook can still cover the edge site
// the only pieces that can meaningfully block the sliding are NOT at the edge of the sliding 
Bitboard RookRelevantBlockersMask(int i, int j);
Bitboard BishopRelevantBlockersMask(int i, int j);
// NAIF HASHING: we simply take the mask of all pieces, apply the relevant mask and use the resulting number as a hash
// rook mask for a1  all pieces bitboard
// . . . . . . . .   . . . . x . . .        . . . . . . . .
// x . . . . . . .   x . . x . . . .        x . . . . . . .
// x . . . . . . .   x . . . . . . x        x . . . . . . .
// x . . . . . . . & . . . . . . . .    =   . . . . . . . . ---> convert to 64-bit integer and use to hash.
// x . . . . . . .   . x . . . . . .        . . . . . . . .
// x . . . . . . .   . . . . . . . .        . . . . . . . .
// x . . . . . . .   . . . . . x . x        . . . . . . . .
// . x x x x x x .   . . . x x . . .        . . . x x . . .
// The problem is that the hash indexes obtained like this are uniformly spread in the range [0, 2^64]. Too much!
// 
// LESS NAIF HASHING: 
// the number of relevant configurations of blockers for the rook on (i,j) is 2^12
// we could thus hash like this:
// . . . . . . . .
// 1 . . . . . . .
// 1 . . . . . . .
// 0 . . . . . . .  -- flattening j-th column and i-th row --> 110000001100
// 0 . . . . . . .
// 0 . . . . . . . 
// 0 . . . . . . .
// . 0 0 1 1 0 0 .
// Although formally correct and with the minimum memory cost, this hashing is SLOW due to the process of flattening the bits
// The reason is that at run-time, flattening is necessary to determine the index for the lookup, and it is done over and over.
//
// MAGIC BITBOARDS
// The 64-bit architecture does bitwise operations faster. 
// The idea is to store the attacks associated to a given configuration of blockers in a 2^n_bits array accessible via an index.
// The index is calculated as:
//      shift = 64 - n_bits
//      index = (blockers & mask_ij) * magic >> shift
// this is basically the product of the relevant blockers with the magic number, where only the first n_bits are considered and the other 64-n_bits are set to 0
// The magic number is a number such that different ATTACKS are given different indexes.
// Since we just want different attacks (and NOT different blockers) to have different indexes, we could also have n_bits < 12
// For example, the following configurations of blockers have the same attack bitboard, and we can store them with the same index (i.e. accept the conflict)
// . . . . . . . .      . . . . . . . .
// x . . . . . . .      . . . . . . . .
// x . . . . . . .      x . . . . . . .
// . . . . . . . .  ;   . . . . . . . .  ;  etc...
// . . . . . . . .      . . . . . . . .
// . . . . . . . .      . . . . . . . .
// . . . . . . . .      . . . . . . . .
// . . . x x . . .      . . . x . . . .
// The magic number is simply generated via trial and error with random numbers.

Bitboard RookCoveredSquaresFromBlockers(Bitboard blockers, int i, int j);
Bitboard BishopCoveredSquaresFromBlockers(Bitboard blockers, int i, int j);
//
Bitboard RookBlockersFromInteger(Bitboard b, int i, int j);
Bitboard BishopBlockersFromInteger(Bitboard b, int i, int j);
//
void FindRookMagic();
void FindBishopMagic();

// function that returns hash index for a given config. of blockers on a gien square
inline uint64_t RookHashIndex(const Bitboard& blockers, const unsigned long& square){
    MaskAndMagic mm = rook_mm[square];
    uint64_t hash_index = ((blockers & mm.mask) * mm.magic) >> SHIFT_ROOK;
    return hash_index;
}
inline uint64_t BishopHashIndex(const Bitboard& blockers, const unsigned long& square){
    MaskAndMagic mm = bishop_mm[square];
    uint64_t hash_index = ((blockers & mm.mask) * mm.magic) >> SHIFT_BISHOP;
    return hash_index;
}

// Functions to run at the engine start that pre-calculates covered squares
void PreComputeBitboards(bool retrieve_from_file);


// generate the bitboard of covered squares from the pieces
Bitboard GetCoveredSquares(Bitboard pieces[12], Bitboard& all_pieces, bool by_white);


// Bitboards to detect passed pawns and outposts
// . . . . . . . .
// x x x . . . . .
// x x x . . . . .
// . o . . . . . . ---> a white pawn on b5 looks ahead at these squares to detect if there are enemy pawns (any rank)
// . . . . . . . . ---> similarly, a knight in b5 is in an outpost square if in these squares there are no enemy pawns
// . . . . . . . .      (only 5th rank)
// . . . . . . . .
// . . . . . . . .
// similarly:
// . . . . . . . .
// . . . . . . . .
// . . . . o . . .
// . . . x x x . . ----> a black pawn on e6 looks at these squares
// . . . x x x . .
// . . . x x x . . 
// . . . x x x . .
// . . . . . . . .
constexpr Bitboard file_A = 0x0101010101010101ULL;
constexpr Bitboard files_bitboards[8] = {
    file_A, file_A << 1, file_A << 2, file_A << 3, file_A << 4, file_A << 5, file_A << 6, file_A << 7
};

constexpr Bitboard rank_1 = 0xFF;
constexpr Bitboard ranks_bitboards[8] = {
    rank_1, rank_1 << (8*1), rank_1 << (8*2), rank_1 << (8*3), rank_1 << (8*4), rank_1 << (8*5), rank_1 << (8*6), rank_1 << (8*7)
};

extern Bitboard mask_white_passed_pawn[64];
extern Bitboard mask_black_passed_pawn[64];

void GetPassedPawnMasks();

// Detect doubled pawns to penalize this situation
// pawn bitboard b ---->    b &= b >> 8   -----> count the bits = n. doubled pawns
// . . . . . . . .        . . . . . . . .
// . . . . . . . .        . . . . . . . .
// . . . . . . . .        . . . . . . . . 
// . x . . . . . . ---->  . x . . . . . . -----> 3
// . x . . . . . .        . x . . . . . .
// . x . . x . x .        . . . . x . . .
// . . . . x . . .        . . . . . . . .
// . . . . . . . .        . . . . . . . .
size_t count_doubled_pawns(Bitboard pawn_bb);
