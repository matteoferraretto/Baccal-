#include <Bitboards.h>
#include <Utilities.h>
#include <fstream>

uint64_t knight_covered_squares_bitboards[64]; 
uint64_t king_covered_squares_bitboards[64];
uint64_t white_pawn_covered_squares_bitboards[64];
uint64_t black_pawn_covered_squares_bitboards[64];
uint64_t white_pawn_advance_squares_bitboards[64];
uint64_t black_pawn_advance_squares_bitboards[64];

uint64_t rook_masks[64];
uint64_t rook_magics[64];
uint64_t *rook_covered_squares_bitboards = nullptr;

uint64_t bishop_masks[64];
uint64_t bishop_magics[64];
uint64_t *bishop_covered_squares_bitboards = nullptr;

uint64_t mask_white_passed_pawn[64];
uint64_t mask_black_passed_pawn[64];


uint64_t knight_covered_squares(int i, int j){
    uint64_t knight_bitboard = 0;
    int i_target, j_target;
    for(int d = 0; d < 8; d++){
        i_target = i + knight_deltas[d][0]; 
        if(i_target < 0 || i_target > 7){ continue; }
        j_target = j + knight_deltas[d][1];
        if(j_target < 0 || j_target > 7){ continue; }
        bit_set(knight_bitboard, i_target, j_target);
    }
    return knight_bitboard;
}

uint64_t king_covered_squares(int i, int j){
    uint64_t king_bitboard = 0;
    int i_target, j_target;
    for(int d = 0; d < 8; d++){
        i_target = i + king_deltas[d][0]; 
        if(i_target < 0 || i_target > 7){ continue; }
        j_target = j + king_deltas[d][1];
        if(j_target < 0 || j_target > 7){ continue; }
        bit_set(king_bitboard, i_target, j_target);
    }
    return king_bitboard;
}

uint64_t white_pawn_covered_squares(int i, int j){
    uint64_t white_pawn_bitboard = 0;
    int i_target, j_target;
    for(int d = 0; d < 2; d++){
        i_target = i + white_pawn_deltas[d][0]; 
        if(i_target < 0 || i_target > 7){ continue; }
        j_target = j + white_pawn_deltas[d][1];
        if(j_target < 0 || j_target > 7){ continue; }
        bit_set(white_pawn_bitboard, i_target, j_target);
    }
    return white_pawn_bitboard;
}

uint64_t black_pawn_covered_squares(int i, int j){
    uint64_t black_pawn_bitboard = 0;
    int i_target, j_target;
    for(int d = 0; d < 2; d++){
        i_target = i + black_pawn_deltas[d][0]; 
        if(i_target < 0 || i_target > 7){ continue; }
        j_target = j + black_pawn_deltas[d][1];
        if(j_target < 0 || j_target > 7){ continue; }
        bit_set(black_pawn_bitboard, i_target, j_target);
    }
    return black_pawn_bitboard;
}

uint64_t white_pawn_advance_squares(int i, int j){
    uint64_t white_pawn_bitboard = 0;
    int i_target, j_target;
    if(i != 0){
        i_target = i - 1; j_target = j;
        bit_set(white_pawn_bitboard, i_target, j_target);
        if(i == 6){
            i_target = i - 2; j_target = j;
            bit_set(white_pawn_bitboard, i_target, j_target);
        }
    }
    return white_pawn_bitboard;
}

uint64_t black_pawn_advance_squares(int i, int j){
    uint64_t black_pawn_bitboard = 0;
    int i_target, j_target;
    if(i != 7){
        i_target = i + 1; j_target = j;
        bit_set(black_pawn_bitboard, i_target, j_target);
        if(i == 1){
            i_target = i + 2; j_target = j;
            bit_set(black_pawn_bitboard, i_target, j_target);
        }
    }
    return black_pawn_bitboard;
}

uint64_t rook_relevant_blockers_mask(int i, int j){
    uint64_t bitboard = 0ULL;
    for(int j_target = 1; j_target < 7; j_target++){
        if(j_target == j){ continue; }
        bit_set(bitboard, i, j_target);
    }
    for(int i_target = 1; i_target < 7; i_target++){
        if(i_target == i){ continue; }
        bit_set(bitboard, i_target, j);
    }
    return bitboard;
}

uint64_t bishop_relevant_blockers_mask(int i, int j){
    uint64_t bitboard = 0ULL;
    int i_target, j_target;
    for(int d = -6; d < 7; d++){
        i_target = i + d;
        j_target = j + d;
        if(i_target < 1 || i_target > 6 || j_target < 1 || j_target > 6 ){ continue; }
        else if(j_target == j || i_target == i) { continue; }
        bit_set(bitboard, i_target, j_target);
    }
    for(int d = -6; d < 7; d++){
        i_target = i + d;
        j_target = j - d;
        if(i_target < 1 || i_target > 6 || j_target < 1 || j_target > 6 ){ continue; }
        else if(j_target == j || i_target == i) { continue; }
        bit_set(bitboard, i_target, j_target);
    } 
    return bitboard;
}


// ----------- SLIDING PIECES ---------------
uint64_t rook_covered_squares_from_blockers(uint64_t blockers, int i, int j){
    uint64_t bitboard = 0ULL;
    // bottom sliding
    for(int i_target = i+1; i_target < 8; i_target++){
        bit_set(bitboard, i_target, j);
        if(bit_get(blockers, i_target, j)){ break; }
    }
    // top sliding
    for(int i_target = i-1; i_target >= 0; i_target--){
        bit_set(bitboard, i_target, j);
        if(bit_get(blockers, i_target, j)){ break; }
    }
    // right sliding
    for(int j_target = j+1; j_target < 8; j_target++){
        bit_set(bitboard, i, j_target);
        if(bit_get(blockers, i, j_target)){ break; }
    }
    // left sliding
    for(int j_target = j-1; j_target >= 0; j_target--){
        bit_set(bitboard, i, j_target);
        if(bit_get(blockers, i, j_target)){ break; }
    }
    return bitboard;
}

uint64_t bishop_covered_squares_from_blockers(uint64_t blockers, int i, int j){
    uint64_t bitboard = 0ULL;
    // bottom-right sliding
    for(int i_target = i+1, j_target = j+1; i_target < 8 && j_target < 8; i_target++, j_target++){
        bit_set(bitboard, i_target, j_target);
        if(bit_get(blockers, i_target, j_target)){ break; }
    }
    // top-right sliding
    for(int i_target = i-1, j_target = j+1; i_target >= 0 && j_target < 8; i_target--, j_target++){
        bit_set(bitboard, i_target, j_target);
        if(bit_get(blockers, i_target, j_target)){ break; }
    }
    // bottom-left sliding
    for(int i_target = i+1, j_target = j-1; i_target < 8 && j_target >= 0; i_target++, j_target--){
        bit_set(bitboard, i_target, j_target);
        if(bit_get(blockers, i_target, j_target)){ break; }
    }
    // top-left sliding
    for(int i_target = i-1, j_target = j-1; i_target >= 0 && j_target >=0; i_target--, j_target--){
        bit_set(bitboard, i_target, j_target);
        if(bit_get(blockers, i_target, j_target)){ break; }
    }
    return bitboard;
}

uint64_t rook_blockers_from_integer(uint64_t b, int i, int j){
    uint64_t blockers = 0ULL;
    // manage errors
    if(b >= 4096){
        std::cout << "Error. Trying to generate blockers from integer which is too large. Returning 0.\n";
        return blockers;
    }
    // convert b into an array of relevant bits
    bool bits[12]; 
    for(int index = 0; index < 12; index++){
        bits[index] = bit_get(b, 0, index); // <--- trick to extract the first bits of b
    }
    // now rearrange the bits in the j-th column and i-th row, excluding the square (i,j)
    int index = 0;
    for(int i_target = 1; i_target < 7; i_target++){
        if(i_target == i){ continue; }
        if(bits[index]){ 
            bit_set(blockers, i_target, j);
        }
        index++;
    }
    for(int j_target = 1; j_target < 7; j_target++){
        if(j_target == j){ continue; }
        if(bits[index]){ 
            bit_set(blockers, i, j_target);
        }
        index++;
    }
    return blockers;
}

uint64_t bishop_blockers_from_integer(uint64_t b, int i, int j){
    uint64_t blockers = 0ULL;
    int b_max = IntPow(2, n_squares_for_bishop_blockers[8*i+j]);
    // manage errors
    if(b >= b_max){
        std::cout << "Error. Trying to generate blockers from integer which is too large. Returning 0.\n";
        return blockers;
    }
    // convert b into an array of relevant bits
    bool *bits = new bool [b_max]; 
    for(int index = 0; index < b_max; index++){
        bits[index] = bit_get(b, 0, index); // <--- trick to extract the first bits of b
    }
    // now rearrange the bits in the diagonal of the square (i, j), excluding the edge squares
    int index = 0;
    // bottom-right sliding
    for(int i_target = i+1, j_target = j+1; i_target < 7 && j_target < 7; i_target++, j_target++){
        if(bits[index]){ 
            bit_set(blockers, i_target, j_target);
        }
        index++;
    }
    // top-right sliding
    for(int i_target = i-1, j_target = j+1; i_target > 0 && j_target < 7; i_target--, j_target++){
        if(bits[index]){ 
            bit_set(blockers, i_target, j_target);
        }
        index++;
    }
    // bottom-left sliding
    for(int i_target = i+1, j_target = j-1; i_target < 7 && j_target > 0; i_target++, j_target--){
        if(bits[index]){ 
            bit_set(blockers, i_target, j_target);
        }
        index++;
    }
    // top-left sliding
    for(int i_target = i-1, j_target = j-1; i_target > 0 && j_target > 0; i_target--, j_target--){
        if(bits[index]){ 
            bit_set(blockers, i_target, j_target);
        }
        index++;
    }
    delete [] bits;
    return blockers;
}

void find_rook_magic(unsigned int n_bits, uint64_t *attacks, uint64_t magics[64]){
    int shift = 64 - n_bits;
    int n_attacks = IntPow(2, n_bits);
    uint64_t magic, blockers, attack, stored_attack, hash_index;
    int i, j;
    bool success;
    // loop over the whole board
    for(int square = 0; square < 64; square++){
        i = square/8; j = square % 8;
        std::cout << "Searching magics for square: (" << i << ", " << j << ")\t";
        // loop until you find the good magic number
        success = false;
        while(!success){
            // create random magic number (with many zero bits)
            magic = rand64() & rand64();
            // reset the attacks array with 111111... numbers
            for(int index = 0; index < n_attacks; index++){
                attacks[square * n_attacks + index] = -1;
            }
            // loop over all the blockers
            for(int b = 0; b < 4096; b++){
                // generate bitboard of blockers corresponding to a given index
                blockers = rook_blockers_from_integer(b, i, j);
                // generate attack bitboard given the blockers configuration
                attack = rook_covered_squares_from_blockers(blockers, i, j);
                // generate hash index
                hash_index = (blockers * magic) >> shift;
                if(hash_index >= n_attacks){ std::cout << "Error!\n"; }
                // check what is stored in the attacks array in correspodence to the hash index:
                stored_attack = attacks[square * n_attacks + hash_index];
                // if attack is stored, check for conflict and in case of conflict, change magic number
                if(stored_attack != -1){
                    if(stored_attack != attack){
                        break; // by breaking the loop, the magic number is changed
                    }
                }
                else{
                    attacks[square * n_attacks + hash_index] = attack;
                }
                // if no conflict occured at the end of the inner loop, the search for magics was successful
                if(b == 4095){
                    success = true;
                }
            }
        }
        std::cout << " Found the number " << magic << "\n";
        magics[square] = magic;
    }
}

void find_bishop_magic(unsigned int n_bits, uint64_t *attacks, uint64_t magics[64]){
    int shift = 64 - n_bits;
    int n_attacks = IntPow(2, n_bits);
    uint64_t magic, blockers, attack, stored_attack, hash_index;
    int i, j;
    int b_max;
    bool success;
    // loop over the whole board
    for(int square = 0; square < 64; square++){
        i = square/8; j = square % 8;
        b_max = IntPow(2, n_squares_for_bishop_blockers[square]);
        std::cout << "Searching magics for square: (" << i << ", " << j << ")\t";
        // loop until you find the good magic number
        success = false;
        while(!success){
            // create random magic number (with many zero bits)
            magic = rand64() & rand64();
            // reset the attacks array with 111111... numbers
            for(int index = 0; index < n_attacks; index++){
                attacks[square * n_attacks + index] = -1;
            }
            // loop over all the blockers
            for(int b = 0; b < b_max; b++){
                // generate bitboard of blockers corresponding to a given index
                blockers = bishop_blockers_from_integer(b, i, j);
                // generate attack bitboard given the blockers configuration
                attack = bishop_covered_squares_from_blockers(blockers, i, j);
                // generate hash index
                hash_index = (blockers * magic) >> shift;
                if(hash_index >= n_attacks){ std::cout << "Error!\n"; }
                // check what is stored in the attacks array in correspodence to the hash index:
                stored_attack = attacks[square * n_attacks + hash_index];
                // if attack is stored, check for conflict and in case of conflict, change magic number
                if(stored_attack != -1){
                    if(stored_attack != attack){
                        break; // by breaking the loop, the magic number is changed
                    }
                }
                else{
                    attacks[square * n_attacks + hash_index] = attack;
                }
                // if no conflict occured at the end of the inner loop, the search for magics was successful
                if(b == b_max-1){
                    success = true;
                }
            }
        }
        std::cout << " Found the number " << magic << "\n";
        magics[square] = magic;
    }
}

uint64_t rook_hash_index(uint64_t blockers, int square, int n_attacks){
    uint64_t mask = rook_masks[square]; 
    uint64_t magic = rook_magics[square];
    uint64_t hash_index = ((blockers & mask) * magic) >> shift_rook;
    return (square * n_attacks + hash_index);
}

uint64_t bishop_hash_index(uint64_t blockers, int square, int n_attacks){
    uint64_t mask = bishop_masks[square]; 
    uint64_t magic = bishop_magics[square];
    uint64_t hash_index = ((blockers & mask) * magic) >> shift_bishop;
    return (square * n_attacks + hash_index);
}


void PreComputeBitboards(bool retrieve_from_file){
    uint64_t bitboard;
    int i, j;
    // dynamic allocate attack arrays
    int n_rook_attacks = IntPow(2, n_bits_rook);
    int n_bishop_attacks = IntPow(2, n_bits_bishop);
    rook_covered_squares_bitboards = new uint64_t[64 * n_rook_attacks];
    bishop_covered_squares_bitboards = new uint64_t[64 * n_bishop_attacks];
    // initialize covered squares for non sliding pieces:
    for(int square = 0; square < 64; square++){
        i = square / 8; j = square % 8;
        // knight bitboards
        bitboard = knight_covered_squares(i,j);
        knight_covered_squares_bitboards[square] = bitboard;
        // king bitboards
        bitboard = king_covered_squares(i,j);
        king_covered_squares_bitboards[square] = bitboard;
        // pawn bitboards
        bitboard = white_pawn_covered_squares(i,j);
        white_pawn_covered_squares_bitboards[square] = bitboard;
        bitboard = white_pawn_advance_squares(i,j);
        white_pawn_advance_squares_bitboards[square] = bitboard;
        bitboard = black_pawn_covered_squares(i,j);
        black_pawn_covered_squares_bitboards[square] = bitboard;
        bitboard = black_pawn_advance_squares(i,j);
        black_pawn_advance_squares_bitboards[square] = bitboard;
        // masks of relevant bitboards
        rook_masks[square] = rook_relevant_blockers_mask(i, j);
        bishop_masks[square] = bishop_relevant_blockers_mask(i, j);
    }
    // initialize covered squares for sliding pieces and save them
    if(!retrieve_from_file){
        find_rook_magic(n_bits_rook, rook_covered_squares_bitboards, rook_magics);
        find_bishop_magic(n_bits_bishop, bishop_covered_squares_bitboards, bishop_magics);
        // save data
        write_to_file(rook_magics, 64, "../assets/rook_magics.txt");
        write_to_file(rook_covered_squares_bitboards, 64*n_rook_attacks, "../assets/rook_attacks.txt");
        write_to_file(bishop_magics, 64, "../assets/bishop_magics.txt");
        write_to_file(bishop_covered_squares_bitboards, 64*n_bishop_attacks, "../assets/bishop_attacks.txt");
    }
    // or load from a file
    else{
        read_from_file(rook_magics, 64, "../assets/rook_magics.txt");
        read_from_file(rook_covered_squares_bitboards, 64*n_rook_attacks, "../assets/rook_attacks.txt");
        read_from_file(bishop_magics, 64, "../assets/bishop_magics.txt");
        read_from_file(bishop_covered_squares_bitboards, 64*n_bishop_attacks, "../assets/bishop_attacks.txt");
    }
    // initialize masks for passed pawn and outpost detection
    get_passed_pawn_masks();
}


void CleanBitboards(){
    delete [] rook_covered_squares_bitboards;
    delete [] bishop_covered_squares_bitboards;
}

// generate the bitboard of covered squares by a given side (white or black)
uint64_t GetCoveredSquares(uint64_t pieces[12], uint64_t& all_pieces, bool by_white){
    uint64_t piece;
    uint64_t attacks = 0;
    unsigned long square;
    uint64_t hash_index_rook, hash_index_bishop;

    // white to move:
    if(by_white){

        // KING
        piece = pieces[0];
        // loop over all the pieces of the same type
        while(piece){
            _BitScanForward64(&square, piece); // find position of piece and assign it to square
            attacks |= king_covered_squares_bitboards[square]; // retrieve attack bitboard
            clear_last_active_bit(piece); // remove the evaluated piece
        }

        // QUEEN
        piece = pieces[1];
        while(piece){
            _BitScanForward64(&square, piece); 
            hash_index_rook = rook_hash_index(all_pieces, square, n_attacks_rook);
            hash_index_bishop = bishop_hash_index(all_pieces, square, n_attacks_bishop);
            attacks |= rook_covered_squares_bitboards[hash_index_rook]; 
            attacks |= bishop_covered_squares_bitboards[hash_index_bishop];
            clear_last_active_bit(piece);
        }

        // ROOK
        piece = pieces[2];
        while(piece){
            _BitScanForward64(&square, piece); 
            hash_index_rook = rook_hash_index(all_pieces, square, n_attacks_rook);
            attacks |= rook_covered_squares_bitboards[hash_index_rook]; 
            clear_last_active_bit(piece);
        }

        // BISHOP
        piece = pieces[3];
        while(piece){
            _BitScanForward64(&square, piece); 
            hash_index_bishop = bishop_hash_index(all_pieces, square, n_attacks_bishop);
            attacks |= bishop_covered_squares_bitboards[hash_index_bishop];
            clear_last_active_bit(piece);
        }

        // KNIGHT
        piece = pieces[4];
        while(piece){
            _BitScanForward64(&square, piece); 
            attacks |= knight_covered_squares_bitboards[square]; 
            clear_last_active_bit(piece);
        }

        // PAWNS
        piece = pieces[5];
        while(piece){
            _BitScanForward64(&square, piece); 
            attacks |= white_pawn_covered_squares_bitboards[square];
            clear_last_active_bit(piece);
        }
        
    }
    // black to move:
    else{

        // KING
        piece = pieces[6];
        // loop over all the pieces of the same type
        while(piece){
            _BitScanForward64(&square, piece); // find position of piece and assign it to square
            attacks |= king_covered_squares_bitboards[square]; // retrieve attack bitboard
            clear_last_active_bit(piece); // remove the evaluated piece
        }

        // QUEEN
        piece = pieces[7];
        while(piece){
            _BitScanForward64(&square, piece); 
            hash_index_rook = rook_hash_index(all_pieces, square, n_attacks_rook);
            hash_index_bishop = bishop_hash_index(all_pieces, square, n_attacks_bishop);
            attacks |= rook_covered_squares_bitboards[hash_index_rook]; 
            attacks |= bishop_covered_squares_bitboards[hash_index_bishop];
            clear_last_active_bit(piece);
        }

        // ROOK
        piece = pieces[8];
        while(piece){
            _BitScanForward64(&square, piece); 
            hash_index_rook = rook_hash_index(all_pieces, square, n_attacks_rook);
            attacks |= rook_covered_squares_bitboards[hash_index_rook]; 
            clear_last_active_bit(piece);
        }

        // BISHOP
        piece = pieces[9];
        while(piece){
            _BitScanForward64(&square, piece); 
            hash_index_bishop = bishop_hash_index(all_pieces, square, n_attacks_bishop);
            attacks |= bishop_covered_squares_bitboards[hash_index_bishop];
            clear_last_active_bit(piece);
        }

        // KNIGHT
        piece = pieces[10];
        while(piece){
            _BitScanForward64(&square, piece); 
            attacks |= knight_covered_squares_bitboards[square]; 
            clear_last_active_bit(piece);
        }

        // PAWNS
        piece = pieces[11];
        while(piece){
            _BitScanForward64(&square, piece); 
            attacks |= black_pawn_covered_squares_bitboards[square];
            clear_last_active_bit(piece);
        }

    }

    return attacks;
}

void get_passed_pawn_masks(){
    int i, j;
    uint64_t rank_bb, file_bb;
    for(int square = 0; square < 63; square++){
        i = square / 8; j = square % 8;
        // black:
        rank_bb = 0; file_bb = 0;
        for(int rank = i+1; rank < 7; rank++){
            rank_bb |= ranks_bitboards[rank];
        }
        for(int file = j-1; file <= j+1; file++){
            if(file < 0 || file > 8){ continue; }
            file_bb |= files_bitboards[file];  
        }
        mask_black_passed_pawn[square] = rank_bb & file_bb;
        // same for white:
        rank_bb = 0; file_bb = 0;
        for(int rank = i-1; rank > 0; rank--){
            rank_bb |= ranks_bitboards[rank];
        }
        for(int file = j-1; file <= j+1; file++){
            if(file < 0 || file > 8){ continue; }
            file_bb |= files_bitboards[file];  
        }
        mask_white_passed_pawn[square] = rank_bb & file_bb;
    }
}

size_t count_doubled_pawns(uint64_t pawn_bitboard){
    uint64_t bb = pawn_bitboard & (pawn_bitboard >> 8);
    return pop_count(bb);
}
