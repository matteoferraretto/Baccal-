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
#include <fstream>


int main(){

    InitializeZobrist();
    TTInit();
    PreComputeBitboards(true); // true = read from file

    std::string pos_fen;
    unsigned int max_depth;
    std::cout << "Insert a valid FEN string \n";
    std::getline( std::cin, pos_fen );
    Position pos = PositionFromFen(pos_fen);
    PrintBoard(pos);
    std::cout << "Insert max depth of search \n";
    std::cin >> max_depth;

    // start clock 
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    Move best_move = IterativeDeepening(pos, 1, max_depth, 1);
    
/*    int perft = 0, total = 0;

    for(int move_index = 0; move_index < 256; move_index++){
        move = pseudolegal_moves[move_index];
        if(move == 0){ break; }
        MakeMove(pos, move, state);
        if(!IsLegal(pos, move)){
            UnmakeMove(pos, move, state); 
            continue; 
        }
        PrintMoveNew(move);
        perft = PerftNew(pos, max_depth - 1, state);
        total += perft;
        std::cout << "Perft = " << perft << "\n";
        UnmakeMove(pos, move, state);
    }

    std::cout << "\nPerft = " << total << "\n"; */

    //PerftTesting();

    // stop clock 
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "Time is: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " [ms] \n";

    CleanBitboards();

    return 0;
}