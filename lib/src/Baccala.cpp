#pragma once
#include <Baccala.h>
#include <Move.h>
#include <Position.h>
#include <Utilities.h>
#include <TranspositionTable.h>
#include <algorithm>
#include <iostream>
#include <chrono>
#include <intrin.h>
#include <assert.h>

std::chrono::steady_clock::time_point START_TIME;
constexpr int THINK_TIME_MS = 8 * 60 * 1000; // minutes

inline bool time_up(std::chrono::steady_clock::time_point START_TIME) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - START_TIME
    ).count() >= THINK_TIME_MS;
}

uint64_t N_EXPLORED_NODES = 0;

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

unsigned long long int Perft(Position& pos, int depth){
    if(depth == 0){ return 1ULL; }

    //uint64_t zobrist_key = ZobristHashing(pos);
    //TTEntry* entry = TTProbe(zobrist_key);
    //if(entry && entry->depth == depth){ return entry->score; }

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

    //TTStorePerft(depth, zobrist_key, n_nodes);

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


int QuiescenceSearch(Position& pos, int alpha, int beta, int quiesce_ply){
    N_EXPLORED_NODES++;

    if(pos.half_move_counter >= 50){
        return 0;
    }

    int stand_pat = PositionScore(pos);

    if(pos.white_to_move){
        // fails high: black won't allow this -> return beta
        if(stand_pat >= beta){
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
            return alpha;
        }
        // this is a better option -> update beta
        if(stand_pat < beta){ 
            beta = stand_pat; 
        }
    }

    // safety measure: avoid quiescence to go too far in the future 
    if(quiesce_ply > MAX_QUIESCE_DEPTH)
        return pos.white_to_move ? alpha : beta;
    
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
                eval = QuiescenceSearch(pos, alpha, beta, quiesce_ply + 1);
                if(eval >= beta){
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
                eval = QuiescenceSearch(pos, alpha, beta, quiesce_ply + 1);
                if(eval <= alpha){
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

    return pos.white_to_move ? alpha : beta;
}


int BestEvaluation(Position& pos, int depth, int alpha, int beta, bool previous_null){
    // ------------------------------------------------------
    // ----- RETRIEVE SCORE FROM TRANSPOSITION TABLE --------
    // ------------------------------------------------------
    // compute Zobrist key for the current position
    assert(pos.zobrist_key == ZobristHashing(pos));

    // check if the move is already present in the transposition table:
    // if yes return a pointer to its memory address; if no return nullptr
    TTEntry* entry = TTProbe(pos.zobrist_key);
    // if the position is stored and it has been analyzed better than what we are about to do here
    // then just return the already found score
    int tt_best_idx = -1;
    if(entry && entry->depth >= depth){
        if (entry->flag == EXACT){
            return entry->score;
        }
        else if (entry->flag == LOWERBOUND && entry->score >= beta){
            return entry->score;
        }
        else if (entry->flag == UPPERBOUND && entry->score <= alpha){
            return entry->score;
        }
        else{
            tt_best_idx = entry->best_idx;
        }
    }

    N_EXPLORED_NODES++;

    // -----------------------------------------------------------------------
    // -- MANAGE EARLY EXIT CASES: DEPTH=0; DRAWS; STALEMATE; CHECKMATE ETC --
    // -----------------------------------------------------------------------
    // count considered positions (total number of nodes)
    // manage 50-moves rule
    if(pos.half_move_counter >= 50){
        return 0;
    }

    int best_evaluation;
    int null_eval;

    // limit case: at anti_depth = 0 just return the material value of the input position
    if(depth == 0){
        best_evaluation = QuiescenceSearch(pos, alpha, beta, 0);
        TTStore(depth, pos.zobrist_key, best_evaluation, EXACT, -1);
        return best_evaluation;
    }

    // ------------------------------------------------
    // -------------- NULL MOVE PRUNING ---------------
    // ------------------------------------------------
    if(
        !InCheck(pos) && // side to move not in check
        !previous_null && // no response to another null move
        !OnlyPawnsRemaining(pos) && // no risk of zwugzwang
        depth >= 1 + 3 // we only make null move pruning for deep searches, not for shallow ones
    ){
        StateMemory null_state;
        // white skip their move
        if(pos.white_to_move){
            MakeNullMove(pos, null_state);
            null_eval = BestEvaluation(pos, depth - 1 - 3, beta - 1, beta, true);
            // if the null move fails high, we can safely prune
            if(null_eval >= beta){
                TTStore(depth, pos.zobrist_key, beta, LOWERBOUND, 255); // best index is 255 -> it means this move will never be considered! ok because there's no real hash move...
                UnmakeNullMove(pos, null_state);
                return beta;
            }
            UnmakeNullMove(pos, null_state);
        }
        // black skip their move
        else{
            MakeNullMove(pos, null_state);
            null_eval = BestEvaluation(pos, depth - 1 - 3, alpha, alpha + 1, true);
            // if the null move fails high, we can safely prune
            if(null_eval <= alpha){
                TTStore(depth, pos.zobrist_key, alpha, UPPERBOUND, 255);
                UnmakeNullMove(pos, null_state);
                return alpha;
            }
            UnmakeNullMove(pos, null_state);
        }
    }

    // else generate all the new positions applying all the legal moves 
    // then recursively call this function and update best_evaluation if needed
    int eval;
    Move move = 0;
    Move moves[MAX_NUMBER_OF_MOVES] = { };
    PseudoLegalMoves(pos, moves);
    int best_idx;
    int n_pseudolegal_moves = 0;

    // score all the moves
    int scores[MAX_NUMBER_OF_MOVES] = { };
    for(int move_index = 0; move_index < MAX_NUMBER_OF_MOVES; move_index++){
        move = moves[move_index];
        if(move == 0) break;
        scores[move_index] = ScoreMove(pos, move);
        // if TT offered a best move, give it the highest score so it is considered first
        if(move_index == tt_best_idx)
            scores[tt_best_idx] = BONUS_TT_BEST_MOVE;
        // if killer move, score it accordingly
        if(move == killer_moves[depth][0] || move == killer_moves[depth][1])
            scores[move_index] = BONUS_KILLER_MOVE;
        n_pseudolegal_moves++;
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
                found_legal_move = true;
                eval = BestEvaluation(pos, depth - 1, alpha, beta, false);
                // if move is too good: black will not allow this 
                // "fail-high" CUTOFF NODE 
                if(eval >= beta){
                    // if the move is quiet, store it as killer move
                    if( (move >> 12) < 4 ){
                        killer_moves[depth][1] = killer_moves[depth][0];
                        killer_moves[depth][0] = move;
                    }
                    // store it in the TT
                    TTStore(depth, pos.zobrist_key, beta, LOWERBOUND, move_index);
                    UnmakeMove(pos, move, state);
                    return beta;
                }
                // if move is better than all the previously examined, but < beta so black can't stop it,
                // then we have a new best forcing line -> improve alpha
                if(eval > best_evaluation){
                    best_evaluation = eval;
                    best_idx = move_index;
                    if(best_evaluation > alpha){ alpha = best_evaluation; }
                }
                // else, the move was just a bad move, let's pick the next one
                if(best_evaluation >= 100000){ UnmakeMove(pos, move, state); break; }
            }
            UnmakeMove(pos, move, state);
        }
        // black to move
        else{
            MakeMove(pos, move, state);
            if(IsLegal(pos, move)){
                found_legal_move = true;
                eval = BestEvaluation(pos, depth - 1, alpha, beta, false);
                // if move is too good: white will not allow this 
                // "fail-high" CUTOFF NODE
                if(eval <= alpha){
                    // if the move is quiet, store it as killer move
                    if( (move >> 12) < 4 ){
                        killer_moves[depth][1] = killer_moves[depth][0];
                        killer_moves[depth][0] = move;
                    }
                    // store in the TT
                    TTStore(depth, pos.zobrist_key, alpha, UPPERBOUND, move_index);
                    UnmakeMove(pos, move, state);
                    return alpha;
                }
                // if move is better than all the previously examined, but > alpha so white can't stop it,
                // then we have a new best forcing line -> improve beta
                if(eval < best_evaluation){
                    best_evaluation = eval;
                    best_idx = move_index;
                    if(best_evaluation < beta){ beta = best_evaluation; }
                }
                // else, the move was just a bad move, let's pick the next one
                if(best_evaluation <= -100000){ UnmakeMove(pos, move, state); break; }
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
                TTStore(depth, pos.zobrist_key, -100000-depth, EXACT, -1);
                return (-100000-depth);
            }
            // if it's not attacked, STALEMATE!
            else{ 
                pos.white_to_move = true;
                TTStore(depth, pos.zobrist_key, 0, EXACT, -1);
                return 0;
            }
            pos.white_to_move = true; // unmake the null move
        }
        // BLACK HAS NO LEGAL MOVES -> is black king attacked?
        else{
            _BitScanForward64(&king_square, pos.pieces[6]);
            pos.white_to_move = true; // make a null move 
            // if it's attacked, CHECKMATE FOR WHITE
            if(SquareIsAttacked(pos, king_square)){
                pos.white_to_move = false;
                TTStore(depth, pos.zobrist_key, 100000+depth, EXACT, -1);
                return (100000 + depth);
            }
            // if it's not attacked, STALEMATE!
            else{
                pos.white_to_move = false;
                TTStore(depth, pos.zobrist_key, 0, EXACT, -1);
                return 0; 
            }
            pos.white_to_move = false; // unmake the null move
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
    // fow black
    else if (best_evaluation >= original_beta)
        flag = LOWERBOUND;
    // the best evaluation is in the interval [alpha_orig, beta_orig], the search succeeded in finding a better option than the previous lines
    else
        flag = EXACT;
    TTStore(depth, pos.zobrist_key, best_evaluation, flag, best_idx);

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

    // swap legal moves with illegal moves and cut off the latter: Also, score the legal moves
    int offset = 0; int n_legal_moves = 0;
    for(int idx = 0; idx < pos.n_pseudolegal_moves; idx++){
        move = moves[idx];
        StateMemory state;
        MakeMove(pos, move, state);
        if(!IsLegal(pos, move) || move == 0){
            scores[idx] = 0;
            std::swap(moves[idx], moves[pos.n_pseudolegal_moves - 1 + offset]);
            offset--; 
        }
        else{
            scores[idx] = ScoreMove(pos, move);
            n_legal_moves++;
        }
        UnmakeMove(pos, move, state);
    }

    // ITERATIVE DEEPENING LOOP
    for(int depth = min_depth; depth <= max_depth; depth++){

        // Reset stuff
        best_eval_this_depth = pos.white_to_move ? negative_infinity : positive_infinity;
        best_move_this_depth = 0;
        N_EXPLORED_NODES = 0;

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
            if(scores[idx] < - 100000){ std::cout << "\t eval: forced loss\n"; continue; }
            else if(scores[idx] > 100000){ 
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
            eval = BestEvaluation(pos, depth - 1, negative_infinity, positive_infinity, false);
            std::cout << "\t eval: " << eval << "\n"; 
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

        std::cout << "Best move is "; PrintMove(best_move);
        std::cout << "Explored " << N_EXPLORED_NODES << " nodes\n";

        // win detected
        if(win_detected)
            break;

    }

    return best_move;
}