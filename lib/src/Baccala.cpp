#pragma once
#include <Baccala.h>
#include <Move.h>
#include <Position.h>
#include <Utilities.h>
#include <TranspositionTable.h>
#include <algorithm>
#include <iostream>


void ScoreAllMoves(ScoredMove* moves, uint8_t n_moves){
    ScoredMove m;
    for(int move_index = 0; move_index < n_moves; move_index++){
        moves[move_index].score = ScoreMove(moves[move_index].move);
    }
}

void PickBestMove(ScoredMove* moves, int n_moves, int i){
    int best_index = i; // assume the current move is best
    // Loop over remaining moves: ... i+1, i+2, ... , n_moves
    for(int j = i+1; j < n_moves; j++){
        // if the j-th move scores better than the current move, update best_index
        if(moves[j].score > moves[best_index].score){
            best_index = j;
        }
    }
    // now best_index refers to the move with highest score above i
    std::swap(moves[i], moves[best_index]);
}

/*bool SafeNullMoveSearch(Position& pos){
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
}*/


unsigned long long int Perft(Position& pos, int depth, StateMemory state){
    if(depth == 0){ return 1ULL; }

    uint64_t zobrist_key = ZobristHashing(pos);
    TTEntry* entry = TTProbe(zobrist_key);
    if(entry && entry->depth == depth){ return entry->score; }

    unsigned long long int n_nodes = 0;
    
    // generate legal moves
    Move moves[MAX_NUMBER_OF_MOVES];
    PseudoLegalMoves(pos, moves);
    uint8_t n_moves = pos.n_pseudolegal_moves;

    for(int move_index = 0; move_index < n_moves; move_index++){
        MakeMove(pos, moves[move_index], state);
        if(IsLegal(pos, moves[move_index])){
            n_nodes += Perft(pos, depth - 1, state);
        }
        UnmakeMove(pos, moves[move_index], state);
    }

    TTStore(depth, zobrist_key, n_nodes, EXACT);

    return n_nodes;
}

void PerftTesting(){
    std::string pos1_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    std::string pos2_fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 0";
    std::string pos3_fen = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1";
    std::string pos4_fen = "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1";
    std::string pos5_fen = "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8";
    std::string pos6_fen = "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10";
    Position pos1 = PositionFromFen(pos1_fen);
    Position pos2 = PositionFromFen(pos2_fen);
    Position pos3 = PositionFromFen(pos3_fen);
    Position pos4 = PositionFromFen(pos4_fen);
    Position pos5 = PositionFromFen(pos5_fen);
    Position pos6 = PositionFromFen(pos6_fen);

    int depth = 5;
    StateMemory state;
    std::cout << "Performing Perft test at depth " << depth << ".\n 1 = ok; 0 = not ok. The test can take a few minutes...\n";
    TTInit();
    std::cout << "Testing position 1: "; std::cout << (Perft(pos1, depth, state) == 4865609) << "\n";
    TTInit();
    std::cout << "Testing position 2: "; std::cout << (Perft(pos2, depth, state) == 193690690) << "\n";
    TTInit();
    std::cout << "Testing position 3: "; std::cout << (Perft(pos3, depth, state) == 674624) << "\n";
    TTInit();
    std::cout << "Testing position 4: "; std::cout << (Perft(pos4, depth, state) == 15833292) << "\n";
    TTInit();
    std::cout << "Testing position 5: "; std::cout << (Perft(pos5, depth, state) == 89941194) << "\n";
    TTInit();
    std::cout << "Testing position 6: "; std::cout << (Perft(pos6, depth, state) == 164075551) << "\n";
}


int QuiescenceSearch(Position& pos, int alpha, int beta, int& n_explored_positions){
    int best_evaluation = PositionScore(pos);
    
    // manage 50-moves rule
    if(pos.half_move_counter >= 50){
        n_explored_positions++; 
        return 0;
    }

    if (pos.white_to_move) {
        if (best_evaluation >= beta)
            return beta;  // fail-high cutoff
        if (best_evaluation > alpha)
            alpha = best_evaluation;
    } else {
        if (best_evaluation <= alpha)
            return alpha; // fail-low cutoff
        if (best_evaluation < beta)
            beta = best_evaluation;
    }

    // else generate all the new positions applying all the legal moves 
    // then recursively call this function and update best_evaluation if needed
    int eval;
    Move move;
    uint8_t flags;
    StateMemory state;
    Move moves[MAX_NUMBER_OF_MOVES];
    PseudoLegalMoves(pos, moves);

    // manage stalemate and checkmate: no legal moves in the current position
    //best_evaluation = pos.white_to_move ? negative_infinity : positive_infinity;

    // ---------------------------------------------------------
    // ------ MIN - MAX SEARCH WITH ALPHA - BETA PRUNING -------
    // ---------------------------------------------------------
    int original_alpha = alpha, original_beta = beta;
    // Loop again to recursively iterate the function 
    for(int move_index = 0; move_index < pos.n_pseudolegal_moves; move_index++){
        // pick the best move in the range [move_index + 1, n_moves] and bring it to the current index
        move = moves[move_index];
        flags = move << 12;
        // don't consider the move if it's not an aggressive move (capture or promotion)
        if(flags < 8 && flags != 5 && flags != 4){ continue; }
        // white to move
        if(pos.white_to_move){
            MakeMove(pos, move, state);
            if(IsLegal(pos, move)){
                eval = QuiescenceSearch(pos, alpha, beta, n_explored_positions);
                best_evaluation = std::max(best_evaluation, eval);
            }
            UnmakeMove(pos, move, state);
            if(best_evaluation >= 100000){ break; }
            alpha = std::max(alpha, eval); // best evaluation for white encountered so far down the tree
            if(beta <= alpha){ break; } 
        }
        // black to move
        else{
            MakeMove(pos, move, state);
            if(IsLegal(pos, move)){
                eval = QuiescenceSearch(pos, alpha, beta, n_explored_positions);
                best_evaluation = std::min(best_evaluation, eval);
            }
            UnmakeMove(pos, move, state);
            if(best_evaluation <= -100000){ break; }
            beta = std::min(beta, eval);
            if(beta <= alpha){ break; }
        }
    }

    if (pos.white_to_move) {
        if (best_evaluation >= beta)
            return beta;  // fail-high cutoff
        if (best_evaluation > alpha)
            alpha = best_evaluation;
    } else {
        if (best_evaluation <= alpha)
            return alpha; // fail-low cutoff
        if (best_evaluation < beta)
            beta = best_evaluation;
    }
}


int BestEvaluation(Position& pos, int depth, int alpha, int beta, int& n_explored_positions){
    // ------------------------------------------------------
    // ----- RETRIEVE SCORE FROM TRANSPOSITION TABLE --------
    // ------------------------------------------------------
    // compute Zobrist key for the current position
    //uint64_t zobrist_key = ZobristHashing(pos);
    // check if the move is already present in the transposition table:
    // if yes return a pointer to its memory address; if no return nullptr
    //TTEntry* entry = TTProbe(zobrist_key);
    // if the position is store and it has been analyzed better than what we are about to do here
    // then just return the already found score
    /*if(entry && entry->depth >= depth){
        if (entry->flag == EXACT)
            return entry->score;
        else if (entry->flag == LOWERBOUND && entry->score >= beta)
            return entry->score;
        else if (entry->flag == UPPERBOUND && entry->score <= alpha)
            return entry->score;
    }*/

    // -----------------------------------------------------------------------
    // -- MANAGE EARLY EXIT CASES: DEPTH=0; DRAWS; STALEMATE; CHECKMATE ETC --
    // -----------------------------------------------------------------------
    // count considered positions (total number of nodes)
    // manage 50-moves rule
    if(pos.half_move_counter >= 50){
        n_explored_positions++; 
        return 0;
    }
    // limit case: at anti_depth = 0 just return the material value of the input position
    if(depth == 0){
        n_explored_positions++; 
        return PositionScore(pos);
    }
    // else generate all the new positions applying all the legal moves 
    // then recursively call this function and update best_evaluation if needed
    int eval, best_evaluation;
    Move move;
    StateMemory state;
    Move moves[MAX_NUMBER_OF_MOVES];
    PseudoLegalMoves(pos, moves);
    uint8_t n_pseudolegal_moves = pos.n_pseudolegal_moves;

    ScoredMove scored_moves[MAX_NUMBER_OF_MOVES];
    for(uint8_t move_index = 0; move_index < n_pseudolegal_moves; move_index++){
        scored_moves[move_index].move = moves[move_index];
        scored_moves[move_index].score = ScoreMove(moves[move_index]);
    }

    bool found_legal_move = false;
    // manage stalemate and checkmate: no legal moves in the current position
    best_evaluation = pos.white_to_move ? negative_infinity : positive_infinity;

    // ---------------------------------------------------------
    // ------ MIN - MAX SEARCH WITH ALPHA - BETA PRUNING -------
    // ---------------------------------------------------------
    int original_alpha = alpha, original_beta = beta;
    // Loop again to recursively iterate the function 
    for(int move_index = 0; move_index < n_pseudolegal_moves; move_index++){
        // pick the best move in the range [move_index + 1, n_moves] and bring it to the current index
        //PickBestMove(scored_moves, n_pseudolegal_moves, move_index);
        move = scored_moves[move_index].move;
        // white to move
        if(pos.white_to_move){
            MakeMove(pos, move, state);
            if(IsLegal(pos, move)){
                found_legal_move = true;
                eval = BestEvaluation(pos, depth - 1, alpha, beta, n_explored_positions);
                best_evaluation = std::max(best_evaluation, eval);
                if(best_evaluation >= 100000){ UnmakeMove(pos, move, state); break; }
                alpha = std::max(alpha, eval); // best evaluation for white encountered so far down the tree
                if(beta <= alpha){ UnmakeMove(pos, move, state); break; } 
            }
            UnmakeMove(pos, move, state);
        }
        // black to move
        else{
            MakeMove(pos, move, state);
            if(IsLegal(pos, move)){
                found_legal_move = true;
                eval = BestEvaluation(pos, depth - 1, alpha, beta, n_explored_positions);
                best_evaluation = std::min(best_evaluation, eval);
                if(best_evaluation <= -100000){ UnmakeMove(pos, move, state); break; }
                beta = std::min(beta, eval);
                if(beta <= alpha){ UnmakeMove(pos, move, state); break; }
            }
            UnmakeMove(pos, move, state);
        }
    }

    // NO LEGAL MOVES: CHECKMATE OR STALEMATE 
    if(!found_legal_move){
        unsigned long int king_square;
        // WHITE HAS NO LEGAL MOVES -> is white king attacked?
        if(pos.white_to_move){
            _BitScanForward64(&king_square, pos.pieces[0]);
            pos.white_to_move = false; // make a null move 
            // if it's attacked, CHECKMATE FOR BLACK
            if(SquareIsAttacked(pos, king_square)){
                pos.white_to_move = true;
                return (-100000-depth);
            }
            // if it's not attacked, STALEMATE!
            else{ 
                pos.white_to_move = true;
                return 0;
            }
            pos.white_to_move = true; // unmake the null move
        }
        // BLACK HAS NO LEGAL MOVES -> is black king attacked?
        else {
            _BitScanForward64(&king_square, pos.pieces[6]);
            pos.white_to_move = true; // make a null move 
            // if it's attacked, CHECKMATE FOR WHITE
            if(SquareIsAttacked(pos, king_square)){
                pos.white_to_move = false;
                return (100000 + depth);
            }
            // if it's not attacked, STALEMATE!
            else{
                pos.white_to_move = false;
                return 0; 
            }
            pos.white_to_move = false; // unmake the null move
        }
    }

    // --------------------------------------------------------
    // ------ STORE POSITION IN THE TRANSPOSITION TABLE -------
    // --------------------------------------------------------
    /*NodeFlag flag;
    if (best_evaluation <= original_alpha)
        flag = UPPERBOUND;
    else if (best_evaluation >= original_beta)
        flag = LOWERBOUND;
    else
        flag = EXACT;
    TTStore(depth, zobrist_key, best_evaluation, flag);*/

    return best_evaluation;
}

/*
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
    MoveAndPosition legal_moves[MAX_NUMBER_OF_MOVES];
    LegalMoves(pos, legal_moves);
    uint8_t n_moves = pos.n_legal_moves;
    best_move = legal_moves[0];
    // Loop through the legal moves to assign a heuristic score
    ScoreAllMoves(legal_moves, n_moves);
    // initialize the hash-map for the Transposition table
    // loop over all the legal moves from the current position
    for(int move_index = 0; move_index < n_moves; move_index++){
        // pick move with highest score
        PickBestMove(legal_moves, n_moves, move_index);
        m = legal_moves[move_index];
        //std::cout << "depth: " << depth << " ; move: "; PrintMove(m.move);
        // generate child position and find its best evaluation down the tree 
        eval = BestEvaluation(m.position, depth-1, negative_infinity, positive_infinity, n_explored_positions, true); // depth-1 because we are rooting from the child position
        //std::cout << "eval: " << eval << "\n";
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
    //std::cout << "I have considered " << n_explored_positions << " positions. \n";
    std::cout << "The best move is "; PrintMove(best_move.move);
    return best_move;
}*/


Move IterativeDeepening(Position& pos, int min_depth, int max_depth, int depth_step){
    int eval;
    Move move;
    Move moves[MAX_NUMBER_OF_MOVES];
    int scores[MAX_NUMBER_OF_MOVES];
    StateMemory state;

    // generate pseudolegal moves from current pos.
    PseudoLegalMoves(pos, moves);

    // swap legal moves with illegal moves and cut off the latter: Also, score the legal moves
    int n_pseudo_moves = pos.n_pseudolegal_moves;
    int offset = 0; int n_legal_moves = 0;
    for(int idx = 0; idx < n_pseudo_moves; idx++){
        move = moves[idx];
        MakeMove(pos, move, state);
        if(!IsLegal(pos, move)){
            scores[idx] = 0;
            std::swap(moves[idx], moves[n_pseudo_moves - 1 + offset]);
            offset--; 
        }
        else{
            scores[idx] = ScoreMove(move);
            n_legal_moves++;
        }
        UnmakeMove(pos, move, state);
    }

    // ITERATIVE DEEPENING LOOP
    for(int depth = min_depth; depth <= max_depth; depth++){
        int n_explored_positions = 0;
        StateMemory state;

        std::cout << "\n------------------------------------\nIterative Deepening at depth " << depth << "\n";

        // Loop over legal moves
        for(int idx = 0; idx < n_legal_moves; idx++){

            // pick the best move down the list (from the current index)
            for(int j = idx + 1; j < n_legal_moves; j++){
                if(scores[j] > scores[idx]){
                    std::swap(scores[idx], scores[j]);
                    std::swap(moves[idx], moves[j]);
                }
            }
            move = moves[idx];
            MakeMove(pos, move, state);
            eval = BestEvaluation(pos, depth - 1, negative_infinity, positive_infinity, n_explored_positions);
            std::cout << "move: "; PrintMove(move);
            std::cout << "\t eval: " << eval << "\n"; 
            UnmakeMove(pos, move, state);
            // update move score based on eval at current iteration
            if(pos.white_to_move)
                scores[idx] = eval;
            else
                scores[idx] = -eval;
        }

    }

    return moves[0];
}