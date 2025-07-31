#pragma once
#include <Move.h>
#include <Utilities.h>
#include <Position.h>

// An entry of the transposition table represents what we have evaluated about an already encountered position
// it contains the following variables:
//  - depth: this is the distance between the evaluated position and the search horizon
//  - hash: is an identifier of the position, i.e. a number labeling the position (see Zobrist key below)
//  - score: is the score that we have assigned to the position when we encountered it
//  - best_move: is the best move that we have evaluated
//  - flag: determines the node type: 
//          - EXACT -> means no pruning: the assigned score is obtained by searching till the horizon
//          - LOWERBOUND -> pruning happened because score > beta
//          - UPPERBOUND -> pruning happened because score < alpha
enum NodeFlag {
    EXACT, LOWERBOUND, UPPERBOUND
};

struct TTEntry {
    int depth;
    uint64_t hash;
    int score;
    NodeFlag flag;
    //Move best_move;
};

// Transposition Table (TT)
// it contains a table and methods to clear, fill or access the table
const int TT_SIZE = 1 << 20; // 2^18 \approx 250 000 ---> we can do better!!

extern TTEntry transposition_table[TT_SIZE];

void TTInit();

// check if the given zobrist_key matches some entry in the transposition table
// and in case of success, return a pointer to that entry
TTEntry* TTProbe(uint64_t zobrist_key);

// store entry in the table ONLY in 2 cases:
// - if the table at that index is empty
// - if the depth of the entry that we are storing is greater than the depth of the entry that we attempt to overwrite
void TTStore(int depth, uint64_t hash, int score, NodeFlag flag/*, Move best_move*/);

// Zobrist hashing is a method to map a position to a (almost unique) number:
//                   Zobrist hashing
//      Position ---------------------> uint64_t
//
// Of course the number of legal chess positions is greater than 2^64
// which means that this function cannot be injective, i.e. different positions can be hashed to the same number! 
// A conflict could lead to allucination of the engine;
// nevertheless, with the Zobrist method, the probability of conflict is low, and we accept the risk
//
// Zobrist hashing works like this:
// 1. create a Zobrist table with a bunch of uint64_t RANDOM numbers: 
//        12 (n. pieces) x 64 (n. squares) + 1 (side to move) + 4 (n. castling rights) + 8 (n. en passant target files)
// 2. start from hash = 0 (64 bit) and xor hash with all the random numbers like this:
//        if p-th piece is in the s-th square, do hash ^= ZobristTable.pieces_and_squares[p][s]
//        if white to move, do hash ^= ZobristTable.white_to_move
//        if white can castle kingside, do hash ^= ZobristTable.castling_rights[0]
//        ....
//        if black can castle queenside, do hash ^= ZobristTable.castling_rights[3]
//        if there's an en-passant target file j, do hash ^= ZobristTable.en_passant_file[j]

// Zobrist table (struct and initialization)
struct ZobristTable {
    uint64_t pieces_and_squares[12][64];
    uint64_t white_to_move;
    uint64_t castling_rights[4];
    uint64_t en_passant_file[8];
};

extern ZobristTable zobrist_table;

void InitializeZobrist();

// careful: this function is called inside the search loop: it has to be as fast as possible
uint64_t ZobristHashing(Position& pos);