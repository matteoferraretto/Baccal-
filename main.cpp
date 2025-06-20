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
    
    std::string random_position_fen = "4k3/4Q3/8/8/8/8/8/4R2K w - - 3 50";
    Position pos = Position(random_position_fen);
    pos.PrintBoard();
    std::cout << pos.material_value << "\n";

    std::vector<Move> legal_moves = pos.LegalMoves();
    std::vector<Position> new_positions;
    new_positions.reserve(legal_moves.size());
    Position new_pos = pos;
    for(Move& move: legal_moves){
        std::cout << move.AlgebraicNotation() << "\n";
        new_pos = NewPosition(pos, move);
        new_pos.PrintBoard();
        std::cout << new_pos.material_value << "\n";
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
    
    return 0;
}