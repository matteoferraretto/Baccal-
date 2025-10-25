#pragma once
#include <iostream>
#include <bitset>
#include <chrono>
#include <Position.h>
#include <Utilities.h>
#include <Bitboards.h>
#include <Move.h>
#include <Baccala.h>
#include <TranspositionTable.h>
#include <Game.h>
#include <fstream>
#include <intrin.h>
#include <SDL.h>
#include <vector>
#include <string>

#pragma message("Compiling with MSVC")

int main(int argc, char* argv[]){

    InitializeZobrist();
    TTInit();
    HistoryInit();
    PreComputeBitboards(true); // true = read from file

    
    /*std::string pos_fen;
    unsigned int max_depth;
    std::cout << "Insert a valid FEN string \n";
    std::getline( std::cin, pos_fen );
    Position pos = PositionFromFen(pos_fen);
    PrintBoard(pos, pos.white_to_move);
    while(!PositionIsPlayable(pos)){
        std::cout << "Position is illegal. Insert legal position\n";
        std::getline( std::cin, pos_fen );
        pos = PositionFromFen(pos_fen);
        PrintBoard(pos, pos.white_to_move);
    }
    PrintLegalMoves(pos);
    std::cout << "Insert max depth of search \n";
    std::cin >> max_depth;*/

    // start clock 
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    Game game;
    game.PlayVsEngine();
    //game.ShowGame("../assets/PGN_prova.txt");

    //Move best_move = IterativeDeepening(pos, 1, max_depth, 1);
    //PerftTesting();

    /*std::vector<std::string> moves_SAN_history = ReadGameFromPGN("../assets/PGN_prova.txt");
    Position pos = PositionFromFen(starting_position_fen);
    Move pseudolegal_moves[256] = { };
    PseudoLegalMoves(pos, pseudolegal_moves);

    Move moves_history[256] = { };
    Move move = 0;
    std::string move_str;
    int idx_move_in_game = 0;
    
    // LOOP over game history in SAN: 
    //    1. generate pseudolegal moves from pos
    //    2. LOOP over these moves and skip illegal ones
    //    3. translate the others in SAN
    //    4. if one of them matches the current move in SAN, update pos applying this move
    for(int idx_move_in_game = 0; idx_move_in_game < moves_SAN_history.size(); idx_move_in_game++){
        for(int idx = 0; idx < 256; idx++){
            move = pseudolegal_moves[idx];
            if(move == 0) break;
            StateMemory state;
            move_str = AlgebraicNotation(pos, move);

            MakeMove(pos, move, state);
            if(!IsLegal(pos, move)){
                UnmakeMove(pos, move, state);
                continue;
            }
            if(move_str == moves_SAN_history[idx_move_in_game]){
                std::cout << move_str << "\n";
                moves_history[idx_move_in_game] = move;
                break;
            }
            else{
                UnmakeMove(pos, move, state);
            }
        }

        if(move == 0) break;

        for(int idx = 0; idx < 256; idx++){ pseudolegal_moves[idx] = 0; }
        PseudoLegalMoves(pos, pseudolegal_moves);
    }

    for(auto move_str : moves_SAN_history){
        std::cout << move_str << " ";
    }
    std::cout << "\n----------------------------\n";

    for(int idx_move_in_game = 0; idx_move_in_game < moves_SAN_history.size(); idx_move_in_game++){
        move = moves_history[idx_move_in_game];
        PrintMove(move); std::cout << " ";
    }*/

    // stop clock 
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "\nElapsed time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " [ms] \n";

    return 0;
}