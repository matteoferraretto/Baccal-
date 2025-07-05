// compile and run with the following commands:
//      cd C:\Users\matte\Documents\Study_C++\ChessEngine
//      mkdir build 
//      cd build
//      cmake --build .
//      .\Debug\ChessEngine.exe

// link libraries
#include <SDL.h>
#include <SDL_render.h>
#include <SDL_ttf.h>
#include <iostream>
#include <cmath>
#include <sstream>
#include <string>
#include <algorithm>
#include "Position.h"
#include "Baccala.h"
#include <unordered_map>
#include <random>

// Random generator of unsigned 64-bit integers
std::mt19937_64 rng(20250704); // Fixed seed for reproducibility
uint64_t rand64() {
    return rng();
}

// Zobrist hashing for the transposition table.
// Let N_positions be the total number of chess positions.
// the idea is that we want to store N positions that we encounter during the min-max search.
// where necessarily N << N_positions. 

// Hashing means that we need to map all these N positions into distinct numbers.
// The Zobrist hashing maps a position into an unsigned 64-bit integer, which can take 2^64 distinct values,
// thus allowing to hash up to 2^64 positions that we encounter (we actually need N way less than that).

// Construct random Zobrist table of uint64_t type 
struct ZobristTable {
    uint64_t pieces_and_squares[12][64];
    uint64_t white_to_move;
    uint64_t castling_rights[4];
    uint64_t en_passant_file[8];
};

void InitializeZobrist(ZobristTable& z){
    for(int i=0; i<12; i++){
        for(int j=0; j<64; j++){
            z.pieces_and_squares[i][j] = rand64();
        }
    }
    z.white_to_move = rand64();
    for(int i=0; i<4; i++){
        z.castling_rights[i] = rand64();
    }
    for(int i=0; i<8; i++){
        z.en_passant_file[i] = rand64();
    }
}

uint64_t Zobrist_Hashing(Position pos) {
    // initialize value of 0
    uint64_t hash = 0;
    ZobristTable z;
    InitializeZobrist(z);

    // Zobrist hashing ...

    return hash;
}

int BestEvaluationAtDepth(Position root_position, int depth, int alpha, int beta, int& n_explored_positions){
    n_explored_positions++;
    // limit case: at depth = 0 just return the material value of the input position
    if(depth == 0){
        return root_position.material_value;
    }
    // else generate all the new positions applying all the legal moves 
    // then recursively call this function and update best_evaluation if needed
    int best_evaluation;
    int eval;
    if(root_position.white_to_move){ best_evaluation = negative_infinity; }
    else{ best_evaluation = positive_infinity; }
    Position child_position = root_position;
    std::vector<Move> moves = root_position.LegalMoves();
    // manage stalemate and checkmate: no legal moves in the current position
    if(moves.size() == 0){
        // WHITE STALEMATED: black to move and the black king is NOT in white's covered squares 
        if(!root_position.white_to_move && !root_position.white_covered_squares_mask[root_position.black_king_square.i][root_position.black_king_square.j]){
            return 0;
        }        
        // BLACK STALEMATED: white to move and the white king is NOT in black's covered squares
        else if(root_position.white_to_move && !root_position.black_covered_squares_mask[root_position.white_king_square.i][root_position.white_king_square.j]){
            return 0;
        }
        // WHITE CHECKMATED: black to move and the black king IS in white's covered squares
        else if(!root_position.white_to_move && root_position.white_covered_squares_mask[root_position.black_king_square.i][root_position.black_king_square.j]){
            return 1000 + depth;  // adding the depth is used to consider a mate in 1 better than a mate in 2 or in 3 etc
        }    
        // BLACK CHECKMATED: white to move and the white king IS in black's covered squares
        else if(root_position.white_to_move && root_position.black_covered_squares_mask[root_position.white_king_square.i][root_position.white_king_square.j]){
            return -1000 - depth;
        }   
    }
    // loop over all legal moves 
    for(Move& move: moves){
        if(root_position.white_to_move){
            eval = BestEvaluationAtDepth(NewPosition(root_position, move), depth-1, alpha, beta, n_explored_positions);
            best_evaluation = std::max(best_evaluation, eval);
            alpha = std::max(alpha, eval); // best evaluation for white encountered so far down the tree
            if(beta <= alpha){ break; } 
        }
        else{
            eval = BestEvaluationAtDepth(NewPosition(root_position, move), depth-1, alpha, beta, n_explored_positions);
            best_evaluation = std::min(best_evaluation, eval);
            beta = std::min(beta, eval);
            if(beta <= alpha){ break; }
        }
    }
    return best_evaluation;
}

Move BestMove(Position root_position, int depth){
    // initialize stuff
    int eval;
    int best_evaluation;
    int n_explored_positions = 0;
    if(root_position.white_to_move){ best_evaluation = negative_infinity; }
    else{ best_evaluation = positive_infinity; }
    Position child_position = root_position;
    std::vector<Move> moves = root_position.LegalMoves();
    Move best_move = moves[0];
    // loop over all the legal moves from the current position
    for(Move& move: moves){
        std::cout << "depth: " << depth << " ; move: " << move.AlgebraicNotation();
        // generate child position and find its best evaluation down the tree 
        child_position = NewPosition(root_position, move);
        eval = BestEvaluationAtDepth(child_position, depth-1, negative_infinity, positive_infinity, n_explored_positions); // depth-1 because we are rooting from the child position
        std::cout << " ; eval: " << eval << "\n";
        // if white to move and the evaluation at given depth of this move is higher than all the previous ones, overwrite best move
        if(root_position.white_to_move){
            if(eval > best_evaluation){ 
                best_evaluation = eval; 
                best_move = move;
            }
            // if this is mate in 1, this MUST be the best move and no further search is required
            if(best_evaluation == 1000 + depth - 1){ break; }
        }
        // if black to move and the evaluation at given depth of this move is lower than all the previous ones, overwrite best move
        else{
            if(eval < best_evaluation){ 
                best_evaluation = eval; 
                best_move = move;
            }
            // if this is mate in 1, this MUST be the best move and no further search is required
            if(best_evaluation == -1000 - depth + 1){ break; }
        }
    }
    std::cout << "I have considered " << n_explored_positions << " positions. \n";
    return best_move;
}


int main(int argc, char* argv[]){
    
    std::string random_position_fen = "8/5k2/4p1p1/2N1PpP1/5P2/5K2/8/3n4 w - - 0 50";
    std::cout << "Insert a position via FEN string: ";
    std::getline (std::cin, random_position_fen);
    int depth;
    std::cout << "Insert desired calculation depth: ";
    std::cin >> depth;
    Position pos = Position(random_position_fen);
    pos.PrintBoard();
    for(Move& move: pos.LegalMoves()){
        std::cout << move.AlgebraicNotation() << "\n";
    }

/*
    std::vector<Move> legal_moves = pos.LegalMoves();
    std::vector<Position> new_positions;
    new_positions.reserve(legal_moves.size());
    Position new_pos = pos;
    for(Move& move: legal_moves){
        std::cout << move.AlgebraicNotation() << "\n";
        new_pos = NewPosition(pos, move);
        new_pos.PrintBoard();
        std::cout << new_pos.material_value << "\n";
    }
*/

    /*
    Square current_square; current_square.i = 6; current_square.j = 1;
    PrintMask(
        QueenMovesMask(current_square, empty_mask, empty_mask)
    );
    PrintMoves(
        BlackPawnTargetSquares(current_square, empty_mask, empty_mask)
    );
    //std::vector<Square> moves = KnightTargetSquares(current_square, empty_mask, empty_mask);
    //PrintMoves(moves);
    */
    std::cout << "Working at depth: " << depth << "\n";
    std::cout << "Best move is: " << BestMove(pos, depth).AlgebraicNotation() << "\n";
    return 0;
}