#pragma once
#include <Baccala.h>
#include <Move.h>
#include <Position.h>
#include <Utilities.h>
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

int BestEvaluation(Position& pos, int anti_depth, int alpha, int beta, /*std::unordered_map<uint64_t, int>& TranspositionTable,*/ int& n_explored_positions){
/*
    // compute hash for the current position
    uint64_t hash = Zobrist_Hashing(pos, zobrist_table);
    // add position to transposition table unless we have already used too much memory
    if(TranspositionTable.size() < MAX_CAPACITY_TT){
        // if not present, add position to the transposition table
        std::pair<std::unordered_map<uint64_t, int>::iterator, bool> emplacing_info;
        emplacing_info = TranspositionTable.emplace(hash, root_position);
        bool inserted = emplacing_info.second;
        // if insertion failed, position has already been evaluated and we can retrieve and return its value
        if(!inserted){
            //std::cout << "hit duplicate position\n";
            return pos.white_material_value + pos.black_material_value;
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
        return pos.white_material_value + pos.black_material_value;
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
            if(best_evaluation >= 100000){ break; }
            alpha = std::max(alpha, eval); // best evaluation for white encountered so far down the tree
            if(beta <= alpha){ break; } 
        }
        // black to move
        else{
            eval = BestEvaluation(move_and_pos.position, anti_depth-1, alpha, beta, /*TranspositionTable,*/ n_explored_positions);
            best_evaluation = std::min(best_evaluation, eval);
            if(best_evaluation <= -100000){ break; }
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
        eval = BestEvaluation(m.position, depth-1, negative_infinity, positive_infinity, /*TranspositionTable,*/ n_explored_positions); // depth-1 because we are rooting from the child position
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
            eval = BestEvaluation(m.position, depth-1, negative_infinity, positive_infinity, /*TranspositionTable,*/ n_explored_positions); // depth-1 because we are rooting from the child position
            std::cout << "eval: " << eval << "\t" << best_evaluation << "\n";
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

ZobristTable InitializeZobrist(){
    ZobristTable z;
    for(int piece = 0; piece < 12; piece++){
        for(int square = 0; square < 64; square++){
            z.pieces_and_squares[piece][square] = rand64();
        }
    }
    z.white_to_move = rand64();
    for(int i=0; i<4; i++){
        z.castling_rights[i] = rand64();
    }
    for(int i=0; i<8; i++){
        z.en_passant_file[i] = rand64();
    }
    return z;
}


uint64_t Zobrist_Hashing(Position& pos, ZobristTable& z) {
    // initialize value of 0
    uint64_t hash = 0;
    // Zobrist hashing ...
    for(int square = 0; square < 64; square++){
        for(int piece_index = 0; piece_index < 12; piece_index++){
            if(bit_get(pos.pieces[piece_index], square)){
                hash ^= z.pieces_and_squares[piece_index][square];
                continue;
            }
        }
    }
    if(pos.white_to_move){ hash ^= z.white_to_move; }
    if(pos.can_white_castle_kingside){ hash ^= z.castling_rights[0]; }
    else if(pos.can_white_castle_queenside){ hash ^= z.castling_rights[1]; }
    else if(pos.can_black_castle_kingside){ hash ^= z.castling_rights[2]; }
    else if(pos.can_black_castle_queenside){ hash ^= z.castling_rights[3]; }
    for(int j=0; j<8; j++){
        if(pos.en_passant_target_square % 8 == j){ hash ^= z.en_passant_file[j]; }
    }
    return hash;
}