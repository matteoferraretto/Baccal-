#include <iostream>
#include <bitset>
#include <chrono>
#include <Position.h>
#include <Utilities.h>
#include <Bitboards.h>
#include <Move.h>

int main(){

    PreComputeBitboards();

    Position pos = PositionFromFen("k7/r7/8/8/8/1p6/PPP5/K7 w - - 0 0");
    PrintBoard(pos);
    std::cout << "white material: " << pos.white_material_value << "\n";
    std::cout << "black material: " << pos.black_material_value << "\n";
    std::cout << "position score is: " << Score(pos) << "\n";

    PrintBitboard(pos.white_pieces);
    PrintBitboard(pos.black_pieces);
    PrintBitboard(pos.all_pieces);
    PrintBitboard(pos.white_covered_squares);
    PrintBitboard(pos.black_covered_squares);

    // check sliding
    uint64_t blockers, attacks; 

    // generate random blocker
    blockers = (rand64() & rand64() & rand64());
    PrintBitboard(blockers);

    // retrieve same attack from stored attacks
    int square = 27;
    int n_attacks_rook = IntPow(2, n_bits_rook);
    int n_attacks_bishop = IntPow(2, n_bits_bishop);
    
    uint64_t hash_index_rook = rook_hash_index(blockers, square, n_attacks_rook);
    uint64_t hash_index_bishop = bishop_hash_index(blockers, square, n_attacks_bishop);

    std::cout << "Attacks: \n";
    PrintBitboard(
        rook_covered_squares_bitboards[hash_index_rook]
    );
    PrintBitboard(
        bishop_covered_squares_bitboards[hash_index_bishop]
    );
    PrintBitboard(
        rook_covered_squares_bitboards[hash_index_rook] | bishop_covered_squares_bitboards[hash_index_bishop]
    );
    
    std::vector<Move> all_moves = AllMoves(pos);

    for(Move& move: all_moves){
        PrintMove(move);
    }

    CleanBitboards();

    return 0;
}