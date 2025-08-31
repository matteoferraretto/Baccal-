#pragma once
#include <Position.h>

extern uint64_t N_EXPLORED_NODES;
extern uint64_t N_CUTOFFS;
extern int PLY;

// Initialize history heuristics
void HistoryInit();

// PERFT: performance testing
// this is a standard test to check performance of move generation
// Perft(pos, depth) returns the number of nodes at the horizon obtained from a given position at a given depth
// this is also useful for debugging
// see results at https://www.chessprogramming.org/Perft_Results
unsigned long long int Perft(Position& pos, int depth);
void PerftTesting();

// MIN - MAX SEARCH with ALPHA - BETA PRUNING
// at every node of the search we have two values as estimated so far:
//  - alpha is the MINIMUM score that white (the maximizing player) can obtain so far: they can do at least this or better;
//  - beta is the MAXIMUM score that black (the minimizing player) can obtain so far: they can do at most this score or better
//
// Let's consider a position P (node), and let's apply a move M that opens a subtree. 
// We explore the future and obtain an evaluation for M
// exploring the future, we also update alpha and beta
// 
// During the search we encounter 3 types of nodes:
//  - EXACT NODES -> alpha < eval < beta: 
//          the move is better than any other move seen so far for the side to move and cannot be refuted by the opponent 
//          we use it to update alpha or beta
//  - FAIL HIGH NODES -> if white moves and eval > beta; or if black moves and eval < alpha
//          this means that the current move is very good for the side to move, but the opponent can avoid this line!
//          we can prune this branch because the opponent will never allow this line.
//  - FAIL LOW NODES: let best_eval be the eval of the best move for the side to move
//          if white moves and best_eval < alpha or if black moves and best_eval > beta
//          At this node, the side to move has no interesting options because all the moves lead to an evaluation which is worse than what we have found elsewhere
int BestEvaluation(Position& pos, int depth, int alpha, int beta, bool previous_null);

// QUIESCENCE SEARCH: at the leaf nodes, extend the search for aggressive moves to fully consider tactics
const int MAX_QUIESCE_DEPTH = 12;
int QuiescenceSearch(Position& pos, int alpha, int beta, int quiesce_ply);

// NULL MOVE LOGIC
const int MIN_DEPTH_NULL = 4;
const int DEPTH_REDUCTION_NULL = 3;
bool NullMoveOk(const Position& pos, int depth, bool previous_null);

// ITERATIVE DEEPENING
Move IterativeDeepening(Position& pos, int min_depth, int max_depth, int depth_step);
Move QuietIterativeDeepening(Position& pos, int max_depth);

// LATE MOVE REDUCTION (LMR): after a given depth, research a bunch of moves at full depth, while the late moves are searched at reduced depth
const int MIN_DEPTH_LMR = 6;
const int N_MOVES_FULL_DEPTH = 3;
const int DEPTH_REDUCTION_LMR = 1;
extern bool LMR_ACTIVE;

// facility to print the PV (Principal Variation)
void PrintPV(Position& pos);