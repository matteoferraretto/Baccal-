#include <Utilities.h>
#include <random>
#include <cassert>

std::mt19937_64 rng(20250704); // Fixed seed for reproducibility
uint64_t rand64(){
    return rng();
}; 

int IntPow(int x, unsigned int p){
    if (p == 0) return 1;
    if (p == 1) return x; 
    int tmp = IntPow(x, p/2);
    if (p%2 == 0) return tmp * tmp;
    else return x * tmp * tmp;
}

void bit_set(uint64_t& bitboard, int i, int j){
    bitboard |= (1ULL << (8*i+j));
    return;
}
void bit_set(uint64_t& bitboard, unsigned long& square){
    bitboard |= (1ULL << square);
    return;
}
void bit_clear(uint64_t bitboard, int i, int j){
    bitboard &= ~(1ULL << (8*i+j));
    return;
}
void bit_clear(uint64_t& bitboard, unsigned long& square){
    bitboard &= ~(1ULL << square);
    return;
}
bool bit_get(uint64_t bitboard, int i, int j){
    return (bitboard >> (8*i+j)) & 1;
}
bool bit_get(const uint64_t& bitboard, const unsigned long& square){
    return (bitboard >> square) & 1;
}
void clear_last_active_bit(uint64_t& bitboard){
    bitboard &= bitboard - 1;
}

void PrintBitboard(uint64_t bitboard){
    int i, j;
    bool bit;
    std::cout << "\n";
    // loop through the bits
    for(int n=0; n<64; n++){
        i = n/8; j = n%8; // extract row (i) and column (j) indexes
        bit = (bitboard >> n) & 1;  // get the n-th bit
        std::cout << bit << " ";
        if(j == 7){ std::cout << "\n"; }
    }
    return;
}


std::string SquareToAlphabet(uint8_t& square){
    assert(square < 64);
    uint8_t i, j; 
    std::string rank, file;
    i = square/8; j = square%8;
    rank = "87654321"[i];
    file = "abcdefgh"[j]; 
    return file + rank;
}

std::string PieceToAlphabet(uint8_t& piece){
    assert(piece < 12);
    std::string piece_str;
    piece_str = "KQRBNPkqrbnp"[piece];
    return piece_str;
}

uint64_t AlphabetToBitboard(std::basic_string<char>& square_string){
    char file = square_string.at(0);
    char rank = square_string.at(1);
    int i, j; 
    uint64_t bitboard = 0;
    i = 7 - ((int)(rank) - 49);
    j = (int)file - 97;
    bit_set(bitboard, i, j);
    return bitboard; 
}