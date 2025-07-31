#pragma once
#include <iostream>
#include <bitset>
#include <chrono>
#include <Position.h>
#include <Utilities.h>
#include <Bitboards.h>
#include <Move.h>
#include <Baccala.h>
#include <chrono>
#include <TranspositionTable.h>

int main(){

    InitializeZobrist();
    TTInit();
    PreComputeBitboards();

    Position pos = PositionFromFen("7k/P6p/8/8/8/8/K7/8 w - - 0 0");
    PrintBoard(pos);
    
/*
    // score moves
    ScoreAllMoves(all_moves);
    std::size_t n_moves = all_moves.size();
    // sort moves (slow for debugging)
    std::cout << "Sorting moves: \n";
    for(int move_index = 0; move_index < n_moves; move_index++){
        PickBestMove(all_moves, n_moves, move_index);
        PrintMove(all_moves[move_index].move);
        std::cout << all_moves[move_index].score;
    }*/

    // start clock 
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    IterativeDeepening(pos, 2, 20, 2);

    // stop clock 
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "Time is: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " [ms] \n";

    CleanBitboards();

    return 0;
}