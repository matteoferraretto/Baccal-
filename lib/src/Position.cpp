#include <Position.h>
#include <Utilities.h>
#include <Bitboards.h>
#include <iostream>
#include <sstream>
#include <cstdint>
#include <intrin.h>
#include <vector>

Position PositionFromFen(std::string fen)
{
    Position pos;

    // take the fen string and separate the "words" separated by spaces
    std::stringstream ss(fen);
    std::vector<std::string> words;
    std::string word;
    while (ss >> word) {
        words.push_back(word);
    }

    // get board in the form of a 8x8 char matrix
    int i = 0; 
    int j = 0; 
    int square = 0;
    int c_casted; 
    // loop over characters of the FEN string
    for(char& c: words[0]){
        c_casted = c - '0';
        if(c_casted == 8) { continue; } // if c is 8, ignore
        if(c_casted == -1) { i++; j = 0; continue; } // if c is '/', increment row index and reset column index to 0
        if(c_casted > 0 && c_casted < 8) { j += (unsigned int) c_casted; continue; }
        square = 8*i + j;
        // depending on the piece, determine the masks
        if(c == 'K'){ 
            //pos.white_material_value += WHITE_KING_VALUE;
            bit_set(pos.pieces[0], i, j);
            bit_set(pos.white_pieces, i, j);
            pos.white_covered_squares |= king_covered_squares_bitboards[square];
        }
        else if(c == 'Q'){ 
            //pos.white_material_value += WHITE_QUEEN_VALUE;
            bit_set(pos.pieces[1], i, j);
            bit_set(pos.white_pieces, i, j);
            // calculation of covered squares is delayed for sliding pieces --> see below
        }
        else if(c == 'R'){ 
            //pos.white_material_value += WHITE_ROOK_VALUE;
            bit_set(pos.pieces[2], i, j);
            bit_set(pos.white_pieces, i, j);
            // calculation of covered squares is delayed for sliding pieces --> see below
        }
        else if(c == 'B'){ 
            //pos.white_material_value += WHITE_BISHOP_VALUE;
            bit_set(pos.pieces[3], i, j);
            bit_set(pos.white_pieces, i, j);
            // calculation of covered squares is delayed for sliding pieces --> see below
        }
        else if(c == 'N'){ 
            //pos.white_material_value += WHITE_KNIGHT_VALUE;
            bit_set(pos.pieces[4], i, j);
            bit_set(pos.white_pieces, i, j);
            pos.white_covered_squares |= knight_covered_squares_bitboards[square];
        }
        else if(c == 'P'){ 
            //pos.white_material_value += WHITE_PAWN_VALUE;
            bit_set(pos.pieces[5], i, j);
            bit_set(pos.white_pieces, i, j);
            pos.white_covered_squares |= white_pawn_covered_squares_bitboards[square];
        }
        else if(c == 'k'){ 
            //pos.black_material_value += BLACK_KING_VALUE;
            bit_set(pos.pieces[6], i, j);
            bit_set(pos.black_pieces, i, j);
            pos.black_covered_squares |= king_covered_squares_bitboards[square];
        }
        else if(c == 'q'){ 
            //pos.black_material_value += BLACK_QUEEN_VALUE;
            bit_set(pos.pieces[7], i, j);
            bit_set(pos.black_pieces, i, j);
            // calculation of covered squares is delayed for sliding pieces --> see below
        }
        else if(c == 'r'){ 
            //pos.black_material_value += BLACK_ROOK_VALUE;
            bit_set(pos.pieces[8], i, j);
            bit_set(pos.black_pieces, i, j);
            // calculation of covered squares is delayed for sliding pieces --> see below
        }
        else if(c == 'b'){ 
            //pos.black_material_value += BLACK_BISHOP_VALUE;
            bit_set(pos.pieces[9], i, j);
            bit_set(pos.black_pieces, i, j);
            // calculation of covered squares is delayed for sliding pieces --> see below
        }
        else if(c == 'n'){ 
            //pos.black_material_value += BLACK_KNIGHT_VALUE;
            bit_set(pos.pieces[10], i, j);
            bit_set(pos.black_pieces, i, j);
            pos.black_covered_squares |= knight_covered_squares_bitboards[square];
        }
        else if(c == 'p'){ 
            //pos.black_material_value += BLACK_PAWN_VALUE;
            bit_set(pos.pieces[11], i, j);
            bit_set(pos.black_pieces, i, j);
            pos.black_covered_squares |= black_pawn_covered_squares_bitboards[square];
        }
        j++; 
    }

    // set side to move
    if(words[1] == "w") { pos.white_to_move = true; }
    else pos.white_to_move = false;

    // set castling rights
    for(char& c: words[2]){
        if(c == '-') { break; }
        else if(c == 'K') { pos.can_white_castle_kingside = true; continue; }
        else if(c == 'Q') { pos.can_white_castle_queenside = true; continue; }
        else if(c == 'k') { pos.can_black_castle_kingside = true; continue; }
        else if(c == 'q') { pos.can_black_castle_queenside = true; continue; }
    }

    // set en-passant target square if different from '-'
    if(words[3] != "-"){
        pos.en_passant_target_square = AlphabetToBitboard(words[3]);
    }

    // set half move counter and move counter from fen
    pos.half_move_counter = std::stoi(words[4]);
//    pos.move_counter = std::stoi(words[5]);

    // complete pieces bitboards for sliding pieces
    uint64_t hash_index, piece;
    unsigned long sq;
    int n_attacks_rook = IntPow(2, n_bits_rook);
    int n_attacks_bishop = IntPow(2, n_bits_bishop);
    pos.all_pieces = pos.white_pieces | pos.black_pieces;
    // white rook covered squares
    piece = pos.pieces[2]; 
    while(piece){ // loop through all the occurences of the rook
        _BitScanForward64(&sq, piece); // find square
        hash_index = rook_hash_index(pos.all_pieces, sq, n_attacks_rook); // find hash index for that square
        pos.white_covered_squares |= rook_covered_squares_bitboards[hash_index]; // generate covered squares
        clear_last_active_bit(piece); // remove the piece and consider the next one
    }
    // black rook covered squares
    piece = pos.pieces[8]; 
    while(piece){ // loop through all the occurences of the rook
        _BitScanForward64(&sq, piece); // find square
        hash_index = rook_hash_index(pos.all_pieces, sq, n_attacks_rook); // find hash index for that square
        pos.black_covered_squares |= rook_covered_squares_bitboards[hash_index]; // generate covered squares
        clear_last_active_bit(piece); // remove the piece and consider the next one
    }
    // white bishop 
    piece = pos.pieces[3];
    while(piece){ // loop through all the occurences of the rook
        _BitScanForward64(&sq, piece); // find square
        hash_index = bishop_hash_index(pos.all_pieces, sq, n_attacks_bishop); // find hash index for that square
        pos.white_covered_squares |= bishop_covered_squares_bitboards[hash_index]; // generate covered squares
        clear_last_active_bit(piece); // remove the piece and consider the next one
    }
    // black bishop 
    piece = pos.pieces[9];
    while(piece){ // loop through all the occurences of the rook
        _BitScanForward64(&sq, piece); // find square
        hash_index = bishop_hash_index(pos.all_pieces, sq, n_attacks_bishop); // find hash index for that square
        pos.black_covered_squares |= bishop_covered_squares_bitboards[hash_index]; // generate covered squares
        clear_last_active_bit(piece); // remove the piece and consider the next one
    }
    // white queen 
    piece = pos.pieces[1];
    while(piece){ // loop through all the occurences of the rook
        _BitScanForward64(&sq, piece); // find square
        hash_index = bishop_hash_index(pos.all_pieces, sq, n_attacks_bishop); // find hash index for that square
        pos.white_covered_squares |= bishop_covered_squares_bitboards[hash_index]; // generate covered squares
        hash_index = rook_hash_index(pos.all_pieces, sq, n_attacks_rook); // find hash index for that square
        pos.white_covered_squares |= rook_covered_squares_bitboards[hash_index]; // generate covered squares
        clear_last_active_bit(piece); // remove the piece and consider the next one
    }
    // black queen 
    piece = pos.pieces[7];
    while(piece){ // loop through all the occurences of the rook
        _BitScanForward64(&sq, piece); // find square
        hash_index = bishop_hash_index(pos.all_pieces, sq, n_attacks_bishop); // find hash index for that square
        pos.black_covered_squares |= bishop_covered_squares_bitboards[hash_index]; // generate covered squares
        hash_index = rook_hash_index(pos.all_pieces, sq, n_attacks_rook); // find hash index for that square
        pos.black_covered_squares |= rook_covered_squares_bitboards[hash_index]; // generate covered squares
        clear_last_active_bit(piece); // remove the piece and consider the next one
    }
    
    return pos;
}


void PrintBoard(Position pos){
    char board[64];
    char pieces_list[12] = {'K', 'Q', 'R', 'B', 'N', 'P', 'k', 'q', 'r', 'b', 'n', 'p'};
    for(int square = 0; square < 64; square++){ board[square] = '0'; } // initialize board
    unsigned long square;
    uint64_t piece;
    // loop through pieces bitboards
    for(int index = 0; index < 12; index++){
        piece = pos.pieces[index];
        if(piece != 0ULL){
            // loop over all pieces of the same type (e.g. find all the rooks, all the pawns etc...)
            while(piece){
                _BitScanForward64(&square, piece); // this changes square to the square where the piece is positioned
                clear_last_active_bit(piece);     
                board[square] = pieces_list[index]; // store it on the right square of the board with the right letter
            }
        }
    }
    // print
    for(int square = 0; square < 64; square++){
        if(square % 8 == 0){ std::cout << "\n"; }
        std::cout << board[square] << " ";
    }
    std::cout << "\n";
};

// assign a score to a given position
int PositionScore(Position& pos){
    // we assume that material value is pre-calculated! It should be done when a position is generated
    // this is the starting point for the position score
    int score = 0;
    unsigned long square;
    uint64_t piece;
    
    // loop over the pieces to add extra value based on the position of the piece
    
    // white king
    /*piece = pos.pieces[0];
    if(piece){
        _BitScanForward64(&square, piece);
        score += WHITE_KING_PST_MIDDLEGAME[square];
    }*/
    // white queen
    piece = pos.pieces[1];
    while(piece){ // loop until all the white queens are considered
        _BitScanForward64(&square, piece);
        score += QUEEN_PST[square];
        score += WHITE_QUEEN_VALUE;
        clear_last_active_bit(piece);   
    }
    // white rook
    piece = pos.pieces[2];
    while(piece){ // loop until all the white queens are considered
        _BitScanForward64(&square, piece);
        score += WHITE_ROOK_PST[square];
        score += WHITE_ROOK_VALUE;
        clear_last_active_bit(piece);   
    }
    // white bishop
    piece = pos.pieces[3];
    while(piece){ // loop until all the white queens are considered
        _BitScanForward64(&square, piece);
        score += BISHOP_PST[square];
        score += WHITE_BISHOP_VALUE;
        clear_last_active_bit(piece);   
    }
    // white knight
    piece = pos.pieces[4];
    while(piece){ // loop until all the white queens are considered
        _BitScanForward64(&square, piece);
        score += KNIGHT_PST[square];
        score += WHITE_KNIGHT_VALUE;
        clear_last_active_bit(piece);   
        // bonus for outpost squares ...
    }
    // white pawns
    piece = pos.pieces[5];
    while(piece){ // loop until all the white queens are considered
        _BitScanForward64(&square, piece);
        score += WHITE_PAWN_PST[square];
        score += WHITE_PAWN_VALUE;
        clear_last_active_bit(piece);   
        // bonus for passed pawns ...
        if(mask_white_passed_pawn[square] & pos.pieces[11]){ continue; }
        else{
            score += BONUS_FOR_PASSED_PAWNS;
        }
    }
    // black king
    /*piece = pos.pieces[6];
    if(piece){
        _BitScanForward64(&square, piece);
        score -= BLACK_KING_PST_MIDDLEGAME[square];
    }*/
    // black queen
    piece = pos.pieces[7];
    while(piece){ // loop until all the white queens are considered
        _BitScanForward64(&square, piece);
        score -= QUEEN_PST[square];
        score += BLACK_QUEEN_VALUE;
        clear_last_active_bit(piece);   
    }
    // black rook
    piece = pos.pieces[8];
    while(piece){ // loop until all the white queens are considered
        _BitScanForward64(&square, piece);
        score -= BLACK_ROOK_PST[square];
        score += BLACK_ROOK_VALUE;
        clear_last_active_bit(piece);   
    }
    // black bishop
    piece = pos.pieces[9];
    while(piece){ // loop until all the white queens are considered
        _BitScanForward64(&square, piece);
        score -= BISHOP_PST[square];
        score += BLACK_BISHOP_VALUE;
        clear_last_active_bit(piece);   
    }
    // black knight
    piece = pos.pieces[10];
    while(piece){ // loop until all the white queens are considered
        _BitScanForward64(&square, piece);
        score -= KNIGHT_PST[square];
        score += BLACK_KNIGHT_VALUE;
        clear_last_active_bit(piece);   
        // bonus for outpost ...
    }
    // black pawns
    piece = pos.pieces[11];
    while(piece){ // loop until all the white queens are considered
        _BitScanForward64(&square, piece);
        score -= BLACK_PAWN_PST[square];
        score += BLACK_PAWN_VALUE;
        clear_last_active_bit(piece);
        // bonus for passed pawns: the black pawn looks forward and if no white pawns are found, it is a passer
        if(mask_black_passed_pawn[square] & pos.pieces[5]){ continue; }
        else{
            score -= BONUS_FOR_PASSED_PAWNS;
        }
    }

    // malus for doubled pawns (or bonus for opponent's doubled pawns):
    score += (
        count_doubled_pawns(pos.pieces[11]) - count_doubled_pawns(pos.pieces[5])
    ) * MALUS_FOR_DOUBLED_PAWNS; 

    return score;
}

void PseudoLegalMoves(Position& pos, Move* moves){
    uint8_t move_index = 0;
    uint64_t piece;
    //uint64_t hash_index_rook, hash_index_bishop; 
    unsigned long square, target_square;
    uint64_t attacks;
    bool is_capture;
    uint16_t flags;
    MaskAndMagic mm;
    uint64_t not_your_pieces;

    // WHITE MOVES
    if(pos.white_to_move){

        not_your_pieces = ~pos.white_pieces;

        // King
        piece = pos.pieces[0];
        while(piece){
            // find position of piece and assign it to square
            _BitScanForward64(&square, piece); 
            attacks = king_covered_squares_bitboards[square]; // retrieve attack bitboard
            attacks &= not_your_pieces; // exclude self-capture
            while(attacks){
                _BitScanForward64(&target_square, attacks); // find the target square
                is_capture = bit_get(pos.black_pieces, target_square);
                is_capture ? flags = 4 : flags = 0;
                // save the move
                moves[move_index] = EncodeMove(square, target_square, flags);
                move_index++;
                // remove considered attack
                clear_last_active_bit(attacks); 
            }
            // remove considered piece
            clear_last_active_bit(piece);
        }

        // Queen
        piece = pos.pieces[1]; // retrieve bitboard of queens
        while(piece){ // consider all the queens
            _BitScanForward64(&square, piece); // find position of queen and assign it to square
            // retrieve bitboard of queen moves
            mm = rook_mm[square];
            attacks = rook_covered_squares_bb[square][((pos.all_pieces & mm.mask) * mm.magic) >> shift_rook];
            mm = bishop_mm[square];
            attacks |= bishop_covered_squares_bb[square][((pos.all_pieces & mm.mask) * mm.magic) >> shift_bishop];
            attacks &= not_your_pieces; // excluse self-capture
            while(attacks){
                _BitScanForward64(&target_square, attacks); // find the target square
                is_capture = bit_get(pos.black_pieces, target_square);
                is_capture ? flags = 4 : flags = 0;
                moves[move_index] = EncodeMove(square, target_square, flags);
                move_index++;
                clear_last_active_bit(attacks); 
            }
            clear_last_active_bit(piece);
        }

        // Rook
        piece = pos.pieces[2];
        while(piece){
            _BitScanForward64(&square, piece); // find position of queen and assign it to square
            // retrieve bitboard of queen moves
            mm = rook_mm[square];
            attacks = rook_covered_squares_bb[square][((pos.all_pieces & mm.mask) * mm.magic) >> shift_rook];
            attacks &= not_your_pieces; // excluse self-capture
            while(attacks){
                _BitScanForward64(&target_square, attacks); // find the target square
                is_capture = bit_get(pos.black_pieces, target_square);
                is_capture ? flags = 4 : flags = 0;
                moves[move_index] = EncodeMove(square, target_square, flags);
                move_index++;
                clear_last_active_bit(attacks); 
            }
            clear_last_active_bit(piece);
        }

        // Bishop
        piece = pos.pieces[3]; // retrieve bitboard of queens
        while(piece){ // consider all the queens
            _BitScanForward64(&square, piece); // find position of queen and assign it to square
            mm = bishop_mm[square];
            attacks = bishop_covered_squares_bb[square][((pos.all_pieces & mm.mask) * mm.magic) >> shift_bishop];
            attacks &= not_your_pieces; // excluse self-capture
            while(attacks){
                _BitScanForward64(&target_square, attacks); // find the target square
                is_capture = bit_get(pos.black_pieces, target_square);
                is_capture ? flags = 4 : flags = 0;
                moves[move_index] = EncodeMove(square, target_square, flags);
                move_index++;
                clear_last_active_bit(attacks); 
            }
            clear_last_active_bit(piece);
        }

        // Knight
        piece = pos.pieces[4]; // retrieve bitboard of knights
        while(piece){ // consider all the knights
            _BitScanForward64(&square, piece); // find position of piece and assign it to square
            is_capture = bit_get(pos.black_pieces, target_square);
            attacks = knight_covered_squares_bitboards[square]; // retrieve attack bitboard
            attacks &= not_your_pieces; // exclude self-capture
            while(attacks){
                _BitScanForward64(&target_square, attacks); // find the target square
                is_capture = bit_get(pos.black_pieces, target_square);
                is_capture ? flags = 4 : flags = 0;
                moves[move_index] = EncodeMove(square, target_square, flags);
                move_index++;
                clear_last_active_bit(attacks); 
            }
            // remove considered piece
            clear_last_active_bit(piece);
        }

        // Pawn capture
        piece = pos.pieces[5];
        while(piece){
            _BitScanForward64(&square, piece); // find position of piece and assign it to square
            // NORMAL CAPTURES
            attacks = white_pawn_covered_squares_bitboards[square]; // retrieve attack bitboard
            attacks &= pos.black_pieces; // only attacked squares occupied by enemy pieces are valid for movement
            while(attacks){
                _BitScanForward64(&target_square, attacks); // find the target square
                // in case of promotion, loop over all possible promoted pieces
                if(WHITE_PAWN_IN_FINAL_RANK[target_square]){
                    for(uint8_t promoted_piece_index = 1; promoted_piece_index < 5; promoted_piece_index++){
                        flags = 16 - promoted_piece_index;
                        moves[move_index] = EncodeMove(square, target_square, flags);
                        move_index++;
                    }
                }
                else{ 
                    flags = 4;
                    moves[move_index] = EncodeMove(square, target_square, flags);
                    move_index++;
                }
                clear_last_active_bit(attacks); // remove considered attack
            }
            // EN PASSANT CAPTURES
            attacks = white_pawn_covered_squares_bitboards[square]; // retrieve attack bitboard
            attacks &= pos.en_passant_target_square; // only attacked squares occupied by enemy pieces are valid for movement
            while(attacks){
                _BitScanForward64(&target_square, attacks); // find the target square            
                flags = 5;
                moves[move_index] = EncodeMove(square, target_square, flags);
                move_index++;
                clear_last_active_bit(attacks); // remove considered attack
            }

            // Pawn push
            attacks = white_pawn_advance_squares_bitboards[square]; // retrieve attack bitboard
            attacks &= ~pos.all_pieces; // control that there are no blockers in front
            // problem: so far, if a piece is in front of the pawn and the pawn is in the starting rank, it can still advance 2 squares!
            // everything is ok if the attack bitboard looks like this (1-square push and 2-square push both possible)
            // .............
            // ... 0 1 0 ...
            // ... 0 1 0 ...
            // ... 0 0 0 ...
            // ... 0 0 0 ...
            // or like this (single push is possible, double push is not)
            // .............
            // ... 0 0 0 ...
            // ... 0 1 0 ...
            // ... 0 0 0 ...
            // ... 0 0 0 ...
            // but a bitboard like this is not acceptable (2-square push is possible, 1-square push is not):
            // .............
            // ... 0 1 0 ...
            // ... 0 0 0 ...
            // ... 0 0 0 ...
            // ... 0 0 0 ...
            // (of course the latter is an acceptable bitboard if the pawn starting square is in the 3rd rank)
            if(WHITE_PAWN_IN_STARTING_RANK[square] && !bit_get(attacks, square - 8) && bit_get(attacks, square - 16)){
                attacks = 0ULL;
            }
            while(attacks){
                _BitScanForward64(&target_square, attacks); // find the target square
                // in case of promotion, loop over all possible promoted pieces
                if(WHITE_PAWN_IN_FINAL_RANK[target_square]){
                    for(uint8_t promoted_piece_index = 1; promoted_piece_index < 5; promoted_piece_index++){
                        flags = 12 - promoted_piece_index;
                        moves[move_index] = EncodeMove(square, target_square, flags);
                        move_index++;
                    }
                }
                // if this is a double push, flag it to manage en-passant target squares later
                else if(RANK_OF_SQUARE[target_square] == 4 && WHITE_PAWN_IN_STARTING_RANK[square]){
                    flags = 1;
                    moves[move_index] = EncodeMove(square, target_square, flags);
                    move_index++;
                }
                else{
                    flags = 0;
                    moves[move_index] = EncodeMove(square, target_square, flags);
                    move_index++;
                }
                clear_last_active_bit(attacks); // remove considered attack
            }
            // remove considered piece
            clear_last_active_bit(piece);
        }

        // Castles kingside
        // here I am nesting if statements, because if one of them fails there's no need to go ahead and check all the other conditions
        // the conditions involving bit_get() require a few bitwise operations, which we can confortably skip in many positions
        if(pos.can_white_castle_kingside){ // 1. you still have right to castle from game history
            //if(!bit_get(pos.all_pieces, 61) && !bit_get(pos.all_pieces, 62)){ // 2. the in-between squares are empty
            if(!(pos.all_pieces & (3ULL << 61))){
                // 3. king does not pass through a square covered by opponent
                //if(bit_get_opt(pos.pieces[0], 60) && bit_get_opt(pos.pieces[2], 63)){ // 4. king and rook are in the correct position
                if(pos.pieces[2] & (1ULL << 63)){
                    moves[move_index] = EncodeMove(60, 62, 2); 
                    move_index++;
                } 
            }   
        }
        // Castles queenside
        if(pos.can_white_castle_queenside){ 
            //if(!bit_get(pos.all_pieces, 59) && !bit_get(pos.all_pieces, 58) && !bit_get(pos.all_pieces, 57)){ 
            if(!(pos.all_pieces & (7ULL << 57))){
                //if(bit_get_opt(pos.pieces[0], 60) && bit_get_opt(pos.pieces[2], 56)){ 
                if(pos.pieces[2] & (1ULL << 56)){
                    moves[move_index] = EncodeMove(60, 58, 3); 
                    move_index++;
                } 
            }   
        }
    }

    // BLACK MOVES
    else{

        not_your_pieces = ~pos.black_pieces;

        // King
        piece = pos.pieces[6];
        while(piece){
            // find position of piece and assign it to square
            _BitScanForward64(&square, piece); 
            attacks = king_covered_squares_bitboards[square]; // retrieve attack bitboard
            attacks &= not_your_pieces; // exclude self-capture
            while(attacks){
                _BitScanForward64(&target_square, attacks); // find the target square
                is_capture = bit_get(pos.white_pieces, target_square);
                is_capture ? flags = 4 : flags = 0;
                // save the move
                moves[move_index] = EncodeMove(square, target_square, flags);
                move_index++;
                // remove considered attack
                clear_last_active_bit(attacks); 
            }
            // remove considered piece
            clear_last_active_bit(piece);
        }

        // Queen
        piece = pos.pieces[7]; // retrieve bitboard of queens
        while(piece){ // consider all the queens
            _BitScanForward64(&square, piece); // find position of queen and assign it to square
            // retrieve bitboard of queen moves
            mm = rook_mm[square];
            attacks = rook_covered_squares_bb[square][((pos.all_pieces & mm.mask) * mm.magic) >> shift_rook];
            mm = bishop_mm[square];
            attacks |= bishop_covered_squares_bb[square][((pos.all_pieces & mm.mask) * mm.magic) >> shift_bishop];
            attacks &= not_your_pieces; // excluse self-capture
            while(attacks){
                _BitScanForward64(&target_square, attacks); // find the target square
                is_capture = bit_get(pos.white_pieces, target_square);
                is_capture ? flags = 4 : flags = 0;
                moves[move_index] = EncodeMove(square, target_square, flags);
                move_index++;
                clear_last_active_bit(attacks); 
            }
            clear_last_active_bit(piece);
        }

        // Rook
        piece = pos.pieces[8];
        while(piece){
            _BitScanForward64(&square, piece); // find position of queen and assign it to square
            // retrieve bitboard of queen moves
            mm = rook_mm[square];
            attacks = rook_covered_squares_bb[square][((pos.all_pieces & mm.mask) * mm.magic) >> shift_rook];
            attacks &= not_your_pieces; // excluse self-capture
            while(attacks){
                _BitScanForward64(&target_square, attacks); // find the target square
                is_capture = bit_get(pos.white_pieces, target_square);
                is_capture ? flags = 4 : flags = 0;
                moves[move_index] = EncodeMove(square, target_square, flags);
                move_index++;
                clear_last_active_bit(attacks); 
            }
            clear_last_active_bit(piece);
        }

        // Bishop
        piece = pos.pieces[9]; // retrieve bitboard of queens
        while(piece){ // consider all the queens
            _BitScanForward64(&square, piece); // find position of queen and assign it to square
            mm = bishop_mm[square];
            attacks = bishop_covered_squares_bb[square][((pos.all_pieces & mm.mask) * mm.magic) >> shift_bishop];
            attacks &= not_your_pieces; // excluse self-capture
            while(attacks){
                _BitScanForward64(&target_square, attacks); // find the target square
                is_capture = bit_get(pos.white_pieces, target_square);
                is_capture ? flags = 4 : flags = 0;
                moves[move_index] = EncodeMove(square, target_square, flags);
                move_index++;
                clear_last_active_bit(attacks); 
            }
            clear_last_active_bit(piece);
        }

        // Knight
        piece = pos.pieces[10]; // retrieve bitboard of knights
        while(piece){ // consider all the knights
            _BitScanForward64(&square, piece); // find position of piece and assign it to square
            is_capture = bit_get(pos.white_pieces, target_square);
            attacks = knight_covered_squares_bitboards[square]; // retrieve attack bitboard
            attacks &= not_your_pieces; // exclude self-capture
            while(attacks){
                _BitScanForward64(&target_square, attacks); // find the target square
                is_capture = bit_get(pos.white_pieces, target_square);
                //is_capture ? flags = 4 : flags = 0;
                flags = (pos.white_pieces & (1ULL << target_square)) ? 4 : 0;  
                moves[move_index] = EncodeMove(square, target_square, flags);
                move_index++;
                clear_last_active_bit(attacks); 
            }
            // remove considered piece
            clear_last_active_bit(piece);
        }

        // Pawn capture
        piece = pos.pieces[11];
        while(piece){
            _BitScanForward64(&square, piece); // find position of piece and assign it to square
            // NORMAL CAPTURES
            attacks = black_pawn_covered_squares_bitboards[square]; // retrieve attack bitboard
            attacks &= pos.white_pieces; // only attacked squares occupied by enemy pieces are valid for movement
            while(attacks){
                _BitScanForward64(&target_square, attacks); // find the target square
                // in case of promotion, loop over all possible promoted pieces
                if(BLACK_PAWN_IN_FINAL_RANK[target_square]){
                    for(uint8_t promoted_piece_index = 7; promoted_piece_index < 11; promoted_piece_index++){
                        flags = 22 - promoted_piece_index;
                        moves[move_index] = EncodeMove(square, target_square, flags);
                        move_index++;
                    }
                }
                else{ 
                    flags = 4;
                    moves[move_index] = EncodeMove(square, target_square, flags);
                    move_index++;
                }
                clear_last_active_bit(attacks); // remove considered attack
            }
            // EN PASSANT CAPTURES
            attacks = black_pawn_covered_squares_bitboards[square]; // retrieve attack bitboard
            attacks &= pos.en_passant_target_square; // only attacked squares occupied by enemy pieces are valid for movement
            while(attacks){
                _BitScanForward64(&target_square, attacks); // find the target square
                flags = 5;
                moves[move_index] = EncodeMove(square, target_square, flags);
                move_index++;
                clear_last_active_bit(attacks); // remove considered attack
            }

            // Pawn push
            attacks = black_pawn_advance_squares_bitboards[square]; // retrieve attack bitboard
            attacks &= ~pos.all_pieces; // control that there are no blockers in front
            if(BLACK_PAWN_IN_STARTING_RANK[square] && !bit_get(attacks, square + 8) && bit_get(attacks, square + 16)){
                attacks = 0ULL;
            }
            while(attacks){
                _BitScanForward64(&target_square, attacks); // find the target square
                // in case of promotion, loop over all possible promoted pieces
                if(BLACK_PAWN_IN_FINAL_RANK[target_square]){
                    for(uint8_t promoted_piece_index = 7; promoted_piece_index < 11; promoted_piece_index++){
                        flags = 22 - promoted_piece_index;
                        moves[move_index] = EncodeMove(square, target_square, flags);
                        move_index++;
                    }
                }
                // if this is a double push, flag it to manage en-passant target squares later
                else if(RANK_OF_SQUARE[target_square] == 3 && BLACK_PAWN_IN_STARTING_RANK[square]){
                    flags = 1;
                    moves[move_index] = EncodeMove(square, target_square, flags);
                    move_index++;
                }
                else{
                    flags = 0;
                    moves[move_index] = EncodeMove(square, target_square, flags);
                    move_index++;
                }
                clear_last_active_bit(attacks); // remove considered attack
            }
            // remove considered piece
            clear_last_active_bit(piece);
        }

        // Castles kingside
        // here I am nesting if statements, because if one of them fails there's no need to go ahead and check all the other conditions
        // the conditions involving bit_get() require a few bitwise operations, which we can confortably skip in many positions
        if(pos.can_black_castle_kingside){ 
            //if(!bit_get(pos.all_pieces, 5) && !bit_get(pos.all_pieces, 6)){ 
            if(!(pos.all_pieces & 96ULL)){
                //if(bit_get_opt(pos.pieces[6], 4) && bit_get_opt(pos.pieces[8], 7)){ 
                //if(pos.pieces[6] & 16ULL){
                if(pos.pieces[8] & 128ULL){
                    moves[move_index] = EncodeMove(4, 6, 2);
                    move_index++;
                }
            }   
        }

        // Castles queenside
        if(pos.can_black_castle_queenside){ 
            //if(!bit_get(pos.all_pieces, 3) && !bit_get(pos.all_pieces, 2) && !bit_get(pos.all_pieces, 1)){ 
            if(!(pos.all_pieces & 14ULL)){
                //if(bit_get_opt(pos.pieces[6], 4) && bit_get_opt(pos.pieces[8], 0)){ 
                if(pos.pieces[8] & 1ULL){
                //if(pos.pieces[6] & 16ULL){
                    moves[move_index] = EncodeMove(4, 2, 3); 
                    move_index++;
                }
            }   
        }
    }
    pos.n_pseudolegal_moves = move_index;
}

const uint64_t WHITE_ROOK_KINGSIDE_CASTLE_MASK = (1ULL << 61) | (1ULL << 63);
const uint64_t WHITE_ROOK_QUEENSIDE_CASTLE_MASK = (1ULL << 56) | (1ULL << 59);
const uint64_t BLACK_ROOK_KINGSIDE_CASTLE_MASK = (1ULL << 7) | (1ULL << 5);
const uint64_t BLACK_ROOK_QUEENSIDE_CASTLE_MASK = (1ULL << 3) | 1ULL;

void MakeMove(Position& pos, const Move& move, StateMemory& state){
    uint8_t from, to, flags;
    uint8_t moved_piece_index = 0, captured_piece_index = 12, promoted_piece_index = 12; // 12 = no piece captured
    // retrieve info from the move
    from = move & 0b00111111;
    to = (move >> 6) & 0b00111111;
    flags = (move >> 12);
    // starting square and ending square bitboards
    uint64_t from_bb = (1ULL << from);
    uint64_t to_bb = (1ULL << to); 
    // WHITE TO MOVE
    if(pos.white_to_move){
        // retrieve what piece has moved
        for(uint8_t piece_index = 0; piece_index < 6; piece_index++){
            if(pos.pieces[piece_index] & from_bb){
                moved_piece_index = piece_index;
                break;
            }
        }
        // retrieve info about captured piece (if any)
        for(uint8_t piece_index = 6; piece_index < 12; piece_index++){
            // standard capture
            if(pos.pieces[piece_index] & to_bb){
                captured_piece_index = piece_index;
                break;
            } 
            // en-passant capture
            else if(flags == 5){
                captured_piece_index = 11; // <--- necessarily a pawn
                break;
            }
        }
        // update en passant target
        if(flags == 1){ // double pawn push
            state.en_passant_target_square = pos.en_passant_target_square;
            pos.en_passant_target_square = 1ULL << (to + 8);
        }
        else{
            pos.en_passant_target_square = 0ULL;
        }
        // retrieve info about promoted piece (if any)
        if(flags == 11 || flags == 15){ promoted_piece_index = 1; } // queen
        else if(flags == 10 || flags == 14){ promoted_piece_index = 2; } // rook
        else if(flags == 9 || flags == 13){ promoted_piece_index = 3; } // bishop
        else if(flags == 8 || flags == 12){ promoted_piece_index = 4; } // knight
        // save current state
        state.moved_piece_index = moved_piece_index;
        state.captured_piece_index = captured_piece_index;
        state.promoted_piece_index = promoted_piece_index;
        // move the piece
        //bit_clear_opt(pos.pieces[moved_piece_index], from); bit_set_opt(pos.pieces[moved_piece_index], to);
        pos.pieces[moved_piece_index] ^= from_bb | to_bb;
        pos.white_pieces ^= from_bb | to_bb;
        // remove captured piece, if any
        if(captured_piece_index != 12){
            // if en passant, piece is not in the target square
            if(flags == 5){
                bit_clear_opt(pos.pieces[11], to + 8);
                bit_clear_opt(pos.black_pieces, to + 8);
            }
            else{
                bit_clear_opt(pos.pieces[captured_piece_index], to);
                bit_clear_opt(pos.black_pieces, to);
            }
        }
        // spawn the promoted piece in case of promotion and remove the pawn
        if(promoted_piece_index != 12){
            bit_clear_opt(pos.pieces[5], to);
            bit_set_opt(pos.pieces[promoted_piece_index], to);
        }
        // Handle castling
        if(flags == 2){// kingside castle
            // transfer the rook
            pos.pieces[2] ^= WHITE_ROOK_KINGSIDE_CASTLE_MASK;
            pos.white_pieces ^= WHITE_ROOK_KINGSIDE_CASTLE_MASK;
        }
        else if(flags == 3){// queenside castle
            // transfer the rook
            pos.pieces[2] ^= WHITE_ROOK_QUEENSIDE_CASTLE_MASK;
            pos.white_pieces ^= WHITE_ROOK_QUEENSIDE_CASTLE_MASK;
        }

        // CASTLING RIGHTS
        // save current castling rights 
        state.can_white_castle_kingside = pos.can_white_castle_kingside;
        state.can_white_castle_queenside = pos.can_white_castle_queenside;
        // loose castling rights if current move is castling or king move 
        if(flags == 2 || flags == 3 || moved_piece_index == 0){
            pos.can_white_castle_kingside = false;
            pos.can_white_castle_queenside = false;
        }
        // if you move the rook on h1 
        if(moved_piece_index == 2 && from == 63){
            pos.can_white_castle_kingside = false;
        }
        // if you move the rook on a1
        if(moved_piece_index == 2 && from == 56){
            pos.can_white_castle_queenside = false;
        }

        // increment half move counter in case of capture or pawn move
        if(captured_piece_index != 12 || moved_piece_index == 5){
            state.half_move_counter = pos.half_move_counter;
            pos.half_move_counter = 0;
        }
        else{ pos.half_move_counter++; }
        // update bitboards
        pos.all_pieces = pos.white_pieces | pos.black_pieces;
        // update side to move
        pos.white_to_move = false;
    }

    // BLACK TO MOVE
    else{
        // retrieve what piece has moved
        for(uint8_t piece_index = 6; piece_index < 12; piece_index++){
            if(pos.pieces[piece_index] & from_bb){
                moved_piece_index = piece_index;
                break;
            }
        }
        // retrieve info about captured piece (if any)
        for(uint8_t piece_index = 0; piece_index < 6; piece_index++){
            // standard capture
            if(pos.pieces[piece_index] & to_bb){
                captured_piece_index = piece_index;
                break;
            } 
            // en-passant capture
            else if(flags == 5){
                captured_piece_index = 5; // <--- necessarily a pawn
                break;
            }
        }
        // update en passant target
        if(flags == 1){ // double pawn push
            state.en_passant_target_square = pos.en_passant_target_square;
            pos.en_passant_target_square = 1ULL << (to - 8);
        }
        else{
            pos.en_passant_target_square = 0ULL;
        }
        // retrieve info about promoted piece (if any)
        if(flags == 11 || flags == 15){ promoted_piece_index = 7; } // queen
        else if(flags == 10 || flags == 14){ promoted_piece_index = 8; } // rook
        else if(flags == 9 || flags == 13){ promoted_piece_index = 9; } // bishop
        else if(flags == 8 || flags == 12){ promoted_piece_index = 10; } // knight
        // memorize current state
        state.moved_piece_index = moved_piece_index;
        state.captured_piece_index = captured_piece_index;
        state.promoted_piece_index = promoted_piece_index;
        // remove the piece from the starting square
        pos.pieces[moved_piece_index] ^= from_bb | to_bb;
        pos.black_pieces ^= from_bb | to_bb;
        // remove captured piece, if any
        if(captured_piece_index != 12){
            // if en passant, piece is not in the target square
            if(flags == 5){
                bit_clear_opt(pos.pieces[5], to - 8);
                bit_clear_opt(pos.white_pieces, to - 8);
            }
            else{
                bit_clear_opt(pos.pieces[captured_piece_index], to);
                bit_clear_opt(pos.white_pieces, to);
            }
        }
        // spawn the promoted piece in case of promotion and remove the pawn
        if(promoted_piece_index != 12){
            bit_clear_opt(pos.pieces[11], to);
            bit_set_opt(pos.pieces[promoted_piece_index], to);
        }
        // Handle castling
        if(flags == 2){// kingside castle
            // transfer the rook
            pos.pieces[8] ^= BLACK_ROOK_KINGSIDE_CASTLE_MASK;
            pos.black_pieces ^= BLACK_ROOK_KINGSIDE_CASTLE_MASK;
        }
        else if(flags == 3){// queenside castle
            // transfer the rook
            pos.pieces[8] ^= BLACK_ROOK_QUEENSIDE_CASTLE_MASK;
            pos.black_pieces ^= BLACK_ROOK_QUEENSIDE_CASTLE_MASK;
        }
        // CASTLING RIGHTS
        // save current castling rights 
        state.can_black_castle_kingside = pos.can_black_castle_kingside;
        state.can_black_castle_queenside = pos.can_black_castle_queenside;
        // loose castling rights if current move is castling or king move 
        if(flags == 2 || flags == 3 || moved_piece_index == 6){
            pos.can_black_castle_kingside = false;
            pos.can_black_castle_queenside = false;
        }
        // if you move the rook on h8 
        if(moved_piece_index == 8 && from == 7){
            pos.can_black_castle_kingside = false;
        }
        // if you move the rook on a8
        else if(moved_piece_index == 8 && from == 0){
            pos.can_black_castle_queenside = false;
        }
        // Reset half move counter in case of capture or pawn move
        if(captured_piece_index != 12 || moved_piece_index == 11){
            state.half_move_counter = pos.half_move_counter;
            pos.half_move_counter = 0;
        }
        // ...or else increment it 
        else{ pos.half_move_counter++; }
        // update bitboards
        pos.all_pieces = pos.white_pieces | pos.black_pieces;
        // update side to move
        pos.white_to_move = true;
    }
} 

void UnmakeMove(Position& pos, const Move& move, const StateMemory& state){
    uint8_t from, to, flags;
    from = move & 0b00111111;
    to = (move >> 6) & 0b00111111;
    flags = (move >> 12);
    // reposition the moved piece
    bit_clear_opt(pos.pieces[state.moved_piece_index], to);
    bit_set_opt(pos.pieces[state.moved_piece_index], from);

    // black made the pseudomove
    if(pos.white_to_move){
        pos.black_pieces ^= (1ULL << from) | (1ULL << to);
        // reposition the captured piece
        if(state.captured_piece_index != 12){
            // if capture is en passant, respawn the white pawn in the correct position
            if(flags == 5){
                bit_set_opt(pos.pieces[5], to - 8);
                bit_set_opt(pos.white_pieces, to - 8);
            }
            // for a normal capture, respawn the piece in the target square of the move
            else{
                bit_set_opt(pos.pieces[state.captured_piece_index], to);
                bit_set_opt(pos.white_pieces, to);
            }
        }
        // remove promoted piece and restore the pawn 
        if(state.promoted_piece_index != 12){
            bit_clear_opt(pos.pieces[state.promoted_piece_index], to);
            bit_set_opt(pos.pieces[11], from);
        }
        // in case of castling, reposition the rook correctly
        if(flags == 2){ // kingside
            pos.pieces[8] ^= BLACK_ROOK_KINGSIDE_CASTLE_MASK;
            pos.black_pieces ^= BLACK_ROOK_KINGSIDE_CASTLE_MASK;
        }
        else if(flags == 3){ // queenside
            pos.pieces[8] ^= BLACK_ROOK_QUEENSIDE_CASTLE_MASK;
            pos.black_pieces ^= BLACK_ROOK_QUEENSIDE_CASTLE_MASK;
        }
        // restore en-passant target square
        else if(flags == 1){ // move was a double pawn push
            pos.en_passant_target_square = state.en_passant_target_square;
        }
        // restore group bitboards
        pos.all_pieces = pos.white_pieces | pos.black_pieces;
        // restore previous castling rights
        pos.can_black_castle_kingside = state.can_black_castle_kingside;
        pos.can_black_castle_queenside = state.can_black_castle_queenside;
        // restore half-move counter
        if(state.captured_piece_index != 12 || state.moved_piece_index == 11){
            pos.half_move_counter = state.half_move_counter;
        }
        else{ pos.half_move_counter--; }
        // update side to move
        pos.white_to_move = false;
    }

    // if white made the pseudomove 
    else{
        pos.white_pieces ^= (1ULL << from) | (1ULL << to);
        // reposition the captured piece
        if(state.captured_piece_index != 12){
            // if the capture was en-passant, the captured pawn does not respawn in the target square of the move
            if(flags == 5){
                bit_set_opt(pos.pieces[11], to + 8);
                bit_set_opt(pos.black_pieces, to + 8);
            }
            else{
                bit_set_opt(pos.pieces[state.captured_piece_index], to);
                bit_set_opt(pos.black_pieces, to);
            }
        }
        // remove promoted piece and restore the pawn 
        if(state.promoted_piece_index != 12){
            bit_clear_opt(pos.pieces[state.promoted_piece_index], to);
            bit_set_opt(pos.pieces[5], from);
        }
        // in case of castling, reposition the rook correctly
        if(flags == 2){ // kingside
            pos.pieces[2] ^= WHITE_ROOK_KINGSIDE_CASTLE_MASK;
            pos.white_pieces ^= WHITE_ROOK_KINGSIDE_CASTLE_MASK;
        }
        else if(flags == 3){// queenside
            pos.pieces[2] ^= WHITE_ROOK_QUEENSIDE_CASTLE_MASK;
            pos.white_pieces ^= WHITE_ROOK_QUEENSIDE_CASTLE_MASK;
        }
        // restore en-passant target
        else if(flags == 1){
            pos.en_passant_target_square = state.en_passant_target_square;
        }
        // restore group bitboards
        pos.all_pieces = pos.white_pieces | pos.black_pieces;
        // restore previous castling rights
        pos.can_white_castle_kingside = state.can_white_castle_kingside;
        pos.can_white_castle_queenside = state.can_white_castle_queenside;
        // restore half-move counter
        if(state.captured_piece_index != 12 || state.moved_piece_index == 5){
            pos.half_move_counter = state.half_move_counter;
        }
        else{ pos.half_move_counter--; }
        // update side to move
        pos.white_to_move = true;
    }
}

bool SquareIsAttacked(Position& pos, const unsigned long int square){
    uint64_t attacks; 
    MaskAndMagic mm;
    // if white to move: white is the attacker
    if(pos.white_to_move){
        // 1. check attacks from opponent's king 
        attacks = king_covered_squares_bitboards[square];
        if(attacks & pos.pieces[0]){ return true; }
        // 2: check attacks from  knight
        attacks = knight_covered_squares_bitboards[square];
        if(attacks & pos.pieces[4]){ return true; }
        // 3: check attacks from  pawns
        attacks = black_pawn_covered_squares_bitboards[square];
        if(attacks & pos.pieces[5]){ return true; }
        // 4: check attacks from diagonal directions
        mm = bishop_mm[square];
        attacks = bishop_covered_squares_bb[square][((pos.all_pieces & mm.mask) * mm.magic) >> shift_bishop];
        if(attacks & (pos.pieces[1] | pos.pieces[3])){ return true; }
        // 5: check attacks from horizontal or vertical directions
        mm = rook_mm[square];
        attacks = rook_covered_squares_bb[square][((pos.all_pieces & mm.mask) * mm.magic) >> shift_rook];
        if(attacks & (pos.pieces[1] | pos.pieces[2])){ return true; }
    }
    // else black is the attacker
    else{
        // 1. check attacks from opponent's king 
        attacks = king_covered_squares_bitboards[square];
        if(attacks & pos.pieces[6]){ return true; }
        // 2: check attacks from  knight
        attacks = knight_covered_squares_bitboards[square];
        if(attacks & pos.pieces[10]){ return true; }
        // 3: check attacks from  pawns
        attacks = white_pawn_covered_squares_bitboards[square];
        if(attacks & pos.pieces[11]){ return true; }
        // 4: check attacks from diagonal directions
        mm = bishop_mm[square];
        attacks = bishop_covered_squares_bb[square][((pos.all_pieces & mm.mask) * mm.magic) >> shift_bishop];
        if(attacks & (pos.pieces[7] | pos.pieces[9])){ return true; }
        // 5: check attacks from horizontal or vertical directions
        mm = rook_mm[square];
        attacks = rook_covered_squares_bb[square][((pos.all_pieces & mm.mask) * mm.magic) >> shift_rook];
        if(attacks & (pos.pieces[7] | pos.pieces[8])){ return true; }
    }
    // if survived till here...
    return false;
}


bool IsLegal(Position& pos, const Move& move){ 
    uint64_t attacks;
    MaskAndMagic mm;
    unsigned long king_square;
    uint8_t flags = (move >> 12); 
    // if white to move and black's king is in check, pos is illegal
    if(pos.white_to_move){
        // step 1: get black's king position
        _BitScanForward64(&king_square, pos.pieces[6]);
        // step 2: check attacks from white king
        attacks = king_covered_squares_bitboards[king_square];
        if(attacks & pos.pieces[0]){ return false; }
        // step 3: check attacks from white knight
        attacks = knight_covered_squares_bitboards[king_square];
        if(attacks & pos.pieces[4]){ return false; }
        // step 4: check attacks from white pawns
        attacks = black_pawn_covered_squares_bitboards[king_square];
        if(attacks & pos.pieces[5]){ return false; }
        // step 5: check attacks from diagonal directions
        mm = bishop_mm[king_square];
        attacks = bishop_covered_squares_bb[king_square][((pos.all_pieces & mm.mask) * mm.magic) >> shift_bishop];
        if(attacks & (pos.pieces[1] | pos.pieces[3])){ return false; }
        // step 6: check attacks from horizontal or vertical directions
        mm = rook_mm[king_square];
        attacks = rook_covered_squares_bb[king_square][((pos.all_pieces & mm.mask) * mm.magic) >> shift_rook];
        if(attacks & (pos.pieces[1] | pos.pieces[2])){ return false; }
        // if black just castled (so now is white to move), control that the black king was not passing through a square covered by white
        if(flags == 2){
            /*pos.white_covered_squares = GetCoveredSquares(pos.pieces, pos.all_pieces, true);
            if(pos.white_covered_squares & BLACK_KINGSIDE_CASTLE_MASK){
                return false;
            }*/
            if(63624ULL & pos.pieces[0]){ return false; }
            if(16309248ULL & pos.pieces[4]){ return false; }
            if(63488ULL & pos.pieces[5]){ return false; }
            mm = bishop_mm[6];
            attacks = bishop_covered_squares_bb[6][((pos.all_pieces & mm.mask) * mm.magic) >> shift_bishop];
            if(attacks & (pos.pieces[1] | pos.pieces[3])){ return false; }
            mm = rook_mm[6];
            attacks = rook_covered_squares_bb[6][((pos.all_pieces & mm.mask) * mm.magic) >> shift_rook];
            if(attacks & (pos.pieces[1] | pos.pieces[2])){ return false; }

            mm = bishop_mm[5];
            attacks = bishop_covered_squares_bb[5][((pos.all_pieces & mm.mask) * mm.magic) >> shift_bishop];
            if(attacks & (pos.pieces[1] | pos.pieces[3])){ return false; }
            mm = rook_mm[5];
            attacks = rook_covered_squares_bb[5][((pos.all_pieces & mm.mask) * mm.magic) >> shift_rook];
            if(attacks & (pos.pieces[1] | pos.pieces[2])){ return false; }

            mm = bishop_mm[4];
            attacks = bishop_covered_squares_bb[4][((pos.all_pieces & mm.mask) * mm.magic) >> shift_bishop];
            if(attacks & (pos.pieces[1] | pos.pieces[3])){ return false; }
            mm = rook_mm[4];
            attacks = rook_covered_squares_bb[4][((pos.all_pieces & mm.mask) * mm.magic) >> shift_rook];
            if(attacks & (pos.pieces[1] | pos.pieces[2])){ return false; }
        }
        else if(flags == 3){
            /*pos.white_covered_squares = GetCoveredSquares(pos.pieces, pos.all_pieces, true);
            if(pos.white_covered_squares & BLACK_QUEENSIDE_CASTLE_MASK){
                return false;
            }*/
            if(15906ULL & pos.pieces[0]){ return false; }
            if(4093696ULL & pos.pieces[4]){ return false; }
            if(15872ULL & pos.pieces[5]){ return false; }
            mm = bishop_mm[2];
            attacks = bishop_covered_squares_bb[2][((pos.all_pieces & mm.mask) * mm.magic) >> shift_bishop];
            if(attacks & (pos.pieces[1] | pos.pieces[3])){ return false; }
            mm = rook_mm[2];
            attacks = rook_covered_squares_bb[2][((pos.all_pieces & mm.mask) * mm.magic) >> shift_rook];
            if(attacks & (pos.pieces[1] | pos.pieces[2])){ return false; }

            mm = bishop_mm[3];
            attacks = bishop_covered_squares_bb[3][((pos.all_pieces & mm.mask) * mm.magic) >> shift_bishop];
            if(attacks & (pos.pieces[1] | pos.pieces[3])){ return false; }
            mm = rook_mm[3];
            attacks = rook_covered_squares_bb[3][((pos.all_pieces & mm.mask) * mm.magic) >> shift_rook];
            if(attacks & (pos.pieces[1] | pos.pieces[2])){ return false; }

            mm = bishop_mm[4];
            attacks = bishop_covered_squares_bb[4][((pos.all_pieces & mm.mask) * mm.magic) >> shift_bishop];
            if(attacks & (pos.pieces[1] | pos.pieces[3])){ return false; }
            mm = rook_mm[4];
            attacks = rook_covered_squares_bb[4][((pos.all_pieces & mm.mask) * mm.magic) >> shift_rook];
            if(attacks & (pos.pieces[1] | pos.pieces[2])){ return false; }
        }
        // if all the previous legality checks are passed, return true
        return true;
    }
    // if black to move and white's king is in check, pos is illegal
    else{
        // step 1: get black's king position
        _BitScanForward64(&king_square, pos.pieces[0]);
        // step 2: check attacks from white king
        attacks = king_covered_squares_bitboards[king_square];
        if(attacks & pos.pieces[6]){ return false; }
        // step 3: check attacks from white knight
        attacks = knight_covered_squares_bitboards[king_square];
        if(attacks & pos.pieces[10]){ return false; }
        // step 4: check attacks from white pawns
        attacks = white_pawn_covered_squares_bitboards[king_square];
        if(attacks & pos.pieces[11]){ return false; }
        // step 5: check attacks from diagonal directions
        mm = bishop_mm[king_square];
        attacks = bishop_covered_squares_bb[king_square][((pos.all_pieces & mm.mask) * mm.magic) >> shift_bishop];
        if(attacks & (pos.pieces[7] | pos.pieces[9])){ return false; }
        // step 6: check attacks from horizontal or vertical directions
        mm = rook_mm[king_square];
        attacks = rook_covered_squares_bb[king_square][((pos.all_pieces & mm.mask) * mm.magic) >> shift_rook];
        if(attacks & (pos.pieces[7] | pos.pieces[8])){ return false; }
        if(flags == 2){
            pos.black_covered_squares = GetCoveredSquares(pos.pieces, pos.all_pieces, false);
            if(pos.black_covered_squares & WHITE_KINGSIDE_CASTLE_MASK){
                return false;
            }
        }
        else if(flags == 3){
            pos.black_covered_squares = GetCoveredSquares(pos.pieces, pos.all_pieces, false);
            if(pos.black_covered_squares & WHITE_QUEENSIDE_CASTLE_MASK){
                return false;
            }
        }
        return true;
    }
}