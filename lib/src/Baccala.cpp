#pragma once
#include <Baccala.h>
#include <Move.h>
#include <Position.h>
#include <Utilities.h>
#include <TranspositionTable.h>
#include <algorithm>
#include <iostream>
#include <intrin.h>
#include <assert.h>

std::chrono::steady_clock::time_point START_TIME;
constexpr int THINK_TIME_MS = 8 * 60 * 1000; // minutes

inline bool time_up(std::chrono::steady_clock::time_point START_TIME) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - START_TIME
    ).count() >= THINK_TIME_MS;
}

bool time_up(std::chrono::steady_clock::time_point start_time, int think_time) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start_time
    ).count() >= think_time;
}

uint64_t N_EXPLORED_NODES = 0;
uint64_t N_CUTOFFS = 0;
uint64_t TT_HITS = 0; 
uint64_t TT_ENTRIES = 0;
uint64_t N_TRIED_MOVES = 0;
uint64_t N_FIRST_MOVE_CUTOFFS = 0;
int PLY = 0;
bool LMR_ACTIVE = false;

// Killer moves array
// first dimension: max depth (say 30 for now)
// second dimension: max number of killer moves per depth
Move killer_moves[30][2] = { };
void HistoryInit(){
    for(int depth = 0; depth < 30; depth++){
        killer_moves[depth][0] = 0;
        killer_moves[depth][1] = 0;
    }
}


void PrintPV(Position pos, Move best_move) {
    std::cout << "PV: ";
    StateMemory state;
    TTEntry* entry;

    Move move = best_move;
    while (move) {
        std::cout << AlgebraicNotation(pos, move) << " ";
        MakeMove(pos, move, state);
        //PrintBoard(pos);
        entry = TTProbe(pos.zobrist_key);
        if(!entry || entry->flag != EXACT) break;
        move = entry->best_move;
    }

    std::cout << "\n";
}


unsigned long long int Perft(Position& pos, int depth){
    if(depth == 0){ return 1ULL; }

    TTEntryPerft* entry = TTPerftProbe(pos.zobrist_key);
    if(entry && entry->depth == depth){ return entry->perft; }

    // number of nodes
    unsigned long long int n_nodes = 0;
    Move move = 0;
    
    // generate legal moves
    Move moves[MAX_NUMBER_OF_MOVES] = { };
    PseudoLegalMoves(pos, moves);

    for(int move_index = 0; move_index < MAX_NUMBER_OF_MOVES; move_index++){
        StateMemory state;
        move = moves[move_index];
        if(move == 0) break;
        MakeMove(pos, move, state);
        if(IsLegal(pos, move)){
            n_nodes += Perft(pos, depth - 1);
        }
        UnmakeMove(pos, move, state);
    }

    TTPerftStore(depth, pos.zobrist_key, n_nodes);

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
    std::cout << "Testing position 1: "; std::cout << (Perft(pos1, depth) == 4865609) << "\n";
    std::cout << "Testing position 2: "; std::cout << (Perft(pos2, depth) == 193690690) << "\n";
    std::cout << "Testing position 3: "; std::cout << (Perft(pos3, depth) == 674624) << "\n";
    std::cout << "Testing position 4: "; std::cout << (Perft(pos4, depth) == 15833292) << "\n";
    std::cout << "Testing position 5: "; std::cout << (Perft(pos5, depth) == 89941194) << "\n";
    std::cout << "Testing position 6: "; std::cout << (Perft(pos6, depth) == 164075551) << "\n";
}


bool NullMoveOk(const Position& pos, int depth, bool previous_null) {
    if(previous_null) return false; // can't make a null move after another null move
    if(depth < MIN_DEPTH_NULL) return false; // only active for deep searches
    if(InCheck(pos)) return false; // side to move should not be in check
    if(OnlyPawnsRemaining(pos)) return false; // avoid playing a null in zwugzwang positions
    Bitboard all_pieces = pos.all_pieces;
    if(pop_count(all_pieces) < 4) return false;
    return true;
}


int QuiescenceSearch(Position& pos, int alpha, int beta, int quiesce_ply){
    PLY++;
    N_EXPLORED_NODES++;

    // Draw by 50 half-move rule
    if(pos.half_move_counter >= 50){
        PLY--;
        return 0;
    }
    // Draw by insufficient material
    if(InsufficientMaterial(pos)){
        PLY--;
        return 0;
    }
    // Draw by repetition
    if(ThreeRepetitions(pos, PLY)){
        PLY--;
        return 0;
    }

    int stand_pat = PositionScore(pos);

    if(pos.white_to_move){
        // fails high: black won't allow this -> return beta
        if(stand_pat >= beta){
            PLY--;
            return beta;
        }
        // this is a better option -> update alpha 
        if(stand_pat > alpha){
            alpha = stand_pat;
        }
    }
    else{
        // white won't allow this -> return alpha
        if(stand_pat <= alpha){
            PLY--;
            return alpha;
        }
        // this is a better option -> update beta
        if(stand_pat < beta){ 
            beta = stand_pat; 
        }
    }

    // safety measure: avoid quiescence to go too far in the future 
    if(quiesce_ply > MAX_QUIESCE_DEPTH){
        PLY--;
        return pos.white_to_move ? alpha : beta;
    }
    
    // else generate all the new positions applying all the legal moves 
    // then recursively call this function and update best_evaluation if needed
    int eval, best_evaluation;
    int best_idx;
    uint16_t flags;
    Move move = 0;
    StateMemory state;
    Move moves[MAX_NUMBER_OF_MOVES] = { };
    //PseudoLegalMoves(pos, moves);
    AggressiveMoves(pos, moves);
    int n_pseudolegal_moves = 0;

    // score all the moves
    int scores[MAX_NUMBER_OF_MOVES] = { };
    for(int move_index = 0; move_index < MAX_NUMBER_OF_MOVES; move_index++){
        move = moves[move_index];
        if(move == 0) break;
        flags = move >> 12;
        if(flags < 8 && flags != 4 && flags != 5)
            scores[move_index] = -1; // bad score to quiet moves 
        else
            scores[move_index] = ScoreMove(pos, move);
        n_pseudolegal_moves++;
    }

    // manage stalemate and checkmate: no legal moves in the current position
    best_evaluation = pos.white_to_move ? negative_infinity : positive_infinity;

    // ---------------------------------------------------------
    // ------ MIN - MAX SEARCH WITH ALPHA - BETA PRUNING -------
    // ---------------------------------------------------------
    int original_alpha = alpha, original_beta = beta;
    // Loop again to recursively iterate the function 
    for(int move_index = 0; move_index < n_pseudolegal_moves; move_index++){
        // pick the best move in the range [move_index + 1, n_moves] and bring it to the current index
        best_idx = move_index;
        for(int j = move_index + 1; j < n_pseudolegal_moves; j++){
            if(scores[j] > scores[best_idx]){
                best_idx = j;
            }
        }
        std::swap(scores[move_index], scores[best_idx]);
        std::swap(moves[move_index], moves[best_idx]);
        move = moves[move_index];
        if(move == 0) break;

        // skip quiet moves: only consider aggressive moves
        if(scores[move_index] == -1) continue;

        // initialize cache for move
        StateMemory state;

        // white to move
        if(pos.white_to_move){
            MakeMove(pos, move, state);
            if(IsLegal(pos, move)){
                repetition_stack[PLY + 1] = pos.zobrist_key;
                eval = QuiescenceSearch(pos, alpha, beta, quiesce_ply + 1);
                if(eval >= beta){
                    PLY--;
                    UnmakeMove(pos, move, state);
                    return beta;
                }
                if(eval > best_evaluation){
                    best_evaluation = eval;
                    if(best_evaluation > alpha){ alpha = best_evaluation; }
                }
            }
            UnmakeMove(pos, move, state);
        }
        // black to move
        else{
            MakeMove(pos, move, state);
            if(IsLegal(pos, move)){
                repetition_stack[PLY + 1] = pos.zobrist_key;
                eval = QuiescenceSearch(pos, alpha, beta, quiesce_ply + 1);
                if(eval <= alpha){
                    PLY--;
                    UnmakeMove(pos, move, state);
                    return alpha;
                }
                if(eval < best_evaluation){
                    best_evaluation = eval;
                    if(best_evaluation < beta){ beta = best_evaluation; }
                }
            }
            UnmakeMove(pos, move, state);
        }
    }

    PLY--;
    return pos.white_to_move ? alpha : beta;
}


int BestEvaluation(Position& pos, int depth, int alpha, int beta, bool previous_null){
    // any time the function is called, ply increments 
    PLY++;

    // ------------------------------------------------------
    // ----- RETRIEVE SCORE FROM TRANSPOSITION TABLE --------
    // ------------------------------------------------------
    // check if the move is already present in the transposition table:
    // if yes return a pointer to its memory address; if no return nullptr
    TTEntry* entry = TTProbe(pos.zobrist_key);
    // if the position is stored and it has been analyzed better than what we are about to do here
    // then just return the already found score
    Move tt_best_move = 0;
    if(entry && entry->depth >= depth){
        TT_HITS++;
        if (entry->flag == EXACT){
            PLY--; // any time the function returns, ply decrements
            return entry->score;
        }
        else if (entry->flag == LOWERBOUND && entry->score >= beta){
            PLY--;
            return entry->score;
        }
        else if (entry->flag == UPPERBOUND && entry->score <= alpha){
            PLY--;
            return entry->score;
        }
        else{
            tt_best_move = entry->best_move;
        }
    }

    N_EXPLORED_NODES++;

    // ------------------------------------
    // -- MANAGE EARLY EXIT CASES: DRAWS --
    // ------------------------------------
    int best_evaluation, null_eval;

    // Draw by 50-moves rule
    if(pos.half_move_counter >= 50){
        PLY--;
        return 0;
    }
    // Draw by insufficient material
    if(InsufficientMaterial(pos)){
        PLY--;
        return 0;
    }
    // Draw by repetition
    if(ThreeRepetitions(pos, PLY)){
        PLY--;
        return 0;
    }


    // at leaf nodes launch quiescence search
    if(depth <= 0){
        best_evaluation = QuiescenceSearch(pos, alpha, beta, 0);
        PLY--;
        // cutoff for white
        if(pos.white_to_move){
            // fail high: black will not allow this
            if(best_evaluation >= beta){
                N_CUTOFFS++;
                TTStore(depth, pos.zobrist_key, beta, LOWERBOUND, 0);
                return beta;
            }
            // if evaluation is better than alpha, update alpha
            if(best_evaluation > alpha)
                alpha = best_evaluation;
        } 
        // cutoff for black
        else {
            // fail high: white will not allow this
            if(best_evaluation <= alpha){
                N_CUTOFFS++;
                TTStore(depth, pos.zobrist_key, alpha, UPPERBOUND, 0);
                return alpha;
            }
            // if evaluation is better than beta, update beta
            if(best_evaluation < beta)
                beta = best_evaluation;
        }
        TTStore(depth, pos.zobrist_key, best_evaluation, EXACT, 0);
        return best_evaluation;
    }

    // ------------------------------------------------
    // -------------- NULL MOVE PRUNING ---------------
    // ------------------------------------------------
    if(NullMoveOk(pos, depth, previous_null)){
        StateMemory null_state;
        // white skip their move
        if(pos.white_to_move){
            MakeNullMove(pos, null_state);
            null_eval = BestEvaluation(pos, depth - 1 - DEPTH_REDUCTION_NULL, beta - 1, beta, true);
            // if the null move fails high, we can safely prune
            if(null_eval >= beta){
                TTStore(depth, pos.zobrist_key, beta, LOWERBOUND, 0); // best index is 255 -> it means this move will never be considered! ok because there's no real hash move...
                UnmakeNullMove(pos, null_state);
                PLY--;
                N_CUTOFFS++;
                return beta;
            }
            UnmakeNullMove(pos, null_state);
        }
        // black skip their move
        else{
            MakeNullMove(pos, null_state);
            null_eval = BestEvaluation(pos, depth - 1 - DEPTH_REDUCTION_NULL, alpha, alpha + 1, true);
            // if the null move fails high, we can safely prune
            if(null_eval <= alpha){
                TTStore(depth, pos.zobrist_key, alpha, UPPERBOUND, 0);
                UnmakeNullMove(pos, null_state);
                PLY--;
                N_CUTOFFS++;
                return alpha;
            }
            UnmakeNullMove(pos, null_state);
        }
    }

    // else generate all the new positions applying all the legal moves 
    // then recursively call this function and update best_evaluation if needed
    int eval;
    Move move = 0, best_move = 0;
    Move moves[MAX_NUMBER_OF_MOVES] = { };
    PseudoLegalMoves(pos, moves);
    int best_idx;
    int n_pseudolegal_moves = 0, n_legal_moves = 0;

    // score all the moves
    int scores[MAX_NUMBER_OF_MOVES] = { };
    for(int move_index = 0; move_index < MAX_NUMBER_OF_MOVES; move_index++){
        move = moves[move_index];
        if(move == 0) break;
        scores[move_index] = ScoreMove(pos, move);
        // if TT offered a best move, give it the highest score so it is considered first
        if(move == tt_best_move)
            scores[move_index] = BONUS_TT_BEST_MOVE;
        // if killer move, score it accordingly
        if(move == killer_moves[depth][0] || move == killer_moves[depth][1])
            scores[move_index] = BONUS_KILLER_MOVE;
        n_pseudolegal_moves++;
    }

    // manage stalemate and checkmate: no legal moves in the current position
    best_evaluation = pos.white_to_move ? negative_infinity : positive_infinity;

    // ---------------------------------------------------------
    // ------ MIN - MAX SEARCH WITH ALPHA - BETA PRUNING -------
    // ---------------------------------------------------------
    int original_alpha = alpha, original_beta = beta;
    // Loop again to recursively iterate the function 
    for(int move_index = 0; move_index < n_pseudolegal_moves; move_index++){
        // pick the best move in the range [move_index + 1, n_moves] and bring it to the current index
        best_idx = move_index;
        for(int j = move_index + 1; j < n_pseudolegal_moves; j++){
            if(scores[j] > scores[best_idx]){
                best_idx = j;
            }
        }
        std::swap(scores[move_index], scores[best_idx]);
        std::swap(moves[move_index], moves[best_idx]);
        move = moves[move_index];
        if(move == 0) break;

        StateMemory state;

        // white to move
        if(pos.white_to_move){
            MakeMove(pos, move, state);
            if(IsLegal(pos, move)){
                n_legal_moves++;
                N_TRIED_MOVES++;
                // fill up position stack
                repetition_stack[PLY + 1] = pos.zobrist_key;
                // search at full depth
                if(!LMR_ACTIVE || n_legal_moves <= N_MOVES_FULL_DEPTH)
                    eval = BestEvaluation(pos, depth - 1, alpha, beta, false);
                // LMR: shallow search for late moves
                else{
                    eval = BestEvaluation(pos, depth - 1 - DEPTH_REDUCTION_LMR, alpha, beta, false);
                    // if this is a fail-high node, research at full depth
                    if(eval >= beta) eval = BestEvaluation(pos, depth - 1, alpha, beta, false);
                }
                
                // "fail-high" CUTOFF NODE: if move is too good black will not allow this 
                if(eval >= beta){
                    // if the move is quiet, store it as killer move
                    if( (move >> 12) < 4 ){
                        killer_moves[depth][1] = killer_moves[depth][0];
                        killer_moves[depth][0] = move;
                    }
                    // store it in the TT
                    TTStore(depth, pos.zobrist_key, beta, LOWERBOUND, 0);
                    UnmakeMove(pos, move, state);
                    PLY--;
                    N_CUTOFFS++;
                    return beta;
                }
                // if move is better than all the previously examined, but < beta so black can't stop it,
                // then we have a new best forcing line -> improve alpha
                if(eval > best_evaluation){
                    best_evaluation = eval;
                    best_move = move;
                    if(best_evaluation > alpha){ alpha = best_evaluation; }
                }
            }
            UnmakeMove(pos, move, state);
        }

        // black to move
        else{
            MakeMove(pos, move, state);
            if(IsLegal(pos, move)){
                n_legal_moves++;
                N_TRIED_MOVES++;
                repetition_stack[PLY + 1] = pos.zobrist_key;
                // search at full depth
                if(!LMR_ACTIVE || n_legal_moves <= N_MOVES_FULL_DEPTH)
                    eval = BestEvaluation(pos, depth - 1, alpha, beta, false);
                // LMR: shallow search for late moves
                else{
                    eval = BestEvaluation(pos, depth - 1 - DEPTH_REDUCTION_LMR, alpha, beta, false);
                    // if this is a fail-high node, research at full depth
                    if(eval <= alpha) eval = BestEvaluation(pos, depth - 1, alpha, beta, false);
                }
                // if move is too good: white will not allow this 
                // "fail-high" CUTOFF NODE
                if(eval <= alpha){
                    // if the move is quiet, store it as killer move
                    if( (move >> 12) < 4 ){
                        killer_moves[depth][1] = killer_moves[depth][0];
                        killer_moves[depth][0] = move;
                    }
                    // store in the TT
                    TTStore(depth, pos.zobrist_key, alpha, UPPERBOUND, 0);
                    UnmakeMove(pos, move, state);
                    PLY--;
                    N_CUTOFFS++;
                    return alpha;
                }
                // if move is better than all the previously examined, but > alpha so white can't stop it,
                // then we have a new best forcing line -> improve beta
                if(eval < best_evaluation){
                    best_evaluation = eval;
                    best_move = move;
                    if(best_evaluation < beta){ beta = best_evaluation; }
                }
            }
            UnmakeMove(pos, move, state);
        }
    }

    // NO LEGAL MOVES: CHECKMATE OR STALEMATE 
    if(n_legal_moves == 0){
        /*unsigned long int king_square;
        // WHITE HAS NO LEGAL MOVES -> is white king attacked?
        if(pos.white_to_move){
            _BitScanForward64(&king_square, pos.pieces[0]);
            pos.white_to_move = false; // make a null move 
            // if it's attacked, CHECKMATE FOR BLACK
            if(SquareIsAttacked(pos, king_square)){
                pos.white_to_move = true;
                PLY--;
                return (- MATE_SCORE - depth);
            }
            // if it's not attacked, STALEMATE!
            else{ 
                pos.white_to_move = true;
                PLY--;
                return 0;
            }
            pos.white_to_move = true; // unmake the null move
        }
        // BLACK HAS NO LEGAL MOVES -> is black king attacked?
        else{
            _BitScanForward64(&king_square, pos.pieces[BLACK_KING]);
            pos.white_to_move = true; // make a null move 
            // if it's attacked, CHECKMATE FOR WHITE
            if(SquareIsAttacked(pos, king_square)){
                pos.white_to_move = false;
                PLY--;
                return (MATE_SCORE + depth);
            }
            // if it's not attacked, STALEMATE!
            else{
                pos.white_to_move = false;
                PLY--;
                return 0; 
            }
            pos.white_to_move = false; // unmake the null move
        }*/
        if(InCheck(pos)){
            PLY--;
            return pos.white_to_move ? (- MATE_SCORE - depth) : (MATE_SCORE + depth);
        }
        else {
            PLY--;
            return 0;
        }
    }

    // --------------------------------------------------------
    // ------ STORE POSITION IN THE TRANSPOSITION TABLE -------
    // --------------------------------------------------------
    NodeFlag flag;
    // "fails-low" node: the search was unable to find a better option than other lines explored so far
    // for white
    if (best_evaluation <= original_alpha)
        flag = UPPERBOUND;
    // for black
    else if (best_evaluation >= original_beta)
        flag = LOWERBOUND;
    // the best evaluation is in the interval [alpha_orig, beta_orig], the search succeeded in finding a better option than the previous lines
    else
        flag = EXACT;
    TTStore(depth, pos.zobrist_key, best_evaluation, flag, best_move);

    PLY--;
    return best_evaluation;
}


Move IterativeDeepening(Position& pos, int min_depth, int max_depth, int depth_step){
    // start the clock
    std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();

    int eval, best_eval_this_depth;
    int best_eval = pos.white_to_move ? negative_infinity : positive_infinity;
    Move move = 0;
    Move best_move = 0, best_move_this_depth = 0;
    Move moves[MAX_NUMBER_OF_MOVES] = { };
    int scores[MAX_NUMBER_OF_MOVES] = { };
    bool win_detected = false;

    // generate pseudolegal moves from current pos.
    PseudoLegalMoves(pos, moves);

    // filter out illegal moves
    int n_legal_moves = 0, legal_idx = 0;
    for(int idx = 0; idx < MAX_NUMBER_OF_MOVES; idx++){
        move = moves[idx];
        if(move == 0) break;
        StateMemory state;
        MakeMove(pos, move, state);
        if(IsLegal(pos, move)){
            moves[legal_idx] = move;
            legal_idx++;
            n_legal_moves++;
        }
        UnmakeMove(pos, move, state);
    }
    // score legal moves
    for(int idx = 0; idx < n_legal_moves; idx++){
        scores[idx] = ScoreMove(pos, moves[idx]);
    }


    // ITERATIVE DEEPENING LOOP
    for(int depth = min_depth; depth <= max_depth; depth++){

        // Reset stuff
        best_eval_this_depth = pos.white_to_move ? negative_infinity : positive_infinity;
        best_move_this_depth = 0;
        N_EXPLORED_NODES = 0;
        N_TRIED_MOVES = 0;
        N_CUTOFFS = 0;
        TT_HITS = 0;
        PLY = 0;
        ResetRepetitionStack();
        repetition_stack[0] = pos.zobrist_key;
        if(depth >= MIN_DEPTH_LMR) LMR_ACTIVE = true;

        std::cout << "\n------------------------------------\nIterative Deepening at depth " << depth << "\n";

        // Loop over legal moves
        for(int idx = 0; idx < n_legal_moves; idx++){

            // time out 
            if(time_up(start_time)){
                std::cout << "Time out. Best move is "; PrintMove(best_move);
                return best_move;
            }

            // pick the best move down the list (from the current index)
            for(int j = idx + 1; j < n_legal_moves; j++){
                if(scores[j] > scores[idx]){
                    std::swap(scores[idx], scores[j]);
                    std::swap(moves[idx], moves[j]);
                }
            }
            move = moves[idx];
            std::cout << "move: "; PrintMove(move);

            // if move leads to forced loss, do not consider it!
            if(scores[idx] < - MATE_SCORE){ 
                std::cout << "\t eval: forced loss\n"; continue; 
            }
            else if(scores[idx] > MATE_SCORE){ 
                best_move_this_depth = move;
                win_detected = true;
                std::cout << "\t eval: forced win in " << (depth - scores[idx] + 100000)/2 << " moves\n"; 
                break; 
            }

            // initialize the best move, if not initialized yet
            if(best_move_this_depth == 0) best_move_this_depth = move;

            // make the move and evaluate it
            StateMemory state_1;
            MakeMove(pos, move, state_1);
            N_TRIED_MOVES++;
            repetition_stack[1] = pos.zobrist_key;
            eval = BestEvaluation(pos, depth - 1, negative_infinity, positive_infinity, false);
            UnmakeMove(pos, move, state_1);

            std::cout << "\t eval: " << eval << "\n"; 

            // update best move if possible and the moves' score
            if(pos.white_to_move){
                scores[idx] = eval; // update move score
                if(eval > best_eval_this_depth){
                    best_move_this_depth = move; 
                    best_eval_this_depth = eval;
                }
                if(best_eval_this_depth > best_eval){
                    best_move = best_move_this_depth;
                    best_eval = best_eval_this_depth;
                }
            }
            else{
                scores[idx] = - eval;
                if(eval < best_eval_this_depth){
                    best_move_this_depth = move;
                    best_eval_this_depth = eval;
                }
                if(best_eval_this_depth < best_eval){
                    best_move = best_move_this_depth;
                    best_eval = best_eval_this_depth;
                }
            }

        }

        best_eval = best_eval_this_depth;
        best_move = best_move_this_depth;

        std::cout << "Best move is "; PrintMove(best_move); std::cout << "\n";
        std::cout << "Explored " << N_EXPLORED_NODES << " nodes\n";
        std::cout << "Cutoffs " << N_CUTOFFS << "\n";
        std::cout << "TT Hits " << TT_HITS << "\n";
        std::cout << "TT Entries " << TT_ENTRIES << "\n";
        std::cout << "Moves tried " << N_TRIED_MOVES << "\n";

        // win detected
        if(win_detected)
            break;

    }

    std::cout << "Best move is: "; PrintMove(best_move); std::cout << "\n";
    PrintPV(pos, best_move);

    return best_move;
}


Move QuietIterativeDeepening(Position& pos, int think_time){
    // start the clock
    std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();

    int eval, best_eval_this_depth;
    int best_eval = pos.white_to_move ? negative_infinity : positive_infinity;
    Move move = 0;
    Move best_move = 0, best_move_this_depth = 0;
    Move moves[MAX_NUMBER_OF_MOVES] = { };
    int scores[MAX_NUMBER_OF_MOVES] = { };
    bool win_detected = false;

    // generate pseudolegal moves from current pos.
    PseudoLegalMoves(pos, moves);

    // filter out illegal moves
    int n_legal_moves = 0, legal_idx = 0;
    for(int idx = 0; idx < MAX_NUMBER_OF_MOVES; idx++){
        move = moves[idx];
        if(move == 0) break;
        StateMemory state;
        MakeMove(pos, move, state);
        if(IsLegal(pos, move)){
            moves[legal_idx] = move;
            legal_idx++;
            n_legal_moves++;
        }
        UnmakeMove(pos, move, state);
    }
    // score legal moves
    for(int idx = 0; idx < n_legal_moves; idx++){
        scores[idx] = ScoreMove(pos, moves[idx]);
    }


    // ITERATIVE DEEPENING LOOP
    for(int depth = 1; depth <= 50; depth++){

        // Reset stuff
        best_eval_this_depth = pos.white_to_move ? negative_infinity : positive_infinity;
        best_move_this_depth = 0; 
        PLY = 0;
        ResetRepetitionStack();
        repetition_stack[0] = pos.zobrist_key;
        if(depth >= MIN_DEPTH_LMR) LMR_ACTIVE = true;

        // Loop over legal moves
        for(int idx = 0; idx < n_legal_moves; idx++){

            // time out 
            if(time_up(start_time, think_time)) return best_move;

            // pick the best move down the list (from the current index)
            for(int j = idx + 1; j < n_legal_moves; j++){
                if(scores[j] > scores[idx]){
                    std::swap(scores[idx], scores[j]);
                    std::swap(moves[idx], moves[j]);
                }
            }
            move = moves[idx];

            // if move leads to forced loss, do not consider it!
            //if(scores[idx] < - MATE_SCORE) continue; 
            
            if(scores[idx] > MATE_SCORE){ 
                best_move_this_depth = move;
                win_detected = true;
                break; 
            }

            // initialize the best move, if not initialized yet
            if(best_move_this_depth == 0) best_move_this_depth = move;

            // make the move and evaluate it
            StateMemory state_1;
            MakeMove(pos, move, state_1);
            repetition_stack[1] = pos.zobrist_key;
            eval = BestEvaluation(pos, depth - 1, negative_infinity, positive_infinity, false);
            UnmakeMove(pos, move, state_1);

            // update best move if possible and the moves' score
            if(pos.white_to_move){
                scores[idx] = eval; // update move score
                if(eval > best_eval_this_depth){
                    best_move_this_depth = move; 
                    best_eval_this_depth = eval;
                }
                if(best_eval_this_depth > best_eval){
                    best_move = best_move_this_depth;
                    best_eval = best_eval_this_depth;
                }
            }
            else{
                scores[idx] = - eval;
                if(eval < best_eval_this_depth){
                    best_move_this_depth = move;
                    best_eval_this_depth = eval;
                }
                if(best_eval_this_depth < best_eval){
                    best_move = best_move_this_depth;
                    best_eval = best_eval_this_depth;
                }
            }

        }

        best_eval = best_eval_this_depth;
        best_move = best_move_this_depth;

        // win detected
        if(win_detected) break;

    }

    return best_move;
}