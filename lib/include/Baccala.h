#pragma once
#include <Position.h>

// Assign a heuristic score to all the moves
void ScoreAllMoves(MoveAndPosition* moves, uint8_t n_moves);

// PICK MOVE
// In the alpha-beta pruning, instead of sorting the moves, we just pick the move with highest score 
// and we swap it to move it to the current index.
// In the min-max search, we loop over the unsorted moves 
// if we are currently considering the i-th move, we swap legal_moves[i] with legal_moves[best_idx],
// where best_idx is the index of the move with highest score considering only moves from index i to n_moves.
void PickBestMove(MoveAndPosition* moves, uint8_t n_moves, int i);

// PERFT: performance testing
// this is a standard test to check performance of move generation
// Perft(pos, depth) returns the number of nodes at the horizon obtained from a given position at a given depth
// this is also useful for debugging
// see results at https://www.chessprogramming.org/Perft_Results
unsigned long long int Perft(Position pos, int depth);
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
// NULL MOVE LOGIC
bool SafeNullMoveSearch(Position& pos);
int BestEvaluation(Position& pos, int anti_depth, int alpha, int beta, /*std::unordered_map<uint64_t, Position>& TranspositionTable,*/ int& n_explored_positions, bool can_do_null);
MoveAndPosition BestMove(Position pos, int depth);

// ITERATIVE DEEPENING
MoveAndPosition IterativeDeepening(Position& pos, int min_depth, int max_depth, int depth_step);