#include <iostream>
#include <bitset>
#include <chrono>
#include <Position.h>
#include <Utilities.h>
#include <Bitboards.h>
#include <Move.h>
#include <Baccala.h>

int main(){

    PreComputeBitboards();

    Position pos = PositionFromFen("k6n/b7/8/8/8/8/8/RK4Q1 w - - 0 0");
    PrintBoard(pos);
    std::cout << "white material: " << pos.white_material_value << "\n";
    std::cout << "black material: " << pos.black_material_value << "\n";
    std::cout << "position score is: " << Score(pos) << "\n";

    PrintBitboard(pos.white_pieces);
    PrintBitboard(pos.black_pieces);
    PrintBitboard(pos.all_pieces);
    PrintBitboard(pos.white_covered_squares);
    PrintBitboard(pos.black_covered_squares);
    
    std::vector<MoveAndPosition> all_moves = LegalMoves(pos);
    for(MoveAndPosition& m: all_moves){
        PrintMove(m.move);
        PrintBoard(m.position);
        std::cout << ScoreMove(m.move) << "\n\n";
    }
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
    BestMove(pos, 2);

    CleanBitboards();

    return 0;
}