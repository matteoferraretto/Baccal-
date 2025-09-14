#pragma once
#include <iostream>
#include <string>
#include <cstdint>
#include <bitset>
#include <vector>
#include <sstream>


// piece type
enum piece_type{
    WHITE_KING,
    WHITE_QUEEN,
    WHITE_ROOK,
    WHITE_BISHOP,
    WHITE_KNIGHT,
    WHITE_PAWN,
    BLACK_KING,
    BLACK_QUEEN,
    BLACK_ROOK,
    BLACK_BISHOP,
    BLACK_KNIGHT,
    BLACK_PAWN,
    NO_PIECE
};

// bitwise operations
typedef uint8_t Square; // 0, ... , 63 <-- 8 bits
typedef uint64_t Bitboard;

inline void bit_set(Bitboard& bitboard, const std::size_t& square){
    bitboard |= (1ULL << square);
}

inline void bit_clear(Bitboard& bitboard, const std::size_t& square){
    bitboard &= ~(1ULL << square);
}

inline bool bit_get(const Bitboard& bitboard, const std::size_t& square){
    return (bitboard >> square) & 1;
}

inline void clear_last_active_bit(Bitboard& bitboard){
    bitboard &= bitboard - 1ULL;
} 

size_t pop_count(Bitboard& bitboard); // count the number of 1 in the binary representation of bitboard

int chebyshev_distance(unsigned long square1, unsigned long square2);


// Utilities 
void PrintBitboard(Bitboard bitboard);
std::string SquareToAlphabet(Square& square);
//std::string PieceToAlphabet(uint8_t& piece);
Bitboard AlphabetToBitboard(std::basic_string<char>& square_string);


// PIECE SQUARE TABLES (PST)
const int WHITE_KNIGHT_PST[64] = {
    270, 280, 290, 290, 290, 290, 280, 270,
    280, 300, 320, 325, 325, 320, 300, 280,
    290, 325, 330, 335, 335, 330, 325, 290,
    290, 320, 335, 340, 340, 335, 320, 290,
    290, 325, 335, 340, 340, 335, 325, 290,
    290, 320, 330, 335, 335, 330, 320, 290,
    280, 300, 320, 320, 320, 320, 300, 280,
    270, 280, 290, 290, 290, 290, 280, 270
};
const int BLACK_KNIGHT_PST[64] = {
    270, 280, 290, 290, 290, 290, 280, 270,
    280, 300, 320, 320, 320, 320, 300, 280,
    290, 320, 330, 335, 335, 330, 320, 290,
    290, 325, 335, 340, 340, 335, 325, 290,
    290, 320, 335, 340, 340, 335, 320, 290,
    290, 325, 330, 335, 335, 330, 325, 290,
    280, 300, 320, 325, 325, 320, 300, 280,
    270, 280, 290, 290, 290, 290, 280, 270
};
const int WHITE_PAWN_PST[64] = {
     0,  0,  0,   0,   0,   0,   0,   0,
    150, 150, 150, 150, 150, 150, 150, 150,
    110, 110, 120, 130, 130, 120, 110, 110,
    105, 105, 110, 125, 125, 110, 105, 105,
    100, 100, 100, 120, 120, 100, 100, 100,
    105,  95,  90, 100, 100,  90,  95, 105,
    105, 110, 110,  80,  80, 110, 110, 105,
      0,   0,   0,   0,   0,   0,   0,   0
};
const int BLACK_PAWN_PST[64] = {
      0,   0,   0,   0,   0,   0,   0,   0,
    105, 110, 110,  80,  80, 110, 110, 105,  
    105,  95,  90, 100, 100,  90,  95, 105,   
    100, 100, 100, 120, 120, 100, 100, 100,
    105, 105, 110, 125, 125, 110, 105, 105,
    110, 110, 120, 130, 130, 120, 110, 110,
    150, 150, 150, 150, 150, 150, 150, 150,
      0,   0,   0,   0,   0,   0,   0,   0
};
const int BISHOP_PST[64] = {
    310, 320, 320, 320, 320, 320, 320, 310,
    320, 335, 330, 330, 330, 330, 335, 320,
    320, 340, 340, 340, 340, 340, 340, 320,
    320, 335, 335, 340, 340, 335, 335, 320,
    320, 335, 335, 340, 340, 335, 335, 320,
    320, 340, 340, 340, 340, 340, 340, 320,
    320, 335, 330, 330, 330, 330, 335, 320,
    310, 320, 320, 320, 320, 320, 320, 310
};
const int WHITE_ROOK_PST[64] = {
    500, 500, 500, 505, 505, 500, 500, 500,
    510, 520, 520, 520, 520, 520, 520, 510,
    495, 500, 500, 500, 500, 500, 500, 495,
    495, 500, 500, 500, 500, 500, 500, 495,
    495, 500, 500, 500, 500, 500, 500, 495,
    495, 500, 500, 500, 500, 500, 500, 495,
    495, 500, 500, 500, 500, 500, 500, 495,
    500, 500, 505, 510, 510, 505, 500, 500
};
const int BLACK_ROOK_PST[64] = {
    500, 500, 505, 510, 510, 505, 500, 500,
    495, 500, 500, 500, 500, 500, 500, 495,
    495, 500, 500, 500, 500, 500, 500, 495,
    495, 500, 500, 500, 500, 500, 500, 495,
    495, 500, 500, 500, 500, 500, 500, 495,
    495, 500, 500, 500, 500, 500, 500, 495,
    510, 520, 520, 520, 520, 520, 520, 510,
    500, 500, 500, 505, 505, 500, 500, 500
};
const int QUEEN_PST[64] = {
    880, 890, 890, 895, 895, 890, 890, 880,
    890, 900, 900, 900, 900, 900, 900, 890,
    890, 900, 905, 905, 905, 905, 900, 890,
    895, 900, 905, 905, 905, 905, 900, 895,
    895, 900, 905, 905, 905, 905, 900, 895,
    890, 900, 905, 905, 905, 905, 900, 890,
    890, 900, 900, 900, 900, 900, 900, 890,
    880, 890, 890, 895, 895, 890, 890, 880
};
const int WHITE_KING_PST_MIDDLEGAME[64] = {
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -20, -30, -30, -40, -40, -30, -30, -20,
    -10, -20, -20, -20, -20, -20, -20, -10,
     20,  20,   0,   0,   0,   0,  20,  20,
     20,  30,  10,   0,   0,  10,  30,  20
};
const int BLACK_KING_PST_MIDDLEGAME[64] = {
    20,  30,  10,   0,   0,  10,  30,  20,
    20,  20,   0,   0,   0,   0,  20,  20,
    -10, -20, -20, -20, -20, -20, -20, -10,
    -20, -30, -30, -40, -40, -30, -30, -20,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30
};
const int KING_PST_ENDGAME[64] = {
    -50,-40,-30,-20,-20,-30,-40,-50,
    -30,-20,-10,  0,  0,-10,-20,-30,
    -30,-10, 20, 30, 30, 20,-10,-30,
    -30,-10, 30, 40, 40, 30,-10,-30,
    -30,-10, 30, 40, 40, 30,-10,-30,
    -30,-10, 20, 30, 30, 20,-10,-30,
    -30,-30,  0,  0,  0,  0,-30,-30,
    -50,-30,-30,-30,-30,-30,-30,-50
};

// malus for every pair of doubled pawns (2 pairs of doubled pawns = - 1 pawn)
const int MALUS_FOR_DOUBLED_PAWNS = 25; 
const int BONUS_FOR_PASSED_PAWNS = 25;
const int BONUS_FOR_OUTPOST = 25;


// relevant constants
const int negative_infinity = std::numeric_limits<int>::min();
const int positive_infinity = std::numeric_limits<int>::max();


// random generator of 64-bit unsigned integers
uint64_t rand64();

// integer power
//int IntPow(int x, unsigned int p);

// write / read bitboard array to / from file
template <size_t N, size_t M>
inline void write_to_file(const uint64_t (&arr)[N][M], std::string file_name){
    std::ofstream fout(file_name);
    if(!fout){
        std::cout << "Unable to write data on file. \n";
        return;
    }
    for(size_t i = 0; i < N; i++){
        for(size_t j = 0; j < M; j++){
            fout << arr[i][j] << "\n";
        }
    }
    fout.close();
}

template <size_t N, size_t M>
inline void read_from_file(uint64_t (&arr)[N][M], std::string file_name){
    std::ifstream fin(file_name);
    if(!fin){
        std::cout << "Unable to read data from file. \n";
        return;
    }
    size_t i = 0, j = 0; 
    uint64_t x;
    while(fin >> x && i < N){
        arr[i][j] = x; 
        j++;
        if(j == M){ j = 0; i++; }
    }
    fin.close();
}

void write_to_file(const uint64_t *arr, size_t N, std::string file_name);
void read_from_file(uint64_t *arr, size_t N, std::string file_name);

// convert PGN file to vector of moves as strings in SAN form
std::vector<std::string> ReadGameFromPGN(std::string filename);

// relevant positions
const std::string starting_position_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 0";
const std::string benchmark_position_fen = "1k1r4/pp1b1R2/3q2pp/4p3/2B5/4Q3/PPP2B2/2K5 b - - 0 0"; // 15 seconds for iterative search to max depth 6; 313 seconds to max depth 8; best move: qd1+
const std::string sebastian_lague_fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q2/PPPBBPpP/R3K2R w KQkq - 0 0"; // Perft 4 : 3 553 501 positions; time 1.18 s for him (insanely fast considering that his code was not implementing bitboards ...)

// pieces values
const int WHITE_ROOK_VALUE = 500;
const int BLACK_ROOK_VALUE = -500;
const int WHITE_BISHOP_VALUE = 330;
const int BLACK_BISHOP_VALUE = -330;
const int WHITE_KNIGHT_VALUE = 320;
const int BLACK_KNIGHT_VALUE = -320;
const int WHITE_QUEEN_VALUE = 900;
const int BLACK_QUEEN_VALUE = -900;
const int MATE_SCORE = 100000;
const int WHITE_PAWN_VALUE = +100;
const int BLACK_PAWN_VALUE = -100;
const int PIECES_VALUES[12] = {
    100000, 900, 500, 330, 320, 100, -100000, -900, -500, -330, -320, -100
};

// geometry of the board
const uint8_t RANK_OF_SQUARE[64] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1,
    2, 2, 2, 2, 2, 2, 2, 2,
    3, 3, 3, 3, 3, 3, 3, 3,
    4, 4, 4, 4, 4, 4, 4, 4,
    5, 5, 5, 5, 5, 5, 5, 5,
    6, 6, 6, 6, 6, 6, 6, 6,
    7, 7, 7, 7, 7, 7, 7, 7
};
const uint8_t FILE_OF_SQUARE[64] = {
    0, 1, 2, 3, 4, 5, 6, 7,
    0, 1, 2, 3, 4, 5, 6, 7,
    0, 1, 2, 3, 4, 5, 6, 7,
    0, 1, 2, 3, 4, 5, 6, 7,
    0, 1, 2, 3, 4, 5, 6, 7,
    0, 1, 2, 3, 4, 5, 6, 7,
    0, 1, 2, 3, 4, 5, 6, 7,
    0, 1, 2, 3, 4, 5, 6, 7
};
// count how many king moves are required to reach the closest corner
const int DISTANCE_FROM_CORNERS[64] = {
    0, 1, 2, 3, 3, 2, 1, 0,
    1, 1, 2, 3, 3, 2, 1, 1,
    2, 2, 2, 3, 3, 2, 2, 2,
    3, 3, 3, 3, 3, 3, 3, 3,    
    3, 3, 3, 3, 3, 3, 3, 3,
    2, 2, 2, 3, 3, 2, 2, 2,
    1, 1, 2, 3, 3, 2, 1, 1,
    0, 1, 2, 3, 3, 2, 1, 0
};
const int DISTANCE_FROM_EDGES[64] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 1, 1, 1, 1, 1, 0,
    0, 1, 2, 2, 2, 2, 1, 0,
    0, 1, 2, 3, 3, 2, 1, 0,    
    0, 1, 2, 3, 3, 2, 1, 0,
    0, 1, 2, 2, 2, 2, 1, 0,
    0, 1, 1, 1, 1, 1, 1, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};
const bool WHITE_PAWN_IN_STARTING_RANK[64] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 
    0, 0, 0, 0, 0, 0, 0, 0
};
const bool WHITE_PAWN_IN_FINAL_RANK[64] = {
    1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};
const bool BLACK_PAWN_IN_STARTING_RANK[64] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};
const bool BLACK_PAWN_IN_FINAL_RANK[64] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1
};

// for each square we store the number of squares where one can put a blocker for bishop
const int n_squares_for_bishop_blockers[64] = {
    6, 5, 5, 5, 5, 5, 5, 6,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    6, 5, 5, 5, 5, 5, 5, 6
};

const Bitboard LACK_QUEENSIDE_CASTLE_MASK = (7ULL << 2);
const Bitboard BLACK_KINGSIDE_CASTLE_MASK = (7ULL << 4);
const Bitboard WHITE_QUEENSIDE_CASTLE_MASK = (7ULL << 58);
const Bitboard WHITE_KINGSIDE_CASTLE_MASK = (7ULL << 60);

// pawn promotions
const char pieces_white_pawn_becomes[4] = {'Q', 'R', 'B', 'N'};
const char pieces_black_pawn_becomes[4] = {'q', 'r', 'b', 'n'};
// pieces deltas
const int knight_deltas[8][2] = {
    {1, 2}, {2, 1}, {2, -1}, {1, -2}, {-1, -2}, {-2, -1}, {-2, 1}, {-1, 2}
};
const int king_deltas[8][2] = {
    {1, 1}, {0, 1}, {-1, 1}, {1, 0}, {-1, 0}, {1, -1}, {0, -1}, {-1, -1}
};
const int white_pawn_deltas[2][2] = {
    {-1, 1}, {-1, -1}
};
const int black_pawn_deltas[2][2] = {
    {1, 1}, {1, -1}
};
