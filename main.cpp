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
#include "Baccala.h"

const int negative_infinity = std::numeric_limits<int>::min();
const int positive_infinity = std::numeric_limits<int>::max();

int BestEvaluationAtDepth(Position root_position, int depth, int alpha, int beta){
    // limit case: at depth = 0 just return the material value of the input position
    if(depth == 0){
        return root_position.material_value;
    }
    // else generate all the new positions applying all the legal moves 
    // then recursively call this function and update best_evaluation if needed
    int best_evaluation;
    int eval;
    if(root_position.white_to_move){ 
        best_evaluation = negative_infinity; 
    }
    else{ 
        best_evaluation = positive_infinity; 
    }
    Position child_position = root_position;
    // loop over all legal moves 
    for(Move& move: root_position.LegalMoves()){
        if(root_position.white_to_move){
            eval = BestEvaluationAtDepth(NewPosition(root_position, move), depth-1, alpha, beta);
            best_evaluation = std::max(best_evaluation, eval);
            alpha = std::max(alpha, eval); // best evaluation for white encountered so far down the tree
            if(beta <= alpha){ break; } 
        }
        else{
            eval = BestEvaluationAtDepth(NewPosition(root_position, move), depth-1, alpha, beta);
            best_evaluation = std::min(best_evaluation, eval);
            beta = std::min(beta, eval);
            if(beta <= alpha){ break; }
        }
    }
    /*
    // find the move that gives the maximum (if white to move) or minimum (if black to move) material value
    std::vector<int>::iterator max_it;
    if(pos.white_to_move){ max_it = std::max_element(material_values.begin(), material_values.end()); }
    else{ max_it = std::min_element(material_values.begin(), material_values.end()); }
    int max_pos = std::distance(material_values.begin(), max_it);
    best_move = legal_moves[max_pos];
    // iteratively apply the function 
    for(Position& new_pos: new_positions){
        best_move = BestMove(new_pos, iteration+1, depth);
    }
    // */
    return best_evaluation;
}

Move BestMove(Position root_position, int depth){
    // initialize stuff
    int eval;
    int best_evaluation;
    if(root_position.white_to_move){ best_evaluation = std::numeric_limits<int>::min(); }
    else{ best_evaluation = std::numeric_limits<int>::max(); }
    Position child_position = root_position;
    std::vector<Move> moves = root_position.LegalMoves();
    Move best_move = moves[0];
    // loop over all the legal moves from the current position
    for(Move& move: moves){
        std::cout << "depth: " << depth << " ; move: " << move.AlgebraicNotation();
        // generate child position and find its best evaluation down the tree 
        child_position = NewPosition(root_position, move);
        eval = BestEvaluationAtDepth(child_position, depth-1, negative_infinity, positive_infinity); // depth-1 because we are rooting from the child position
        std::cout << " ; eval: " << eval << "\n";
        // if white to move and the evaluation at given depth of this move is higher than all the previous ones, overwrite best move
        if(root_position.white_to_move){
            if(eval > best_evaluation){ 
                best_evaluation = eval; 
                best_move = move;
            }
        }
        // if black to move and the evaluation at given depth of this move is lower than all the previous ones, overwrite best move
        else{
            if(eval < best_evaluation){ 
                best_evaluation = eval; 
                best_move = move;
            }
        }
    }
    return best_move;
}

int main(int argc, char* argv[]){
    
    std::string random_position_fen = "8/5k2/4p1p1/2N1PpP1/5P2/5K2/8/3n4 w - - 0 50";
    std::cout << "Insert a position via FEN string: ";
    std::getline (std::cin, random_position_fen);
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
    int depth = 5;
//    std::cout << "Best move is: " << BestMove(pos, 0, 2).AlgebraicNotation() << "\n";
    std::cout << "Working at depth: " << depth << "\n";
    std::cout << "Best move is: " << BestMove(pos, depth).AlgebraicNotation() << "\n";
    return 0;
}