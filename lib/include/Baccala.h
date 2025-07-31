#pragma once
#include <Position.h>

// Assign a heuristic score to all the moves
void ScoreAllMoves(std::vector<MoveAndPosition>& moves);

// PICK MOVE
// In the alpha-beta pruning, instead of sorting the moves, we just pick the move with highest score 
// and we swap it to move it to the current index.
// In the min-max search, we loop over the unsorted moves 
// if we are currently considering the i-th move, we swap legal_moves[i] with legal_moves[best_idx],
// where best_idx is the index of the move with highest score considering only moves from index i to n_moves.
void PickBestMove(std::vector<MoveAndPosition>& moves, std::size_t n_moves, int i);

// MIN - MAX SEARCH with ALPHA - BETA PRUNING
int BestEvaluation(Position& pos, int anti_depth, int alpha, int beta, /*std::unordered_map<uint64_t, Position>& TranspositionTable,*/ int& n_explored_positions);
MoveAndPosition BestMove(Position pos, int depth);

// ITERATIVE DEEPENING
MoveAndPosition IterativeDeepening(Position& pos, int min_depth, int max_depth, int depth_step);