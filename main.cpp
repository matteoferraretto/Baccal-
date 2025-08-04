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
#include <fstream>

int main(){

    InitializeZobrist();
    TTInit();
    PreComputeBitboards(true); // true = read from file
/*
    std::string pos_fen;
    unsigned int max_depth;
    std::cout << "Insert a valid FEN string \n";
    std::getline( std::cin, pos_fen );
    Position pos = PositionFromFen(pos_fen);
    PrintBoard(pos);
    std::cout << "Insert max depth of search \n";
    std::cin >> max_depth;
*/

    // start clock 
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

//    IterativeDeepening(pos, 2, max_depth, 2);
    PerftTesting();
    
/*    std::cout << "Perft = " << Perft(pos, max_depth) << "\n";

    MoveAndPosition legal_moves[256];
    LegalMoves(pos, legal_moves);
    size_t n_moves = pos.n_legal_moves;
    for(int i=0; i<n_moves; i++){
        MoveAndPosition m = legal_moves[i];
        std::cout << "\t "; PrintMove(m.move);
        std::cout << "\t " << Perft(m.position, max_depth-1) << "\n";
        MoveAndPosition new_legal_moves[256];
        LegalMoves(m.position, new_legal_moves);

        for(int j=0; j<m.position.n_legal_moves; j++){
            std::cout << "\t\t";
            PrintMove(new_legal_moves[j].move);
        }
    } */

    // stop clock 
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "Time is: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " [ms] \n";

    CleanBitboards();

    return 0;
}