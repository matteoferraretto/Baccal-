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
#include <intrin.h>

#pragma message("Compiling with MSVC")

int main(){

    InitializeZobrist();
    TTInit();
    HistoryInit();
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

    //std::cout << "Poistion score: " << PositionScore(pos);
    //std::cout << "\n\t doubled pawns for white: " << count_doubled_pawns(pos.pieces[5]);
    //std::cout << "\n\t doubled pawns for black: " << count_doubled_pawns(pos.pieces[11]) << "\n";

    Move best_move = IterativeDeepening(pos, 1, max_depth, 1);
    
    /*unsigned long long int perft = 0, total = 0;
    Move pseudolegal_moves[256] = { };
    Move move = 0;
    Position old_pos;

    PseudoLegalMoves(pos, pseudolegal_moves);

    for(int move_index = 0; move_index < 256; move_index++){
        StateMemory state;
        move = pseudolegal_moves[move_index];
        if(move == 0) break;
        PrintMove(move);
        old_pos = pos;
        MakeMove(pos, move, state);
        if(!IsLegal(pos, move)){
            UnmakeMove(pos, move, state); 
            std::cout << "pos check: " << (old_pos == pos) << "\n";
            continue; 
        }
        //std::cout << "move " << move_index + 1 << "\n";
        perft = Perft(pos, max_depth - 1);
        UnmakeMove(pos, move, state);
        std::cout << "pos check: " << (old_pos == pos) << "\n";
        total += perft;
        std::cout << "Perft = " << perft << "\n";
    }

    std::cout << "\nPerft = " << total << "\n"; 
*/
    //PerftTesting();

    //std::cout << "Is check? " << InCheck(pos) << "\n";
    //std::cout << "Only pawns? " << OnlyPawnsRemaining(pos) << "\n";*/

    // stop clock 
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "Elapsed time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " [ms] \n";

    return 0;
}