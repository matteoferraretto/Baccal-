// compile and run with the following commands:
//      cd C:\Users\matte\Documents\Study_C++\ChessEngine
//      mkdir build 
//      cd build
//      cmake --build .
//      .\Debug\ChessEngine.exe

// link libraries
#include <SDL.h>
#include <SDL_render.h>
#include <SDL_ttf.h>
#include <iostream>
#include <cmath>
#include <sstream>
#include <string>
#include "Baccala.h"


int main(int argc, char* argv[]){
    
    std::string random_position_fen = "k7/8/2p5/pP6/8/8/8/Kb6 w KQkq a6 3 50";
    Position pos = Position(random_position_fen);
    pos.PrintBoard();
    std::cout << pos.half_move_counter << " " << pos.move_counter << "\n";
    std::cout << "\n";

    std::vector<Move> legal_moves = pos.LegalMoves();
    for(Move& move: legal_moves){
        std::cout << move.AlgebraicNotation() << "\n";
    }

    /*
    Square current_square; current_square.i = 6; current_square.j = 1;
    PrintMask(
        QueenMovesMask(current_square, empty_mask, empty_mask)
    );
    PrintMoves(
        BlackPawnTargetSquares(current_square, empty_mask, empty_mask)
    );
    //std::vector<Square> moves = KnightTargetSquares(current_square, empty_mask, empty_mask);
    //PrintMoves(moves);
    */

    std::cout << (int)('a') << std::endl;
    std::cout << ((int)('1') - 48) << std::endl;
    
    return 0;
}