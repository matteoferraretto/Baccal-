#pragma once
#include <Baccala.h>
#include <Move.h>
#include <Position.h>
#include <algorithm>

void ScoreAllMoves(std::vector<MoveAndPosition>& moves){
    for(MoveAndPosition& m : moves){
        m.score = ScoreMove(m.move);
    }
}

void PickBestMove(std::vector<MoveAndPosition>& moves, std::size_t n_moves, int i){
    int best_index = i; // assume the current move is best
    // Loop over remaining moves: ... i+1, i+2, ... , n_moves
    for(int j = i+1; j < n_moves; j++){
        // if the j-th move scores better than the current move, update best_index
        if(moves[j].score > moves[best_index].score){
            best_index = j;
        }
    }
    // now best_index refers to the move with highest score above i
    iter_swap(moves.begin() + i, moves.begin() + best_index);
}

int BestEvaluation(Position& pos, int anti_depth, int alpha, int beta, /*std::unordered_map<uint64_t, Position>& TranspositionTable,*/ int& n_explored_positions){
/*
    // compute hash for the current position
    uint64_t hash = Zobrist_Hashing(root_position, z);
    // add position to transposition table unless we have already used too much memory
    if(TranspositionTable.size() < max_capacity_transposition_table){
        // if not present, add position to the transposition table
        std::pair<std::unordered_map<uint64_t, Position>::iterator, bool> emplacing_info;
        emplacing_info = TranspositionTable.emplace(hash, root_position);
        bool inserted = emplacing_info.second;
        // if insertion failed, position has already been evaluated and we can retrieve and return its value
        if(!inserted){
            //std::cout << "hit duplicate position\n";
            return root_position.score;
        }
    }
    else{
        if(TranspositionTable.find(hash) != TranspositionTable.end()){
            //std::cout << "hit duplicate position after maxing out storage of hash map\n";
            return root_position.score;
        }
    }
*/
    // count considered positions (total number of nodes)
    n_explored_positions++; 
    // limit case: at anti_depth = 0 just return the material value of the input position
    if(anti_depth == 0){
        return pos.white_material_value - pos.black_material_value;
    }
    // manage 50-moves rule
    if(pos.half_move_counter >= 50){
        return 0;
    }
    // else generate all the new positions applying all the legal moves 
    // then recursively call this function and update best_evaluation if needed
    int eval, best_evaluation;
    if(pos.white_to_move){
        best_evaluation = negative_infinity; 
    }
    else{
        best_evaluation = positive_infinity;
    }
    Position new_pos;
    MoveAndPosition move_and_pos;
    std::vector<MoveAndPosition> legal_moves = LegalMoves(pos);
    std::size_t n_moves = legal_moves.size();
    // manage stalemate and checkmate: no legal moves in the current position
    if(n_moves == 0){
        // WHITE TO MOVE
        if(pos.white_to_move){
            // BLACK STALEMATED: white to move and the white king is NOT in black's covered squares 
            if((pos.black_covered_squares & pos.pieces[0]) == 0){
                return 0; // it's a draw
            }
            // BLACK CHECKMATED
            else{
                return -100000 - anti_depth;
            }
        }
        // BLACK TO MOVE
        else{
            // WHITE STALEMATED: black to move and the black king is NOT in white's covered squares 
            if((pos.white_covered_squares & pos.pieces[6]) == 0){
                return 0; // it's a draw
            }
            // WHITE CHECKMATED: black to move and the black king is in check
            else{
                return 100000 + anti_depth;  // adding the depth is used to consider a mate in 1 better than a mate in 2 or in 3 etc
            }  
        }
    }
    // Loop through the legal moves to assign a heuristic score
    ScoreAllMoves(legal_moves);
    // Loop again to recursively iterate the function 
    for(int move_index = 0; move_index < n_moves; move_index++){
        // pick the best move in the range [move_index + 1, n_moves] and bring it to the current index
        PickBestMove(legal_moves, n_moves, move_index);
        move_and_pos = legal_moves[move_index];
        // MIN - MAX PROCEDURE WITH ALPHA - BETA PRUNING
        // white to move
        if(pos.white_to_move){
            eval = BestEvaluation(move_and_pos.position, anti_depth-1, alpha, beta, /*TranspositionTable,*/ n_explored_positions);
            best_evaluation = std::max(best_evaluation, eval);
            alpha = std::max(alpha, eval); // best evaluation for white encountered so far down the tree
            if(beta <= alpha){ break; } 
        }
        // black to move
        else{
            eval = BestEvaluation(move_and_pos.position, anti_depth-1, alpha, beta, /*TranspositionTable,*/ n_explored_positions);
            best_evaluation = std::min(best_evaluation, eval);
            beta = std::min(beta, eval);
            if(beta <= alpha){ break; }
        }
    }
    return best_evaluation;
}


MoveAndPosition BestMove(Position pos, int depth){
    // initialize stuff
    int eval;
    int best_evaluation;
    int n_explored_positions = 0;
    MoveAndPosition m, best_move;
    if(pos.white_to_move){
        best_evaluation = negative_infinity; 
    }
    else{
        best_evaluation = positive_infinity;
    }
    std::vector<MoveAndPosition> legal_moves = LegalMoves(pos);
    std::size_t n_moves = legal_moves.size();
    // Loop through the legal moves to assign a heuristic score
    ScoreAllMoves(legal_moves);
    // initialize the hash-map for the Transposition table
/*    std::unordered_map<uint64_t, Position> TranspositionTable;
    TranspositionTable.reserve(max_capacity_transposition_table);*/
    // loop over all the legal moves from the current position
    for(int move_index = 0; move_index < n_moves; move_index++){
        // pick move with highest score
        PickBestMove(legal_moves, n_moves, move_index);
        m = legal_moves[move_index];
        std::cout << "depth: " << depth << " ; move: "; PrintMove(m.move);
        // generate child position and find its best evaluation down the tree 
        eval = BestEvaluation(m.position, depth-1, negative_infinity, positive_infinity, /*TranspositionTable,*/ n_explored_positions); // depth-1 because we are rooting from the child position
        std::cout << " ; eval: " << eval << "\n";
        // if white to move and the evaluation at given depth of this move is higher than all the previous ones, overwrite best move
        if(pos.white_to_move){
            if(eval > best_evaluation){ 
                best_evaluation = eval; 
                best_move = legal_moves[move_index];
            }
            // if this is mate in 1, this MUST be the best move and no further search is required
            if(best_evaluation == 100000 + depth - 1){ break; }
        }
        // if black to move and the evaluation at given depth of this move is lower than all the previous ones, overwrite best move
        else{
            if(eval < best_evaluation){ 
                best_evaluation = eval; 
                best_move = legal_moves[move_index];
            }
            // if this is mate in 1, this MUST be the best move and no further search is required
            if(best_evaluation == -100000 - depth + 1){ break; }
        }
    }
    std::cout << "I have considered " << n_explored_positions << " positions. \n";
    std::cout << "The best move is "; PrintMove(best_move.move);
    return best_move;
}