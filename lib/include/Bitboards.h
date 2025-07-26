#pragma once
#include <cstdint>
#include <Utilities.h>

const int n_bits_rook = 12; // bits used for magic hashing with rooks
const int n_bits_bishop = 9; // bits used for magic hashing with bishops
const int n_attacks_rook = 4096; // 2^12
const int n_attacks_bishop = 512; // 2^9

// arrays that store all the possible movements of the non-sliding pieces
// this is used to avoid on-the-fly calculation during the min-max process
extern uint64_t knight_covered_squares_bitboards[64]; 
extern uint64_t king_covered_squares_bitboards[64];
extern uint64_t white_pawn_covered_squares_bitboards[64];
extern uint64_t black_pawn_covered_squares_bitboards[64]; // when pawns are in first or last raw the array contains garbage
extern uint64_t white_pawn_advance_squares_bitboards[64];
extern uint64_t black_pawn_advance_squares_bitboards[64];

// masks of relevant masks of sliding pieces
extern uint64_t rook_masks[64];
extern uint64_t rook_magics[64];
extern uint64_t *rook_covered_squares_bitboards;

extern uint64_t bishop_masks[64];
extern uint64_t bishop_magics[64];
extern uint64_t *bishop_covered_squares_bitboards;

// functions that generate covered square bitboards for non-sliding pieces when the piece is in the square (i, j)
uint64_t knight_covered_squares(int i, int j);
uint64_t king_covered_squares(int i, int j);
uint64_t white_pawn_covered_squares(int i, int j);
uint64_t white_pawn_advance_squares(int i, int j);
uint64_t black_pawn_covered_squares(int i, int j);
uint64_t black_pawn_advance_squares(int i, int j);


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
uint64_t rook_relevant_blockers_mask(int i, int j);
uint64_t bishop_relevant_blockers_mask(int i, int j);
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
uint64_t rook_covered_squares_from_blockers(uint64_t blockers, int i, int j);
uint64_t bishop_covered_squares_from_blockers(uint64_t blockers, int i, int j);
//
uint64_t rook_blockers_from_integer(uint64_t b, int i, int j);
uint64_t bishop_blockers_from_integer(uint64_t b, int i, int j);
//
void find_rook_magic(unsigned int n_bits, uint64_t *attacks, uint64_t magics[64]);
void find_bishop_magic(unsigned int n_bits, uint64_t *attacks, uint64_t magics[64]);
// function that returns hash index for a given config. of blockers on a gien square
uint64_t rook_hash_index(uint64_t blockers, int square, int n_attacks);
uint64_t bishop_hash_index(uint64_t blockers, int square, int n_attacks);

// Functions to run at the engine start that pre-calculates covered squares
void PreComputeBitboards();

// clean-up
void CleanBitboards();

// generate the bitboard of covered squares from the pieces
uint64_t GetCoveredSquares(uint64_t pieces[12], uint64_t& all_pieces, bool by_white);