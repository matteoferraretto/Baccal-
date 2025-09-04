#include <Utilities.h>
#include <random>
#include <cassert>
#include <fstream>
#include <cstring>


size_t pop_count(Bitboard& bitboard){
    std::bitset<64> b(bitboard);
    return b.count();
}

int chebyshev_distance(unsigned long square1, unsigned long square2){
    int vertical = (square1 / 8) - (square2 / 8);
    int horizontal = (square1 % 8) - (square2 % 8);
    if(vertical < 0) vertical = - vertical;
    if(horizontal < 0) horizontal = - horizontal;
    if(horizontal > vertical) return horizontal;
    else return vertical;
}

void PrintBitboard(Bitboard bitboard){
    bool bit = 0;
    // loop through the bits
    for(Square square = 0; square < 64; square++){
        bit = bit_get(bitboard, square); 
        std::cout << bit << " ";
        if(square % 8 == 7){ std::cout << "\n"; }
    }
}

std::string SquareToAlphabet(Square& square){
    int i = square / 8;
    int j = square % 8; 
    std::string ranks = "87654321";
    std::string files = "abcdefgh"; 
    return std::string() + files[j] + ranks[i];
}

/*std::string PieceToAlphabet(uint8_t& piece){
    assert(piece < 12);
    std::string piece_str;
    piece_str = "KQRBNPkqrbnp"[piece];
    return piece_str;
}*/

Bitboard AlphabetToBitboard(std::basic_string<char>& square_string){
    char file = square_string.at(0);
    char rank = square_string.at(1);
    Bitboard bb = 0;
    uint8_t i = 7 - ((uint8_t)(rank) - 49);
    uint8_t j = ((uint8_t)file) - 97;
    Square square = 8*i + j;
    bit_set(bb, square);
    return bb; 
}


std::mt19937_64 rng(20250704); // Fixed seed for reproducibility
uint64_t rand64(){
    return rng();
}; 


void write_to_file(const uint64_t *arr, size_t N, std::string file_name){
    std::ofstream fout(file_name);
    if(!fout){
        std::cout << "Unable to write data on file. \n";
        return;
    }
    for(size_t i = 0; i < N; i++){
        fout << arr[i] << "\n";
    }
    fout.close();
}

void read_from_file(uint64_t *arr, size_t N, std::string file_name){
    std::ifstream fin(file_name);
    if(!fin){
        std::cout << "Unable to read data from file. \n";
        return;
    }
    size_t i = 0; 
    uint64_t x;
    while(fin >> x && i < N){
        arr[i] = x; 
        i++;
    }
    fin.close();
}