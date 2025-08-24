#include <Bitboards.h>
#include <Utilities.h>
#include <fstream>

Bitboard knight_covered_squares_bitboards[64]; 
Bitboard king_covered_squares_bitboards[64];
Bitboard white_pawn_covered_squares_bitboards[64];
Bitboard black_pawn_covered_squares_bitboards[64];
Bitboard white_pawn_advance_squares_bitboards[64];
Bitboard black_pawn_advance_squares_bitboards[64];

Bitboard rook_masks[64];
Magic rook_magics[64];
MaskAndMagic rook_mm[64];
//Bitboard rook_covered_squares_bitboards[64*N_ATTACKS_ROOK];
Bitboard rook_covered_squares_bb[64][N_ATTACKS_ROOK];

Bitboard bishop_masks[64];
Magic bishop_magics[64];
MaskAndMagic bishop_mm[64];
//Bitboard bishop_covered_squares_bitboards[64*N_ATTACKS_BISHOP];
Bitboard bishop_covered_squares_bb[64][N_ATTACKS_BISHOP];


Bitboard KnightCoveredSquares(int i, int j){
    Bitboard knight_bitboard = 0ULL;
    int i_target = 0, j_target = 0;
    for(int d = 0; d < 8; d++){
        i_target = i + knight_deltas[d][0]; 
        if(i_target < 0 || i_target > 7){ continue; }
        j_target = j + knight_deltas[d][1];
        if(j_target < 0 || j_target > 7){ continue; }
        bit_set(knight_bitboard, 8 * i_target + j_target);
    }
    return knight_bitboard;
}

Bitboard KingCoveredSquares(int i, int j){
    Bitboard king_bitboard = 0ULL;
    int i_target = 0, j_target = 0;
    for(int d = 0; d < 8; d++){
        i_target = i + king_deltas[d][0]; 
        if(i_target < 0 || i_target > 7){ continue; }
        j_target = j + king_deltas[d][1];
        if(j_target < 0 || j_target > 7){ continue; }
        bit_set(king_bitboard, 8 * i_target + j_target);
    }
    return king_bitboard;
}

Bitboard WhitePawnCoveredSquares(int i, int j){
    Bitboard white_pawn_bitboard = 0ULL;
    int i_target = 0, j_target = 0;
    for(int d = 0; d < 2; d++){
        i_target = i + white_pawn_deltas[d][0]; 
        if(i_target < 0 || i_target > 7){ continue; }
        j_target = j + white_pawn_deltas[d][1];
        if(j_target < 0 || j_target > 7){ continue; }
        bit_set(white_pawn_bitboard, 8 * i_target + j_target);
    }
    return white_pawn_bitboard;
}

Bitboard BlackPawnCoveredSquares(int i, int j){
    Bitboard black_pawn_bitboard = 0ULL;
    int i_target = 0, j_target = 0;
    for(int d = 0; d < 2; d++){
        i_target = i + black_pawn_deltas[d][0]; 
        if(i_target < 0 || i_target > 7){ continue; }
        j_target = j + black_pawn_deltas[d][1];
        if(j_target < 0 || j_target > 7){ continue; }
        bit_set(black_pawn_bitboard, 8 * i_target + j_target);
    }
    return black_pawn_bitboard;
}

Bitboard WhitePawnAdvanceSquares(int i, int j){
    Bitboard white_pawn_bitboard = 0ULL;
    int i_target = 0, j_target = 0;
    if(i != 0){
        i_target = i - 1; j_target = j;
        bit_set(white_pawn_bitboard, 8 * i_target + j_target);
        if(i == 6){
            i_target = i - 2; j_target = j;
            bit_set(white_pawn_bitboard, 8 * i_target + j_target);
        }
    }
    return white_pawn_bitboard;
}

Bitboard BlackPawnAdvanceSquares(int i, int j){
    Bitboard black_pawn_bitboard = 0ULL;
    int i_target = 0, j_target = 0;
    if(i != 7){
        i_target = i + 1; j_target = j;
        bit_set(black_pawn_bitboard, 8 * i_target + j_target);
        if(i == 1){
            i_target = i + 2; j_target = j;
            bit_set(black_pawn_bitboard, 8 * i_target + j_target);
        }
    }
    return black_pawn_bitboard;
}

Bitboard RookRelevantBlockersMask(int i, int j){
    Bitboard bitboard = 0ULL;
    for(int j_target = 1; j_target < 7; j_target++){
        if(j_target == j){ continue; }
        bit_set(bitboard, 8 * i + j_target);
    }
    for(int i_target = 1; i_target < 7; i_target++){
        if(i_target == i){ continue; }
        bit_set(bitboard, 8 * i_target + j);
    }
    return bitboard;
}

Bitboard BishopRelevantBlockersMask(int i, int j){
    Bitboard bitboard = 0ULL;
    int i_target = 0, j_target = 0;
    for(int d = -6; d < 7; d++){
        i_target = i + d;
        j_target = j + d;
        if(i_target < 1 || i_target > 6 || j_target < 1 || j_target > 6 ){ continue; }
        else if(j_target == j || i_target == i) { continue; }
        bit_set(bitboard, 8 * i_target + j_target);
    }
    for(int d = -6; d < 7; d++){
        i_target = i + d;
        j_target = j - d;
        if(i_target < 1 || i_target > 6 || j_target < 1 || j_target > 6 ){ continue; }
        else if(j_target == j || i_target == i) { continue; }
        bit_set(bitboard, 8 * i_target + j_target);
    } 
    return bitboard;
}

// ----------- SLIDING PIECES ---------------
Bitboard RookCoveredSquaresFromBlockers(Bitboard blockers, int i, int j){
    Bitboard bitboard = 0ULL;
    // bottom sliding
    for(int i_target = i+1; i_target < 8; i_target++){
        bit_set(bitboard, 8 * i_target + j);
        if(bit_get(blockers, 8 * i_target + j)){ break; }
    }
    // top sliding
    for(int i_target = i-1; i_target >= 0; i_target--){
        bit_set(bitboard, 8 * i_target + j);
        if(bit_get(blockers, 8 * i_target + j)){ break; }
    }
    // right sliding
    for(int j_target = j+1; j_target < 8; j_target++){
        bit_set(bitboard, 8 * i + j_target);
        if(bit_get(blockers, 8 * i + j_target)){ break; }
    }
    // left sliding
    for(int j_target = j-1; j_target >= 0; j_target--){
        bit_set(bitboard, 8 * i + j_target);
        if(bit_get(blockers, 8 * i + j_target)){ break; }
    }
    return bitboard;
}

Bitboard BishopCoveredSquaresFromBlockers(Bitboard blockers, int i, int j){
    Bitboard bitboard = 0ULL;
    // bottom-right sliding
    for(int i_target = i+1, j_target = j+1; i_target < 8 && j_target < 8; i_target++, j_target++){
        bit_set(bitboard, 8 * i_target + j_target);
        if(bit_get(blockers, 8 * i_target + j_target)){ break; }
    }
    // top-right sliding
    for(int i_target = i-1, j_target = j+1; i_target >= 0 && j_target < 8; i_target--, j_target++){
        bit_set(bitboard, 8 * i_target + j_target);
        if(bit_get(blockers, 8 * i_target + j_target)){ break; }
    }
    // bottom-left sliding
    for(int i_target = i+1, j_target = j-1; i_target < 8 && j_target >= 0; i_target++, j_target--){
        bit_set(bitboard, 8 * i_target + j_target);
        if(bit_get(blockers, 8 * i_target + j_target)){ break; }
    }
    // top-left sliding
    for(int i_target = i-1, j_target = j-1; i_target >= 0 && j_target >=0; i_target--, j_target--){
        bit_set(bitboard, 8 * i_target + j_target);
        if(bit_get(blockers, 8 * i_target + j_target)){ break; }
    }
    return bitboard;
}

Bitboard RookBlockersFromInteger(Bitboard b, int i, int j){
    Bitboard blockers = 0ULL;
    // manage errors
    if(b >= 4096){
        std::cout << "Error. Trying to generate blockers from integer which is too large. Returning 0.\n";
        return blockers;
    }
    // convert b into an array of relevant bits
    bool bits[12]; 
    for(int index = 0; index < 12; index++){
        bits[index] = bit_get(b, index); // <--- trick to extract the first bits of b
    }
    // now rearrange the bits in the j-th column and i-th row, excluding the square (i,j)
    int index = 0;
    for(int i_target = 1; i_target < 7; i_target++){
        if(i_target == i){ continue; }
        if(bits[index]){ 
            bit_set(blockers, 8 * i_target + j);
        }
        index++;
    }
    for(int j_target = 1; j_target < 7; j_target++){
        if(j_target == j){ continue; }
        if(bits[index]){ 
            bit_set(blockers, 8 * i + j_target);
        }
        index++;
    }
    return blockers;
}

Bitboard BishopBlockersFromInteger(Bitboard b, int i, int j){
    Bitboard blockers = 0ULL;
    //int b_max = IntPow(2, n_squares_for_bishop_blockers[8*i+j]);
    int b_max = 1 << n_squares_for_bishop_blockers[8*i+j];
    // manage errors
    if(b >= b_max){
        std::cout << "Error. Trying to generate blockers from integer which is too large. Returning 0.\n";
        return blockers;
    }
    // convert b into an array of relevant bits
    bool *bits = new bool [b_max]; 
    for(int index = 0; index < b_max; index++){
        bits[index] = bit_get(b, index); // <--- trick to extract the first bits of b
    }
    // now rearrange the bits in the diagonal of the square (i, j), excluding the edge squares
    int index = 0;
    // bottom-right sliding
    for(int i_target = i+1, j_target = j+1; i_target < 7 && j_target < 7; i_target++, j_target++){
        if(bits[index]){ 
            bit_set(blockers, 8 * i_target + j_target);
        }
        index++;
    }
    // top-right sliding
    for(int i_target = i-1, j_target = j+1; i_target > 0 && j_target < 7; i_target--, j_target++){
        if(bits[index]){ 
            bit_set(blockers, 8 * i_target + j_target);
        }
        index++;
    }
    // bottom-left sliding
    for(int i_target = i+1, j_target = j-1; i_target < 7 && j_target > 0; i_target++, j_target--){
        if(bits[index]){ 
            bit_set(blockers, 8 * i_target + j_target);
        }
        index++;
    }
    // top-left sliding
    for(int i_target = i-1, j_target = j-1; i_target > 0 && j_target > 0; i_target--, j_target--){
        if(bits[index]){ 
            bit_set(blockers, 8 * i_target + j_target);
        }
        index++;
    }
    delete [] bits;
    return blockers;
}

void FindRookMagic(){
    //int shift = 64 - n_bits;
    //int n_attacks = IntPow(2, n_bits);
    //int n_attacks = 1 << n_bits;
    Magic magic = 0ULL; 
    Bitboard blockers = 0ULL, attack = 0ULL, stored_attack = 0ULL, hash_index = 0ULL;
    int i = 0, j = 0;
    bool success = false;
    // loop over the whole board
    for(int square = 0; square < 64; square++){
        i = square/8; j = square % 8;
        std::cout << "Searching magics for square: " << square << "\t";
        // loop until you find the good magic number
        success = false;
        while(!success){
            // create random magic number (with many zero bits)
            magic = rand64() & rand64();
            // reset the attacks array with 111111... numbers
            for(int index = 0; index < N_ATTACKS_ROOK; index++){
                rook_covered_squares_bb[square][index] = ~0ULL;
            }
            // loop over all the blockers~
            for(int b = 0; b < N_ATTACKS_ROOK; b++){
                // generate bitboard of blockers corresponding to a given index
                blockers = RookBlockersFromInteger(b, i, j);
                // generate attack bitboard given the blockers configuration
                attack = RookCoveredSquaresFromBlockers(blockers, i, j);
                // generate hash index
                hash_index = (blockers * magic) >> SHIFT_ROOK;
                if(hash_index >= N_ATTACKS_ROOK){ std::cout << "Error: incorrect hashing of rooks.\n"; }
                // check what is stored in the attacks array in correspodence to the hash index:
                stored_attack = rook_covered_squares_bb[square][hash_index];
                // if attack is stored, check for conflict and in case of conflict, change magic number
                if(stored_attack != ~(0ULL)){
                    if(stored_attack != attack){
                        break; // by breaking the loop, the magic number is changed
                    }
                }
                else{
                    rook_covered_squares_bb[square][hash_index] = attack;
                }
                // if no conflict occured at the end of the inner loop, the search for magics was successful
                if(b == N_ATTACKS_ROOK-1){
                    success = true;
                }
            }
        }
        std::cout << " Found the number " << magic << "\n";
        rook_magics[square] = magic;
    }
}

void FindBishopMagic(){
    //int shift = 64 - n_bits;
    //int n_attacks = IntPow(2, n_bits);
    //int n_attacks = 1 << n_bits;
    Magic magic = 0ULL;
    Bitboard blockers = 0ULL, attack = 0ULL, stored_attack = 0ULL, hash_index = 0ULL;
    int i = 0, j = 0;
    int b_max = 0;
    bool success = false;
    // loop over the whole board
    for(int square = 0; square < 64; square++){
        i = square/8; j = square % 8;
        //b_max = IntPow(2, n_squares_for_bishop_blockers[square]);
        b_max = 1 << n_squares_for_bishop_blockers[square];
        std::cout << "Searching magics for square: " << square << "\t";
        // loop until you find the good magic number
        success = false;
        while(!success){
            // create random magic number (with many zero bits)
            magic = rand64() & rand64();
            // reset the attacks array with 111111... numbers
            for(int index = 0; index < N_ATTACKS_BISHOP; index++){
                bishop_covered_squares_bb[square][index] = ~(0ULL);
            }
            // loop over all the blockers
            for(int b = 0; b < b_max; b++){
                // generate bitboard of blockers corresponding to a given index
                blockers = BishopBlockersFromInteger(b, i, j);
                // generate attack bitboard given the blockers configuration
                attack = BishopCoveredSquaresFromBlockers(blockers, i, j);
                // generate hash index
                hash_index = (blockers * magic) >> SHIFT_BISHOP;
                if(hash_index >= N_ATTACKS_BISHOP){ std::cout << "Error!\n"; }
                // check what is stored in the attacks array in correspodence to the hash index:
                stored_attack = bishop_covered_squares_bb[square][hash_index];
                // if attack is stored, check for conflict and in case of conflict, change magic number
                if(stored_attack != ~(0ULL)){
                    if(stored_attack != attack)
                        break; // by breaking the loop, the magic number is changed
                }
                else
                    bishop_covered_squares_bb[square][hash_index] = attack;
                // if no conflict occured at the end of the inner loop, the search for magics was successful
                if(b == b_max-1)
                    success = true;
            }
        }
        std::cout << " Found the number " << magic << "\n";
        bishop_magics[square] = magic;
    }
} 

void PreComputeBitboards(bool retrieve_from_file){
    Bitboard bitboard = 0ULL;
    int i = 0, j = 0;
    // dynamic allocate attack arrays
    //int n_rook_attacks = IntPow(2, n_bits_rook);
    //int n_bishop_attacks = IntPow(2, n_bits_bishop);
    //rook_covered_squares_bitboards = new uint64_t[64 * n_attacks_rook];
    //bishop_covered_squares_bitboards = new uint64_t[64 * n_attacks_bishop];
    //1 initialize covered squares for non sliding pieces:
    for(int square = 0; square < 64; square++){
        i = square / 8; j = square % 8;
        // knight bitboards
        bitboard = KnightCoveredSquares(i,j);
        knight_covered_squares_bitboards[square] = bitboard;
        // king bitboards
        bitboard = KingCoveredSquares(i,j);
        king_covered_squares_bitboards[square] = bitboard;
        // pawn bitboards
        bitboard = WhitePawnCoveredSquares(i, j);
        white_pawn_covered_squares_bitboards[square] = bitboard;
        bitboard = WhitePawnAdvanceSquares(i, j);
        white_pawn_advance_squares_bitboards[square] = bitboard;
        bitboard = BlackPawnCoveredSquares(i, j);
        black_pawn_covered_squares_bitboards[square] = bitboard;
        bitboard = BlackPawnAdvanceSquares(i, j);
        black_pawn_advance_squares_bitboards[square] = bitboard;
        // masks of relevant bitboards
        rook_masks[square] = RookRelevantBlockersMask(i, j);
        bishop_masks[square] = BishopRelevantBlockersMask(i, j);
    }
    // initialize covered squares for sliding pieces and save them
    if(!retrieve_from_file){
        FindRookMagic();
        FindBishopMagic();
        // save data
        write_to_file(rook_magics, 64, "../assets/rook_magics.txt");
        write_to_file(rook_covered_squares_bb, "../assets/rook_attacks.txt");
        write_to_file(bishop_magics, 64, "../assets/bishop_magics.txt");
        write_to_file(bishop_covered_squares_bb, "../assets/bishop_attacks.txt");
    }
    // or load from a file
    else{
        read_from_file(rook_magics, 64, "../assets/rook_magics.txt");
        read_from_file(rook_covered_squares_bb, "../assets/rook_attacks.txt");
        read_from_file(bishop_magics, 64, "../assets/bishop_magics.txt");
        read_from_file(bishop_covered_squares_bb, "../assets/bishop_attacks.txt");
    }
    // try this ...
    for(Square square = 0; square < 64; square++){
        rook_mm[square] = {rook_masks[square], rook_magics[square]}; 
        bishop_mm[square] = {bishop_masks[square], bishop_magics[square]};
    }
    // initialize masks for passed pawn and outpost detection
    GetPassedPawnMasks();
}


// generate the bitboard of covered squares by a given side (white or black)
Bitboard GetCoveredSquares(Bitboard pieces[12], Bitboard& all_pieces, bool by_white){
    Bitboard piece = 0ULL;
    Bitboard attacks = 0ULL;
    unsigned long square = 0;
    uint64_t hash_index_rook = 0ULL, hash_index_bishop = 0ULL;

    if(by_white){

        // KING
        piece = pieces[WHITE_KING];
        // loop over all the pieces of the same type
        while(piece){
            _BitScanForward64(&square, piece); // find position of piece and assign it to square
            attacks |= king_covered_squares_bitboards[square]; // retrieve attack bitboard
            clear_last_active_bit(piece); // remove the evaluated piece
        }

        // QUEEN
        piece = pieces[WHITE_QUEEN];
        while(piece){
            _BitScanForward64(&square, piece); 
            hash_index_rook = RookHashIndex(all_pieces, square);
            hash_index_bishop = BishopHashIndex(all_pieces, square);
            attacks |= rook_covered_squares_bb[square][hash_index_rook]; 
            attacks |= bishop_covered_squares_bb[square][hash_index_bishop];
            clear_last_active_bit(piece);
        }

        // ROOK
        piece = pieces[WHITE_ROOK];
        while(piece){
            _BitScanForward64(&square, piece); 
            hash_index_rook = RookHashIndex(all_pieces, square);
            attacks |= rook_covered_squares_bb[square][hash_index_rook]; 
            clear_last_active_bit(piece);
        }

        // BISHOP
        piece = pieces[WHITE_BISHOP];
        while(piece){
            _BitScanForward64(&square, piece); 
            hash_index_bishop = BishopHashIndex(all_pieces, square);
            attacks |= bishop_covered_squares_bb[square][hash_index_bishop];
            clear_last_active_bit(piece);
        }

        // KNIGHT
        piece = pieces[WHITE_KNIGHT];
        while(piece){
            _BitScanForward64(&square, piece); 
            attacks |= knight_covered_squares_bitboards[square]; 
            clear_last_active_bit(piece);
        }

        // PAWNS
        piece = pieces[WHITE_PAWN];
        while(piece){
            _BitScanForward64(&square, piece); 
            attacks |= white_pawn_covered_squares_bitboards[square];
            clear_last_active_bit(piece);
        }
        
    }
    // black to move:
    else{

        // KING
        piece = pieces[BLACK_KING];
        // loop over all the pieces of the same type
        while(piece){
            _BitScanForward64(&square, piece); // find position of piece and assign it to square
            attacks |= king_covered_squares_bitboards[square]; // retrieve attack bitboard
            clear_last_active_bit(piece); // remove the evaluated piece
        }

        // QUEEN
        piece = pieces[BLACK_QUEEN];
        while(piece){
            _BitScanForward64(&square, piece); 
            hash_index_rook = RookHashIndex(all_pieces, square);
            hash_index_bishop = BishopHashIndex(all_pieces, square);
            attacks |= rook_covered_squares_bb[square][hash_index_rook]; 
            attacks |= bishop_covered_squares_bb[square][hash_index_bishop];
            clear_last_active_bit(piece);
        }

        // ROOK
        piece = pieces[BLACK_ROOK];
        while(piece){
            _BitScanForward64(&square, piece); 
            hash_index_rook = RookHashIndex(all_pieces, square);
            attacks |= rook_covered_squares_bb[square][hash_index_rook]; 
            clear_last_active_bit(piece);
        }

        // BISHOP
        piece = pieces[BLACK_BISHOP];
        while(piece){
            _BitScanForward64(&square, piece); 
            hash_index_bishop = BishopHashIndex(all_pieces, square);
            attacks |= bishop_covered_squares_bb[square][hash_index_bishop];
            clear_last_active_bit(piece);
        }

        // KNIGHT
        piece = pieces[BLACK_KNIGHT];
        while(piece){
            _BitScanForward64(&square, piece); 
            attacks |= knight_covered_squares_bitboards[square]; 
            clear_last_active_bit(piece);
        }

        // PAWNS
        piece = pieces[BLACK_PAWN];
        while(piece){
            _BitScanForward64(&square, piece); 
            attacks |= black_pawn_covered_squares_bitboards[square];
            clear_last_active_bit(piece);
        }

    }

    return attacks;
}


Bitboard mask_white_passed_pawn[64];
Bitboard mask_black_passed_pawn[64];

void GetPassedPawnMasks(){
    uint8_t i, j;
    Bitboard rank_bb, file_bb;
    for(Square square = 0; square < 64; square++){
        i = square / 8; j = square % 8;
        // black:
        rank_bb = 0ULL; file_bb = 0ULL;
        for(uint8_t rank = i+1; rank < 7; rank++){
            rank_bb |= ranks_bitboards[rank];
        }
        for(uint8_t file = j-1; file <= j+1; file++){
            if(file < 0 || file > 8){ continue; }
            file_bb |= files_bitboards[file];  
        }
        mask_black_passed_pawn[square] = rank_bb & file_bb;
        // same for white:
        rank_bb = 0ULL; file_bb = 0ULL;
        for(uint8_t rank = i-1; rank > 0; rank--){
            rank_bb |= ranks_bitboards[rank];
        }
        for(uint8_t file = j-1; file <= j+1; file++){
            if(file < 0 || file > 8){ continue; }
            file_bb |= files_bitboards[file];  
        }
        mask_white_passed_pawn[square] = rank_bb & file_bb;
    }
}

size_t count_doubled_pawns(Bitboard pawn_bb){
    Bitboard bb = pawn_bb & ((pawn_bb >> 8) | (pawn_bb >> 16) | (pawn_bb >> 24) | (pawn_bb >> 32));
    return pop_count(bb);
}