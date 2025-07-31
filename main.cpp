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
#include <bitset>

int main(){

    InitializeZobrist();
    TTInit();
    PreComputeBitboards();

    Position pos = PositionFromFen(benchmark_position_fen);
    PrintBoard(pos);

    // start clock 
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    IterativeDeepening(pos, 2, 20, 2);

    // stop clock 
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "Time is: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " [ms] \n";

    CleanBitboards();

    return 0;
}