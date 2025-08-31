#include <Game.h>
#include <Utilities.h>
#include <Baccala.h>
#include <iostream>
#include <string>

Game::Game(){
    std::string side;
    std::cout << "Choose your color!\n";
    std::getline( std::cin, side );
    // if human player types "b", they play with black and engine is white
    if(side == "b") engine_is_white = true;
    // if they type anything else -> engine is black by default.
    else engine_is_white = false;

    std::cout << "Let the battle begin!\n\n";

    StartNewGame();
}

void Game::ResetHistory(void){
    Position empty_pos;
    for(int idx = 0; idx < MAX_N_MOVES_IN_GAME; idx++){
        moves_list[idx] = 0;
        positions_list[idx] = empty_pos;
    }
    white_wins = false; black_wins = false; is_draw = false;
}

void Game::ResetPseudoLegalMoves(void){
    for(int i = 0; i < MAX_NUMBER_OF_MOVES; i++){
        pseudolegal_moves[i] = 0;
    }
}


Move Game::MoveFromString(std::string move_str){
    if(move_str.size() < 4 || move_str.size() > 5){
        std::cout << "Invalid move. Returning 0\n";
        return 0;
    }

    std::string from_str = move_str.substr(0, 2);
    std::string to_str = move_str.substr(2, 2);
    Bitboard from_bb = AlphabetToBitboard(from_str);
    Bitboard to_bb = AlphabetToBitboard(to_str);

    unsigned long from_square, to_square;
    _BitScanForward64(&from_square, from_bb);
    Move move = static_cast<uint16_t>(from_square);
    _BitScanForward64(&to_square, to_bb);
    move |= static_cast<uint16_t>(to_square << 6);

    StateMemory state;

    // in case of pawn promotion, we build the flag by hand
    if(move_str.size() == 5){
        // avoid non-sense
        if(pos.white_to_move && (RANK_OF_SQUARE[from_square] != 1 || RANK_OF_SQUARE[to_square] != 0))
            return 0;
        if(!pos.white_to_move && (RANK_OF_SQUARE[from_square] != 6 || RANK_OF_SQUARE[to_square] != 7))
            return 0;
        int delta_x = FILE_OF_SQUARE[to_square] - FILE_OF_SQUARE[from_square];
        if(delta_x > 1 || delta_x < -1)
            return 0;

        std::string promo = move_str.substr(4, 1);
        if(promo == "Q"){
            if(delta_x == 0) move |= static_cast<uint16_t>(PROMOTION_QUEEN << 12);
            else move |= static_cast<uint16_t>(PROMOTION_QUEEN_CAPTURE << 12);
        }
        else if(promo == "R"){
            if(delta_x == 0) move |= static_cast<uint16_t>(PROMOTION_ROOK << 12);
            else move |= static_cast<uint16_t>(PROMOTION_ROOK_CAPTURE << 12);
        }
        else if(promo == "B"){
            if(delta_x == 0) move |= static_cast<uint16_t>(PROMOTION_BISHOP << 12);
            else move |= static_cast<uint16_t>(PROMOTION_BISHOP_CAPTURE << 12);
        }
        else if(promo == "N"){
            if(delta_x == 0) move |= static_cast<uint16_t>(PROMOTION_KNIGHT << 12);
            else move |= static_cast<uint16_t>(PROMOTION_KNIGHT_CAPTURE << 12);
        }

        MakeMove(pos, move, state);
        if(IsLegal(pos, move)) return move;
        else UnmakeMove(pos, move, state);
    }

    // for any other move, we just check if "from" and "to" match some pseudolegal move
    uint16_t mask = 0b0000111111111111;

    for(int idx = 0; idx < MAX_NUMBER_OF_MOVES; idx++){
        // if we have finished the pseudolegal moves, break
        if(pseudolegal_moves[idx] == 0) break;
        // if the input move matches one of the precomputed pseudolegal moves
        if((pseudolegal_moves[idx] & mask) == move){
            move = pseudolegal_moves[idx];
            MakeMove(pos, move, state);
            if(IsLegal(pos, move)) return move;
            else UnmakeMove(pos, move, state);
        }
    }

    return 0;
}


void Game::ReadMove(void){
    Move move = 0;
    while(move == 0){
        std::cout << "Type your move \n";
        std::string move_str;
        std::getline( std::cin, move_str );
        move = MoveFromString(move_str);
    }
}


void Game::StartNewGame(void){
    ResetHistory();
    ResetPseudoLegalMoves();
    pos = PositionFromFen(starting_position_fen);
    PrintBoard(pos, !engine_is_white);
    PseudoLegalMoves(pos, pseudolegal_moves);
}


void Game::Play(void){
    Move engine_move;
    StateMemory state;
    bool game_over = false;
    if(!engine_is_white){
        // player's move
        ReadMove();
        PrintBoard(pos, !engine_is_white);
        ResetPseudoLegalMoves();
        PseudoLegalMoves(pos, pseudolegal_moves);
    }
    while(true){
        // engine move
        engine_move = QuietIterativeDeepening(pos, think_time);
        MakeMove(pos, engine_move, state);
        PrintMove(engine_move); std::cout << "\n";
        PrintBoard(pos, !engine_is_white);
        ResetPseudoLegalMoves();
        PseudoLegalMoves(pos, pseudolegal_moves);
        // check game over
        game_over = GameOver();
        if(game_over) break;
        // player's move
        ReadMove();
        PrintBoard(pos, !engine_is_white);
        ResetPseudoLegalMoves();
        PseudoLegalMoves(pos, pseudolegal_moves);
        // check game over
        game_over = GameOver();
        if(game_over) break;
    }
}

bool Game::GameOver(){
    int n_legal_moves = 0;
    StateMemory state;
    Move move;
    // count the legal moves
    for(int i = 0; i < MAX_NUMBER_OF_MOVES; i++){
        move = pseudolegal_moves[i];
        if(move == 0) break;
        MakeMove(pos, move, state);
        if(IsLegal(pos, move)) n_legal_moves++;
        UnmakeMove(pos, move, state);
    }
    // if legal moves, game on
    if(n_legal_moves > 0) return false;
    // checkmate
    if(InCheck(pos)){
        if(pos.white_to_move)
            std::cout << "Checkmate. Black wins.\n";
        else
            std::cout << "Checkmate. White wins.\n";
        return true;
    }
    // otherwise it's stalemate
    else{
        std::cout << "Draw by stalemate.\n";
        return false;
    }
    // ..
    if(InsufficientMaterial(pos)){
        std::cout << "Draw by insufficient material.\n";
        return true;
    }
    // check draw by repetition or insufficient material

}