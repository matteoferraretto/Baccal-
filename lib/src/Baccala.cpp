#pragma once
#include <Baccala.h>
#include <Move.h>
#include <Position.h>
#include <Utilities.h>
#include <TranspositionTable.h>
#include <algorithm>
#include <unordered_map>
#include <iostream>


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

bool SafeNullMoveSearch(Position& pos){
    // if the side to move is in check, it is NOT safe to skip a move
    if(pos.white_to_move){
        if((pos.pieces[0] & pos.black_covered_squares) != 0){ return false; }
    }
    else{
        if((pos.pieces[6] & pos.white_covered_squares) != 0){ return false; }
    }
    // if very few pieces are remaining (<= 6) avoid it
    if(pop_count(pos.all_pieces) <= 6){ return false; }
    // if only pawns and kings are remaining (i.e. NO other pieces!)
    // add-up bitboard of pieces (no kings, no pawns) and check if it is zero
    if((pos.pieces[1] | pos.pieces[2] | pos.pieces[3] | pos.pieces[4] | pos.pieces[7] | pos.pieces[8] | pos.pieces[9] | pos.pieces[10]) == 0){
        return false;
    }
    // more safety checks? 
    // ...
    // in all the other cases, we are good to go
    return true;
}



int BestEvaluation(Position& pos, int anti_depth, int alpha, int beta, int& n_explored_positions, bool can_do_null){
    // ------------------------------------------------------
    // ----- RETRIEVE SCORE FROM TRANSPOSITION TABLE --------
    // ------------------------------------------------------
    // compute Zobrist key for the current position
    uint64_t zobrist_key = ZobristHashing(pos);
    // check if the move is already present in the transposition table:
    // if yes return a pointer to its memory address; if no return nullptr
    TTEntry* entry = TTProbe(zobrist_key);
    // if the position is store and it has been analyzed better than what we are about to do here
    // then just return the already found score
    if(entry && entry->depth >= anti_depth){
        if (entry->flag == EXACT)
            return entry->score;
        else if (entry->flag == LOWERBOUND && entry->score >= beta)
            return entry->score;
        else if (entry->flag == UPPERBOUND && entry->score <= alpha)
            return entry->score;
    }

    // -----------------------------------------------------------------------
    // -- MANAGE EARLY EXIT CASES: DEPTH=0; DRAWS; STALEMATE; CHECKMATE ETC --
    // -----------------------------------------------------------------------
    // count considered positions (total number of nodes)
    n_explored_positions++; 
    // manage 50-moves rule
    if(pos.half_move_counter >= 50){
        return 0;
    }
    // limit case: at anti_depth = 0 just return the material value of the input position
    if(anti_depth == 0){
        return PositionScore(pos);
    }
    // else generate all the new positions applying all the legal moves 
    // then recursively call this function and update best_evaluation if needed
    int eval, best_evaluation;
    MoveAndPosition move_and_pos;
    std::vector<MoveAndPosition> legal_moves = LegalMoves(pos);
    std::size_t n_moves = legal_moves.size();
    // manage stalemate and checkmate: no legal moves in the current position
    if(n_moves == 0){
        if(pos.white_to_move){
            // BLACK STALEMATED: white to move and the white king is NOT in black's covered squares 
            if((pos.black_covered_squares & pos.pieces[0]) == 0){
                return 0; // it's a draw
            }
            // BLACK CHECKMATED
            else{ return -100000 - anti_depth; }
        }
        else{
            // WHITE STALEMATED: black to move and the black king is NOT in white's covered squares 
            if((pos.white_covered_squares & pos.pieces[6]) == 0){
                return 0; // it's a draw
            }
            // WHITE CHECKMATED: black to move and the black king is in check
            else{ return 100000 + anti_depth; } // adding the depth is used to consider a mate in 1 better than a mate in 2 or in 3 etc
        }
    }
    pos.white_to_move ? best_evaluation = negative_infinity : best_evaluation = positive_infinity;

    // ---------------------------------
    // ------ NULL MOVE PRUNING --------
    // ---------------------------------
    /*if(can_do_null && anti_depth >= 3){ // only applied far from the horizon
        if(SafeNullMoveSearch(pos)){
            int r = 2; // reduction of depth search
            if(pos.white_to_move){
                // make null move
                Position new_pos = pos; 
                new_pos.white_to_move = false; 
                new_pos.en_passant_target_square = 0;
                // launch a shallow evaluation function with no rights of making null move
                // the evaluation is 2 plies shorter than a normal search
                eval = -BestEvaluation(new_pos, anti_depth - r, -beta, -beta + 1, n_explored_positions, false);
            }
            else{
                // make null move
                Position new_pos = pos; 
                new_pos.white_to_move = true; 
                new_pos.en_passant_target_square = 0;
                // launch a shallow evaluation function with no rights of making null move
                // the evaluation is 2 plies shorter than a normal search
                eval = -BestEvaluation(new_pos, anti_depth - r, -beta, -beta+1, n_explored_positions, false);
            }
            if(eval >= beta){ return eval; }
        }
    } */

    // ---------------------------------------------------------
    // ------ MIN - MAX SEARCH WITH ALPHA - BETA PRUNING -------
    // ---------------------------------------------------------
    int original_alpha = alpha, original_beta = beta;
    // Loop through the legal moves to assign a heuristic score
    ScoreAllMoves(legal_moves);
    // Loop again to recursively iterate the function 
    for(int move_index = 0; move_index < n_moves; move_index++){
        // pick the best move in the range [move_index + 1, n_moves] and bring it to the current index
        PickBestMove(legal_moves, n_moves, move_index);
        move_and_pos = legal_moves[move_index];
        // white to move
        if(pos.white_to_move){
            eval = BestEvaluation(move_and_pos.position, anti_depth - 1, alpha, beta, n_explored_positions, true);
            best_evaluation = std::max(best_evaluation, eval);
            if(best_evaluation >= 100000){ break; }
            alpha = std::max(alpha, eval); // best evaluation for white encountered so far down the tree
            if(beta <= alpha){ break; } 
        }
        // black to move
        else{
            eval = BestEvaluation(move_and_pos.position, anti_depth - 1, alpha, beta, n_explored_positions, true);
            best_evaluation = std::min(best_evaluation, eval);
            if(best_evaluation <= -100000){ break; }
            beta = std::min(beta, eval);
            if(beta <= alpha){ break; }
        }

    // --------------------------------------------------------
    // ------ STORE POSITION IN THE TRANSPOSITION TABLE -------
    // --------------------------------------------------------
    NodeFlag flag;
    if (best_evaluation <= original_alpha)
        flag = UPPERBOUND;
    else if (best_evaluation >= original_beta)
        flag = LOWERBOUND;
    else
        flag = EXACT;
    TTStore(anti_depth, zobrist_key, best_evaluation, flag);
    
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
    best_move = legal_moves[0];
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
        eval = BestEvaluation(m.position, depth-1, negative_infinity, positive_infinity, /*TranspositionTable,*/ n_explored_positions, true); // depth-1 because we are rooting from the child position
        std::cout << "eval: " << eval << "\n";
        // if white to move and the evaluation at given depth of this move is higher than all the previous ones, overwrite best move
        if(pos.white_to_move){
            if(eval > best_evaluation){ 
                best_evaluation = eval; 
                best_move = legal_moves[move_index];
            }
            // if this is mate in 1, this MUST be the best move and no further search is required
            if(best_evaluation == 100000 + depth - 1 ){ break; }
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

MoveAndPosition IterativeDeepening(Position& pos, int min_depth, int max_depth, int depth_step){
    int eval;
    int best_evaluation;
    int n_explored_positions;
    bool win_detected = false, loss_detected = false;
    MoveAndPosition m, best_move;
    std::vector<MoveAndPosition> legal_moves = LegalMoves(pos);
    std::size_t n_moves = legal_moves.size();
    best_move = legal_moves[0];
    // Loop through the legal moves to assign a heuristic score
    ScoreAllMoves(legal_moves);
    // initialize the hash-map for the Transposition table
/*    std::unordered_map<uint64_t, Position> TranspositionTable;
    TranspositionTable.reserve(max_capacity_transposition_table);*/
    // Start Iterative deepening
    for(int depth = min_depth; depth <= max_depth; depth += depth_step){
        n_explored_positions = 0;
        std::cout << "Iterative deepening at depth " << depth << "\n";
        if(pos.white_to_move){
            best_evaluation = negative_infinity; 
        }
        else{
            best_evaluation = positive_infinity;
        }
        // loop over all the legal moves from the current position
        for(int move_index = 0; move_index < n_moves; move_index++){
            // pick move with highest score
            PickBestMove(legal_moves, n_moves, move_index);
            m = legal_moves[move_index];
            std::cout << "move: "; PrintMove(m.move);
            // generate child position and find its best evaluation down the tree 
            eval = BestEvaluation(m.position, depth-1, negative_infinity, positive_infinity, /*TranspositionTable,*/ n_explored_positions, true); // depth-1 because we are rooting from the child position
            std::cout << "eval: " << eval << "\n";
            // if white to move and the evaluation at given depth of this move is higher than all the previous ones, overwrite best move
            if(pos.white_to_move){
                legal_moves[move_index].score = eval; // update score with the evaluation at current depth
                if(eval > best_evaluation){ 
                    best_evaluation = eval; 
                    best_move = legal_moves[move_index];
                }
                if(best_evaluation >= 100000){ 
                    win_detected = true; 
                    // if this is mate in 1, this MUST be the best move and no further search is required
                    if(best_evaluation == 100000 + depth - 1 ){ break; }
                }
            }
            // if black to move and the evaluation at given depth of this move is lower than all the previous ones, overwrite best move
            else{
                legal_moves[move_index].score = -eval; // update score with the evaluation at current depth
                if(eval < best_evaluation){ 
                    best_evaluation = eval; 
                    best_move = legal_moves[move_index];
                }
                if(best_evaluation <= -100000){
                    win_detected = true;
                    // if this is mate in 1, this MUST be the best move and no further search is required
                    if(best_evaluation == -100000 - depth + 1){ break; }
                }
            }
        }
        std::cout << "I have considered " << n_explored_positions << " positions. \n";
        std::cout << "The best move is "; PrintMove(best_move.move);
        if(win_detected){ break; }
        // if a forced mate is found, there's no need to search deeper 
    }
    return best_move;
}
