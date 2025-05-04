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
    
    std::string random_position_fen = "4k2r/4r3/8/8/8/8/4R3/R3K3 w Qk - 0 1";
    Position pos = Position(random_position_fen);
    pos.PrintBoard();
    std::cout << "\n";
    PrintMask(pos.black_covered_squares_mask);

    std::vector<Move> legal_moves = pos.LegalMoves();
    for(Move& move: legal_moves){
        move.PrintMove();
        std::cout << "is this move legal? " << pos.IsLegal(move) << "\n";
    }
    std::cout << pos.material_value << "\n";


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

    return 0;
}