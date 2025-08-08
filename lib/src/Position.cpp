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
/*    int score = pos.white_material_value + pos.black_material_value;
    
    // loop over the pieces to add extra value based on the position of the piece
    unsigned long square;
    uint64_t piece;
    // white king
    piece = pos.pieces[0];
    if(piece){
        _BitScanForward64(&square, piece);
        if(pos.black_material_value < -2000){ // very rough logic to distinguish middlegame fron endgame
            score += kingPST_Middlegame[square];
        }
        else{
            score += kingPST_Endgame[square];
        }
    }
    // white queen
    piece = pos.pieces[1];
    while(piece){ // loop until all the white queens are considered
        _BitScanForward64(&square, piece);
        score += queenPST[square];
        clear_last_active_bit(piece);   
    }
    // white rook
    piece = pos.pieces[2];
    while(piece){ // loop until all the white queens are considered
        _BitScanForward64(&square, piece);
        score += rookPST[square];
        clear_last_active_bit(piece);   
    }
    // white bishop
    piece = pos.pieces[3];
    while(piece){ // loop until all the white queens are considered
        _BitScanForward64(&square, piece);
        score += bishopPST[square];
        clear_last_active_bit(piece);   
    }
    // white knight
    piece = pos.pieces[4];
    while(piece){ // loop until all the white queens are considered
        _BitScanForward64(&square, piece);
        score += knightPST[square];
        clear_last_active_bit(piece);   
        // bonus for outpost squares ...
    }
    // white pawns
    piece = pos.pieces[5];
    while(piece){ // loop until all the white queens are considered
        _BitScanForward64(&square, piece);
        score += white_pawnPST[square];
        clear_last_active_bit(piece);   
        // bonus for passed pawns ...
        if(mask_white_passed_pawn[square] & pos.pieces[11]){ continue; }
        else{
            score += BONUS_FOR_PASSED_PAWNS;
        }
    }
    // black king
    piece = pos.pieces[6];
    if(piece){
        _BitScanForward64(&square, piece);
        if(pos.white_material_value > 2000){ // very rough logic to distinguish middlegame fron endgame
            score -= kingPST_Middlegame[56 - square + 2*(square%8)];
        }
        else{
            score -= kingPST_Endgame[56 - square + 2*(square%8)];
        }
    }
    // black queen
    piece = pos.pieces[7];
    while(piece){ // loop until all the white queens are considered
        _BitScanForward64(&square, piece);
        score -= queenPST[56 - square + 2*(square%8)];
        clear_last_active_bit(piece);   
    }
    // black rook
    piece = pos.pieces[8];
    while(piece){ // loop until all the white queens are considered
        _BitScanForward64(&square, piece);
        score -= rookPST[56 - square + 2*(square%8)];
        clear_last_active_bit(piece);   
    }
    // black bishop
    piece = pos.pieces[9];
    while(piece){ // loop until all the white queens are considered
        _BitScanForward64(&square, piece);
        score -= bishopPST[56 - square + 2*(square%8)];
        clear_last_active_bit(piece);   
    }
    // black knight
    piece = pos.pieces[10];
    while(piece){ // loop until all the white queens are considered
        _BitScanForward64(&square, piece);
        score -= knightPST[56 - square + 2*(square%8)];
        clear_last_active_bit(piece);   
        // bonus for outpost ...
    }
    // black pawns
    piece = pos.pieces[11];
    while(piece){ // loop until all the white queens are considered
        _BitScanForward64(&square, piece);
        score -= black_pawnPST[square];
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
*/
    return 0;
}

void PseudoLegalMoves(const Position& pos, MoveNew* moves){
    uint8_t move_index = 0;
    uint64_t piece, hash_index_rook, hash_index_bishop; 
    unsigned long square, target_square;
    uint64_t attacks;
    bool is_capture;
    uint16_t flags;

    // WHITE MOVES
    if(pos.white_to_move){

        // King
        piece = pos.pieces[0];
        while(piece){
            // find position of piece and assign it to square
            _BitScanForward64(&square, piece); 
            attacks = king_covered_squares_bitboards[square]; // retrieve attack bitboard
            attacks &= ~pos.white_pieces; // exclude self-capture
            while(attacks){
                _BitScanForward64(&target_square, attacks); // find the target square
                is_capture = bit_get(pos.black_pieces, target_square);
                is_capture ? flags = 4 : flags = 0;
                // save the move
                moves[move_index] = EncodeMoveNew(square, target_square, flags);
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
            hash_index_rook = rook_hash_index(pos.all_pieces, square, n_attacks_rook);
            hash_index_bishop = bishop_hash_index(pos.all_pieces, square, n_attacks_bishop);
            attacks = rook_covered_squares_bitboards[hash_index_rook] | bishop_covered_squares_bitboards[hash_index_bishop];
            attacks &= ~pos.white_pieces; // excluse self-capture
            while(attacks){
                _BitScanForward64(&target_square, attacks); // find the target square
                is_capture = bit_get(pos.black_pieces, target_square);
                is_capture ? flags = 4 : flags = 0;
                moves[move_index] = EncodeMoveNew(square, target_square, flags);
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
            hash_index_rook = rook_hash_index(pos.all_pieces, square, n_attacks_rook);
            attacks = rook_covered_squares_bitboards[hash_index_rook];
            attacks &= ~pos.white_pieces; // excluse self-capture
            while(attacks){
                _BitScanForward64(&target_square, attacks); // find the target square
                is_capture = bit_get(pos.black_pieces, target_square);
                is_capture ? flags = 4 : flags = 0;
                moves[move_index] = EncodeMoveNew(square, target_square, flags);
                move_index++;
                clear_last_active_bit(attacks); 
            }
            clear_last_active_bit(piece);
        }

        // Bishop
        piece = pos.pieces[3]; // retrieve bitboard of queens
        while(piece){ // consider all the queens
            _BitScanForward64(&square, piece); // find position of queen and assign it to square
            hash_index_bishop = bishop_hash_index(pos.all_pieces, square, n_attacks_bishop);
            attacks = bishop_covered_squares_bitboards[hash_index_bishop];
            attacks &= ~pos.white_pieces; // excluse self-capture
            while(attacks){
                _BitScanForward64(&target_square, attacks); // find the target square
                is_capture = bit_get(pos.black_pieces, target_square);
                is_capture ? flags = 4 : flags = 0;
                moves[move_index] = EncodeMoveNew(square, target_square, flags);
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
            attacks &= ~pos.white_pieces; // exclude self-capture
            while(attacks){
                _BitScanForward64(&target_square, attacks); // find the target square
                is_capture = bit_get(pos.black_pieces, target_square);
                is_capture ? flags = 4 : flags = 0;
                moves[move_index] = EncodeMoveNew(square, target_square, flags);
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
                if(target_square / 8 == 0){
                    for(uint8_t promoted_piece_index = 1; promoted_piece_index < 5; promoted_piece_index++){
                        flags = 16 - promoted_piece_index;
                        moves[move_index] = EncodeMoveNew(square, target_square, flags);
                        move_index++;
                    }
                }
                else{ 
                    flags = 4;
                    moves[move_index] = EncodeMoveNew(square, target_square, flags);
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
                moves[move_index] = EncodeMoveNew(square, target_square, flags);
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
            if((square/8 == 6) && !bit_get(attacks, square - 8) && bit_get(attacks, square - 16)){
                attacks = 0ULL;
            }
            while(attacks){
                _BitScanForward64(&target_square, attacks); // find the target square
                // in case of promotion, loop over all possible promoted pieces
                if(target_square / 8 == 0){
                    for(uint8_t promoted_piece_index = 1; promoted_piece_index < 5; promoted_piece_index++){
                        flags = 12 - promoted_piece_index;
                        moves[move_index] = EncodeMoveNew(square, target_square, flags);
                        move_index++;
                    }
                }
                // if this is a double push, flag it to manage en-passant target squares later
                else if(target_square / 8 == 4 && square / 8 == 6){
                    flags = 1;
                    moves[move_index] = EncodeMoveNew(square, target_square, flags);
                    move_index++;
                }
                else{
                    flags = 0;
                    moves[move_index] = EncodeMoveNew(square, target_square, flags);
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
            if(!bit_get(pos.all_pieces, 61) && 
                !bit_get(pos.all_pieces, 62)){ // 2. the in-between squares are empty
                // 3. king does not pass through a square covered by opponent
                if(bit_get(pos.pieces[0], 60) &&
                    bit_get(pos.pieces[2], 63)){ // 4. king and rook are in the correct position
                    moves[move_index] = EncodeMoveNew(60, 62, 2); 
                    move_index++;
                } 
            }   
        }
        // Castles queenside
        if(pos.can_white_castle_queenside){ 
            if(!bit_get(pos.all_pieces, 59) && 
                !bit_get(pos.all_pieces, 58) &&
                !bit_get(pos.all_pieces, 57)){ 
                if(bit_get(pos.pieces[0], 60) &&
                    bit_get(pos.pieces[2], 56)){ 
                    moves[move_index] = EncodeMove(60, 58, 3); 
                    move_index++;
                } 
            }   
        }
    }

    // BLACK MOVES
    else{

        // King
        piece = pos.pieces[6];
        while(piece){
            // find position of piece and assign it to square
            _BitScanForward64(&square, piece); 
            attacks = king_covered_squares_bitboards[square]; // retrieve attack bitboard
            attacks &= ~pos.black_pieces; // exclude self-capture
            while(attacks){
                _BitScanForward64(&target_square, attacks); // find the target square
                is_capture = bit_get(pos.white_pieces, target_square);
                is_capture ? flags = 4 : flags = 0;
                // save the move
                moves[move_index] = EncodeMoveNew(square, target_square, flags);
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
            hash_index_rook = rook_hash_index(pos.all_pieces, square, n_attacks_rook);
            hash_index_bishop = bishop_hash_index(pos.all_pieces, square, n_attacks_bishop);
            attacks = rook_covered_squares_bitboards[hash_index_rook] | bishop_covered_squares_bitboards[hash_index_bishop];
            attacks &= ~pos.black_pieces; // excluse self-capture
            while(attacks){
                _BitScanForward64(&target_square, attacks); // find the target square
                is_capture = bit_get(pos.white_pieces, target_square);
                is_capture ? flags = 4 : flags = 0;
                moves[move_index] = EncodeMoveNew(square, target_square, flags);
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
            hash_index_rook = rook_hash_index(pos.all_pieces, square, n_attacks_rook);
            attacks = rook_covered_squares_bitboards[hash_index_rook];
            attacks &= ~pos.black_pieces; // excluse self-capture
            while(attacks){
                _BitScanForward64(&target_square, attacks); // find the target square
                is_capture = bit_get(pos.white_pieces, target_square);
                is_capture ? flags = 4 : flags = 0;
                moves[move_index] = EncodeMoveNew(square, target_square, flags);
                move_index++;
                clear_last_active_bit(attacks); 
            }
            clear_last_active_bit(piece);
        }

        // Bishop
        piece = pos.pieces[9]; // retrieve bitboard of queens
        while(piece){ // consider all the queens
            _BitScanForward64(&square, piece); // find position of queen and assign it to square
            hash_index_bishop = bishop_hash_index(pos.all_pieces, square, n_attacks_bishop);
            attacks = bishop_covered_squares_bitboards[hash_index_bishop];
            attacks &= ~pos.black_pieces; // excluse self-capture
            while(attacks){
                _BitScanForward64(&target_square, attacks); // find the target square
                is_capture = bit_get(pos.white_pieces, target_square);
                is_capture ? flags = 4 : flags = 0;
                moves[move_index] = EncodeMoveNew(square, target_square, flags);
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
            attacks &= ~pos.black_pieces; // exclude self-capture
            while(attacks){
                _BitScanForward64(&target_square, attacks); // find the target square
                is_capture = bit_get(pos.white_pieces, target_square);
                is_capture ? flags = 4 : flags = 0;
                moves[move_index] = EncodeMoveNew(square, target_square, flags);
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
                if(target_square / 8 == 7){
                    for(uint8_t promoted_piece_index = 7; promoted_piece_index < 11; promoted_piece_index++){
                        flags = 22 - promoted_piece_index;
                        moves[move_index] = EncodeMoveNew(square, target_square, flags);
                        move_index++;
                    }
                }
                else{ 
                    flags = 4;
                    moves[move_index] = EncodeMoveNew(square, target_square, flags);
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
                moves[move_index] = EncodeMoveNew(square, target_square, flags);
                move_index++;
                clear_last_active_bit(attacks); // remove considered attack
            }

            // Pawn push
            attacks = black_pawn_advance_squares_bitboards[square]; // retrieve attack bitboard
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
            if((square/8 == 1) && !bit_get(attacks, square + 8) && bit_get(attacks, square + 16)){
                attacks = 0ULL;
            }
            while(attacks){
                _BitScanForward64(&target_square, attacks); // find the target square
                // in case of promotion, loop over all possible promoted pieces
                if(target_square / 8 == 7){
                    for(uint8_t promoted_piece_index = 7; promoted_piece_index < 11; promoted_piece_index++){
                        flags = 22 - promoted_piece_index;
                        moves[move_index] = EncodeMoveNew(square, target_square, flags);
                        move_index++;
                    }
                }
                // if this is a double push, flag it to manage en-passant target squares later
                else if(target_square / 8 == 3 && square / 8 == 1){
                    flags = 1;
                    moves[move_index] = EncodeMoveNew(square, target_square, flags);
                    move_index++;
                }
                else{
                    flags = 0;
                    moves[move_index] = EncodeMoveNew(square, target_square, flags);
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
            if(!bit_get(pos.all_pieces, 5) && 
                !bit_get(pos.all_pieces, 6)){ 
                if(bit_get(pos.pieces[6], 4) &&
                    bit_get(pos.pieces[8], 7)){ 
                    moves[move_index] = EncodeMoveNew(4, 6, 2);
                    move_index++;
                } 
            }   
        }

        // Castles queenside
        if(pos.can_black_castle_queenside){ 
            if(!bit_get(pos.all_pieces, 3) && 
                !bit_get(pos.all_pieces, 2) &&
                !bit_get(pos.all_pieces, 1)){ 
                if(bit_get(pos.pieces[6], 4) &&
                    bit_get(pos.pieces[8], 0)){ 
                    moves[move_index] = EncodeMoveNew(4, 2, 3); 
                    move_index++;
                } 
            }   
        }
    }
}


void MakeMove(Position& pos, const MoveNew& move, StateMemory& state){
    uint8_t from, to, flags;
    uint8_t moved_piece_index = 0, captured_piece_index = 12, promoted_piece_index = 12; // 12 = no piece captured
    // retrieve info from the move
    from = move & 0b00111111;
    to = (move >> 6) & 0b00111111;
    flags = (move >> 12);
    // WHITE TO MOVE
    if(pos.white_to_move){
        // retrieve what piece has moved
        for(uint8_t piece_index = 0; piece_index < 6; piece_index++){
            if(bit_get_opt(pos.pieces[piece_index], from)){
                moved_piece_index = piece_index;
                break;
            }
        }
        // retrieve info about captured piece (if any)
        for(uint8_t piece_index = 6; piece_index < 12; piece_index++){
            // standard capture
            if(bit_get_opt(pos.pieces[piece_index], to)){
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
        state.en_passant_target_square = pos.en_passant_target_square;
        if(flags == 1){ // double pawn push
            pos.en_passant_target_square = 1ULL << (to + 8);
            //bit_set_opt(pos.en_passant_target_square, to + 8);
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
        state.moved_piece = pos.pieces[moved_piece_index]; // save before changing
        state.friendly_pieces = pos.white_pieces;
        state.enemy_pieces = pos.black_pieces;
        state.captured_piece = pos.pieces[captured_piece_index];
        // remove the piece from the starting square
        bit_clear_opt(pos.pieces[moved_piece_index], from);
        bit_clear_opt(pos.white_pieces, from);
        // spawn the moved piece on the target square
        bit_set_opt(pos.pieces[moved_piece_index], to);
        bit_set_opt(pos.white_pieces, to);
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
            bit_clear_opt(pos.pieces[2], 63); bit_set_opt(pos.pieces[2], 61);
            bit_clear_opt(pos.white_pieces, 63); bit_set_opt(pos.white_pieces, 61);
        }
        else if(flags == 3){// queenside castle
            // transfer the rook
            bit_clear_opt(pos.pieces[2], 56); bit_set_opt(pos.pieces[2], 59);
            bit_clear_opt(pos.white_pieces, 56); bit_set_opt(pos.white_pieces, 59);
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
            if(bit_get_opt(pos.pieces[piece_index], from)){
                moved_piece_index = piece_index;
                break;
            }
        }
        // retrieve info about captured piece (if any)
        for(uint8_t piece_index = 0; piece_index < 6; piece_index++){
            // standard capture
            if(bit_get_opt(pos.pieces[piece_index], to)){
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
        state.en_passant_target_square = pos.en_passant_target_square;
        if(flags == 1){ // double pawn push
            pos.en_passant_target_square = 1ULL << (to - 8);
            //bit_set_opt(pos.en_passant_target_square, to - 8);
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
        state.moved_piece = pos.pieces[moved_piece_index]; // save before changing
        state.friendly_pieces = pos.black_pieces;
        state.enemy_pieces = pos.white_pieces;
        state.captured_piece = pos.pieces[captured_piece_index];
        // remove the piece from the starting square
        bit_clear_opt(pos.pieces[moved_piece_index], from);
        bit_clear_opt(pos.black_pieces, from);
        // spawn the moved piece on the target square
        bit_set_opt(pos.pieces[moved_piece_index], to);
        bit_set_opt(pos.black_pieces, to);
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
            bit_clear_opt(pos.pieces[8], 7); bit_set_opt(pos.pieces[8], 5);
            bit_clear_opt(pos.black_pieces, 7); bit_set_opt(pos.black_pieces, 5);
        }
        else if(flags == 3){// queenside castle
            // transfer the rook
            bit_clear_opt(pos.pieces[8], 0); bit_set_opt(pos.pieces[8], 3);
            bit_clear_opt(pos.black_pieces, 0); bit_set_opt(pos.black_pieces, 3);
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
        if(moved_piece_index == 8 && from == 0){
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

void UnmakeMove(Position& pos, const MoveNew& move, const StateMemory& state){
    uint8_t from, to, flags;
    uint8_t promoted_piece_index = 12;
    from = move & 0b00111111;
    to = (move >> 6) & 0b00111111;
    flags = (move >> 12);

    // black made the pseudomove
    if(pos.white_to_move){
        // reposition the moved piece
        pos.pieces[state.moved_piece_index] = state.moved_piece;
        // reposition the captured piece
        if(state.captured_piece_index != 12){
            pos.pieces[state.captured_piece_index] = state.captured_piece;
        }
        // remove promoted piece and restore the pawn 
        if(flags == 11 || flags == 15){ promoted_piece_index = 7; } // queen
        else if(flags == 10 || flags == 14){ promoted_piece_index = 8; } // rook
        else if(flags == 9 || flags == 13){ promoted_piece_index = 9; } // bishop
        else if(flags == 8 || flags == 12){ promoted_piece_index = 10; } // knight
        if(promoted_piece_index != 12){
            bit_clear_opt(pos.pieces[promoted_piece_index], to);
            bit_set_opt(pos.pieces[11], from);
        }
        // in case of castling, reposition the rook correctly
        if(flags == 2){ // kingside
            bit_clear_opt(pos.pieces[8], 5);
            bit_set_opt(pos.pieces[8], 7);
        }
        else if(flags == 3){ // queenside
            bit_clear_opt(pos.pieces[8], 3);
            bit_set_opt(pos.pieces[8], 0);
        }
        // restore group bitboards
        pos.white_pieces = state.enemy_pieces;
        pos.black_pieces = state.friendly_pieces;
        pos.all_pieces = pos.white_pieces | pos.black_pieces;
        // restore previous castling rights
        pos.can_black_castle_kingside = state.can_black_castle_kingside;
        pos.can_black_castle_queenside = state.can_black_castle_queenside;
        // restore half-move counter
        if(state.captured_piece_index != 12 || state.moved_piece_index == 11){
            pos.half_move_counter = state.half_move_counter;
        }
        else{ pos.half_move_counter--; }
        // restore en-passant target square
        pos.en_passant_target_square = state.en_passant_target_square;
        // update side to move
        pos.white_to_move = false;
    }

    // if white made the pseudomove 
    else{
        // reposition the moved piece
        pos.pieces[state.moved_piece_index] = state.moved_piece;
        // reposition the captured piece
        if(state.captured_piece_index != 12){
            pos.pieces[state.captured_piece_index] = state.captured_piece;
        }
        // remove promoted piece and restore the pawn 
        if(flags == 11 || flags == 15){ promoted_piece_index = 1; } // queen
        else if(flags == 10 || flags == 14){ promoted_piece_index = 2; } // rook
        else if(flags == 9 || flags == 13){ promoted_piece_index = 3; } // bishop
        else if(flags == 8 || flags == 12){ promoted_piece_index = 4; } // knight
        if(promoted_piece_index != 12){
            bit_clear_opt(pos.pieces[promoted_piece_index], to);
            bit_set_opt(pos.pieces[5], from);
        }
        // in case of castling, reposition the rook correctly
        if(flags == 2){ // kingside
            bit_clear_opt(pos.pieces[2], 61);
            bit_set_opt(pos.pieces[2], 63);
        }
        else if(flags == 3){// queenside
            bit_clear_opt(pos.pieces[2], 59);
            bit_set_opt(pos.pieces[2], 56);
        }
        // restore group bitboards
        pos.white_pieces = state.friendly_pieces;
        pos.black_pieces = state.enemy_pieces;
        pos.all_pieces = pos.white_pieces | pos.black_pieces;
        // restore previous castling rights
        pos.can_white_castle_kingside = state.can_white_castle_kingside;
        pos.can_white_castle_queenside = state.can_white_castle_queenside;
        // restore half-move counter
        if(state.captured_piece_index != 12 || state.moved_piece_index == 5){
            pos.half_move_counter = state.half_move_counter;
        }
        else{ pos.half_move_counter--; }
        // restore en-passant target
        pos.en_passant_target_square = state.en_passant_target_square;
        // update side to move
        pos.white_to_move = true;
    }
}

bool IsLegal(Position& pos, const Move& move){ 
    uint64_t is_illegal;
    uint8_t flags = (move >> 12);
    // if white to move and black's king is in check, pos is illegal
    if(pos.white_to_move){
        pos.white_covered_squares = GetCoveredSquares(pos.pieces, pos.all_pieces, true);
        is_illegal = pos.pieces[6] & pos.white_covered_squares;
        if(is_illegal){ return false; }
        // if black just castled (so now is white to move), control that the black king was not passing through a square covered by white
        if(flags == 2){
            if(bit_get(pos.white_covered_squares, 4) ||
                bit_get(pos.white_covered_squares, 5) ||
                bit_get(pos.white_covered_squares, 6)){
                    return false;
            }
        }
        else if(flags == 3){
            if(bit_get(pos.white_covered_squares, 2) ||
                bit_get(pos.white_covered_squares, 3) ||
                bit_get(pos.white_covered_squares, 4)){
                    return false;
            }
        }
        return true;
    }
    // if black to move and white's king is in check, pos is illegal
    else{
        pos.black_covered_squares = GetCoveredSquares(pos.pieces, pos.all_pieces, false);
        is_illegal = pos.pieces[0] & pos.black_covered_squares;
        if(is_illegal){ return false; }
        if(flags == 2){
            if(bit_get(pos.black_covered_squares, 60) ||
                bit_get(pos.black_covered_squares, 61) ||
                bit_get(pos.black_covered_squares, 62)){
                    return false;
            }
        }
        else if(flags == 3){
            if(bit_get(pos.black_covered_squares, 60) ||
                bit_get(pos.black_covered_squares, 59) ||
                bit_get(pos.black_covered_squares, 58)){
                    return false;
            }
        }
        return true;
    }
}

// FIND LIST OF LEGAL MOVES
void LegalMoves(Position& pos, MoveAndPosition* all_moves){
    size_t move_index = 0;
    bool is_capture, is_illegal;
    uint64_t is_check;
    uint64_t piece, hash_index_rook, hash_index_bishop;
    uint64_t attacks = 0ULL;
    unsigned long square, target_square;
    uint8_t flags = 0;
    uint8_t piece_index, captured_piece_index;
    MoveAndPosition m;
    m.score = 0;

    if((pos.pieces[0] | pos.pieces[6]) == 0){
        pos.n_legal_moves = 0;
        return; 
    }

    // WHITE TO MOVE
    if(pos.white_to_move){

        // King
        piece_index = 0;
        piece = pos.pieces[piece_index]; // retrieve bitboard of king
        while(piece){ 
            _BitScanForward64(&square, piece); // find position of piece and assign it to square
            attacks = king_covered_squares_bitboards[square]; // retrieve attack bitboard
            attacks &= ~pos.white_pieces; // exclude self-capture
            while(attacks){
                flags = 0;
                _BitScanForward64(&target_square, attacks); // find the target square
                is_capture = bit_get(pos.black_pieces, target_square);
                captured_piece_index = 15; // no capture (default)
                // Generate new position applying the move
                m.position = pos;
                // move the piece in white pieces bitboards
                bit_clear(m.position.pieces[piece_index], square); bit_set(m.position.pieces[piece_index], target_square);
                bit_clear(m.position.white_pieces, square); bit_set(m.position.white_pieces, target_square);
                // if capture, also change bitboards of black pieces and recompute values
                if(is_capture){
                    flags += 8; // capture flag
                    // check what black piece has been captured
                    for(uint8_t index = 6; index < 12; index++){
                        if(bit_get(pos.pieces[index], target_square)){
                            captured_piece_index = index;
                            break;
                        }
                    }
                    //m.position.black_material_value -= PIECES_VALUES[captured_piece_index];
                    m.position.half_move_counter = 0;
                    bit_clear(m.position.pieces[captured_piece_index], target_square);
                    bit_clear(m.position.black_pieces, target_square);
                }
                else{ m.position.half_move_counter++; }
                // compute all pieces bitboard and the covered squares
                m.position.all_pieces = m.position.white_pieces | m.position.black_pieces;
                m.position.white_covered_squares = GetCoveredSquares(m.position.pieces, m.position.all_pieces, true);
                m.position.black_covered_squares = GetCoveredSquares(m.position.pieces, m.position.all_pieces, false);
                // check if move is legal
                is_illegal = m.position.pieces[0] & m.position.black_covered_squares;
                if(is_illegal){ clear_last_active_bit(attacks); continue; }
                // castling rights are lost
                m.position.can_white_castle_kingside = false;
                m.position.can_white_castle_queenside = false;
                // standard updates
                m.position.white_to_move = false;
                m.position.en_passant_target_square = 0ULL;
                // check if the move is a check
                is_check = m.position.pieces[6] & m.position.white_covered_squares;
                if(is_check){ flags += 16; }
                // encode move if legal
                m.move = EncodeMove(static_cast<uint8_t>(square), static_cast<uint8_t>(target_square), piece_index, captured_piece_index, 15/*no promotion*/, flags);
                all_moves[move_index] = m;
                move_index++;
                // remove considered attack
                clear_last_active_bit(attacks); 
            }
            // remove considered piece
            clear_last_active_bit(piece);
        }

        // Queen
        piece_index = 1;
        piece = pos.pieces[piece_index]; // retrieve bitboard of queens
        while(piece){ // consider all the queens
            _BitScanForward64(&square, piece); // find position of queen and assign it to square
            // retrieve bitboard of queen moves
            hash_index_rook = rook_hash_index(pos.all_pieces, square, n_attacks_rook);
            hash_index_bishop = bishop_hash_index(pos.all_pieces, square, n_attacks_bishop);
            attacks = rook_covered_squares_bitboards[hash_index_rook] | bishop_covered_squares_bitboards[hash_index_bishop];
            attacks &= ~pos.white_pieces; // excluse self-capture
            // loop over all the attacks
            while(attacks){
                flags = 0;
                _BitScanForward64(&target_square, attacks); // find the target square
                is_capture = bit_get(pos.black_pieces, target_square);
                captured_piece_index = 15; // no capture 
                // Generate new position applying the move
                m.position = pos;
                // move the piece in white pieces bitboards
                bit_clear(m.position.pieces[piece_index], square); bit_set(m.position.pieces[piece_index], target_square);
                bit_clear(m.position.white_pieces, square); bit_set(m.position.white_pieces, target_square);
                // if capture, also change bitboards of black pieces and recompute values
                if(is_capture){
                    flags += 8; // capture flag
                    // check what black piece has been captured
                    for(uint8_t index = 6; index < 12; index++){
                        if(bit_get(pos.pieces[index], target_square)){
                            captured_piece_index = index;
                            break;
                        }
                    }
                    //m.position.black_material_value -= PIECES_VALUES[captured_piece_index];
                    m.position.half_move_counter = 0;
                    bit_clear(m.position.pieces[captured_piece_index], target_square);
                    bit_clear(m.position.black_pieces, target_square);
                }
                else{ m.position.half_move_counter++; }
                // compute all pieces bitboard and the covered squares
                m.position.all_pieces = m.position.white_pieces | m.position.black_pieces;
                m.position.white_covered_squares = GetCoveredSquares(m.position.pieces, m.position.all_pieces, true);
                m.position.black_covered_squares = GetCoveredSquares(m.position.pieces, m.position.all_pieces, false);
                // check if move is legal
                is_illegal = m.position.pieces[0] & m.position.black_covered_squares;
                if(is_illegal){ clear_last_active_bit(attacks); continue; }
                // standard updates
                m.position.white_to_move = false;
                m.position.en_passant_target_square = 0ULL;
                // check if the move is a check
                is_check = m.position.pieces[6] & m.position.white_covered_squares;
                if(is_check){ flags += 16; }
                // encode move 
                m.move = EncodeMove(static_cast<uint8_t>(square), static_cast<uint8_t>(target_square), piece_index, captured_piece_index, 15/*no promotion*/, flags);
                all_moves[move_index] = m;
                move_index++;
                // remove considered attack
                clear_last_active_bit(attacks); 
            }
            // remove considered piece
            clear_last_active_bit(piece);
        }

        // Rook
        piece_index = 2;
        piece = pos.pieces[piece_index];
        while(piece){ // consider all the queens
            _BitScanForward64(&square, piece); // find position of rook and assign it to square
            // retrieve bitboard of rook moves
            hash_index_rook = rook_hash_index(pos.all_pieces, square, n_attacks_rook);
            attacks = rook_covered_squares_bitboards[hash_index_rook];
            attacks &= ~pos.white_pieces; // excluse self-capture
            // loop over all the attacks
            while(attacks){
                flags = 0;
                _BitScanForward64(&target_square, attacks); // find the target square
                is_capture = bit_get(pos.black_pieces, target_square);
                captured_piece_index = 15; // no capture 
                // Generate new position applying the move
                m.position = pos;
                // move the piece in white pieces bitboards
                bit_clear(m.position.pieces[piece_index], square); bit_set(m.position.pieces[piece_index], target_square);
                bit_clear(m.position.white_pieces, square); bit_set(m.position.white_pieces, target_square);
                // if capture, also change bitboards of black pieces and recompute values
                if(is_capture){
                    flags += 8; // capture flag
                    // check what black piece has been captured
                    for(uint8_t index = 6; index < 12; index++){
                        if(bit_get(pos.pieces[index], target_square)){
                            captured_piece_index = index;
                            break;
                        }
                    }
                    //m.position.black_material_value -= PIECES_VALUES[captured_piece_index];
                    m.position.half_move_counter = 0;
                    bit_clear(m.position.pieces[captured_piece_index], target_square);
                    bit_clear(m.position.black_pieces, target_square);
                }
                else{ m.position.half_move_counter++; }
                // compute all pieces bitboard and the covered squares
                m.position.all_pieces = m.position.white_pieces | m.position.black_pieces;
                m.position.white_covered_squares = GetCoveredSquares(m.position.pieces, m.position.all_pieces, true);
                m.position.black_covered_squares = GetCoveredSquares(m.position.pieces, m.position.all_pieces, false);
                // check if move is legal
                is_illegal = m.position.pieces[0] & m.position.black_covered_squares;
                if(is_illegal){ clear_last_active_bit(attacks); continue; }
                // castling rights are lost
                if(square == 63) { m.position.can_white_castle_kingside = false; }
                else if(square == 56) { m.position.can_white_castle_queenside = false; }
                // standard updates
                m.position.white_to_move = false;
                m.position.en_passant_target_square = 0ULL;
                // check if the move is a check
                is_check = m.position.pieces[6] & m.position.white_covered_squares;
                if(is_check){ flags += 16; }
                // encode move 
                m.move = EncodeMove(static_cast<uint8_t>(square), static_cast<uint8_t>(target_square), piece_index, captured_piece_index, 15/*no promotion*/, flags);
                all_moves[move_index] = m;
                move_index++;
                // remove considered attack
                clear_last_active_bit(attacks); 
            }
            // remove considered piece
            clear_last_active_bit(piece);
        }

        // Bishop
        piece_index = 3;
        piece = pos.pieces[piece_index]; // retrieve bitboard of bishops
        while(piece){ // consider all the bishops
            _BitScanForward64(&square, piece); // find position of queen and assign it to square
            // retrieve bitboard of bishop moves
            hash_index_bishop = bishop_hash_index(pos.all_pieces, square, n_attacks_bishop);
            attacks = bishop_covered_squares_bitboards[hash_index_bishop];
            attacks &= ~pos.white_pieces; // excluse self-capture
            // loop over all the attacks
            while(attacks){
                flags = 0;
                _BitScanForward64(&target_square, attacks); // find the target square
                is_capture = bit_get(pos.black_pieces, target_square);
                captured_piece_index = 15; // no capture 
                // Generate new position applying the move
                m.position = pos;
                // move the piece in white pieces bitboards
                bit_clear(m.position.pieces[piece_index], square); bit_set(m.position.pieces[piece_index], target_square);
                bit_clear(m.position.white_pieces, square); bit_set(m.position.white_pieces, target_square);
                // if capture, also change bitboards of black pieces and recompute values
                if(is_capture){
                    flags += 8; // capture flag
                    // check what black piece has been captured
                    for(uint8_t index = 6; index < 12; index++){
                        if(bit_get(pos.pieces[index], target_square)){
                            captured_piece_index = index;
                            break;
                        }
                    }
                    //m.position.black_material_value -= PIECES_VALUES[captured_piece_index];
                    m.position.half_move_counter = 0;
                    bit_clear(m.position.pieces[captured_piece_index], target_square);
                    bit_clear(m.position.black_pieces, target_square);
                }
                else{ m.position.half_move_counter++; }
                // compute all pieces bitboard and the covered squares
                m.position.all_pieces = m.position.white_pieces | m.position.black_pieces;
                m.position.white_covered_squares = GetCoveredSquares(m.position.pieces, m.position.all_pieces, true);
                m.position.black_covered_squares = GetCoveredSquares(m.position.pieces, m.position.all_pieces, false);
                // check if move is legal
                is_illegal = m.position.pieces[0] & m.position.black_covered_squares;
                if(is_illegal){ clear_last_active_bit(attacks); continue; }
                // standard updates
                m.position.white_to_move = false;
                m.position.en_passant_target_square = 0ULL;
                // check if the move is a check
                is_check = m.position.pieces[6] & m.position.white_covered_squares;
                if(is_check){ flags += 16; }
                // encode move 
                m.move = EncodeMove(static_cast<uint8_t>(square), static_cast<uint8_t>(target_square), piece_index, captured_piece_index, 15/*no promotion*/, flags);
                all_moves[move_index] = m;
                move_index++;
                // remove considered attack
                clear_last_active_bit(attacks); 
            }
            // remove considered piece
            clear_last_active_bit(piece);
        }

        // Knight
        piece_index = 4;
        piece = pos.pieces[piece_index]; // retrieve bitboard of knights
        while(piece){ // consider all the knights
            _BitScanForward64(&square, piece); // find position of piece and assign it to square
            attacks = knight_covered_squares_bitboards[square]; // retrieve attack bitboard
            attacks &= ~pos.white_pieces; // exclude self-capture
            while(attacks){
                flags = 0;
                _BitScanForward64(&target_square, attacks); // find the target square
                is_capture = bit_get(pos.black_pieces, target_square);
                captured_piece_index = 15; // no capture 
                // Generate new position applying the move
                m.position = pos;
                // move the piece in white pieces bitboards
                bit_clear(m.position.pieces[piece_index], square); bit_set(m.position.pieces[piece_index], target_square);
                bit_clear(m.position.white_pieces, square); bit_set(m.position.white_pieces, target_square);
                // if capture, also change bitboards of black pieces and recompute values
                if(is_capture){
                    flags += 8; // capture flag
                    // check what black piece has been captured
                    for(uint8_t index = 6; index < 12; index++){
                        if(bit_get(pos.pieces[index], target_square)){
                            captured_piece_index = index;
                            break;
                        }
                    }
                    //m.position.black_material_value -= PIECES_VALUES[captured_piece_index];
                    m.position.half_move_counter = 0;
                    bit_clear(m.position.pieces[captured_piece_index], target_square);
                    bit_clear(m.position.black_pieces, target_square);
                }
                else{ m.position.half_move_counter++; }
                // compute all pieces bitboard and the covered squares
                m.position.all_pieces = m.position.white_pieces | m.position.black_pieces;
                m.position.white_covered_squares = GetCoveredSquares(m.position.pieces, m.position.all_pieces, true);
                m.position.black_covered_squares = GetCoveredSquares(m.position.pieces, m.position.all_pieces, false);
                // check if move is legal
                is_illegal = m.position.pieces[0] & m.position.black_covered_squares;
                if(is_illegal){ clear_last_active_bit(attacks); continue; }
                // standard updates
                m.position.white_to_move = !pos.white_to_move;
                m.position.en_passant_target_square = 0ULL;
                // check if the move is a check
                is_check = m.position.pieces[6] & m.position.white_covered_squares;
                if(is_check){ flags += 16; }
                // encode move 
                m.move = EncodeMove(static_cast<uint8_t>(square), static_cast<uint8_t>(target_square), piece_index, captured_piece_index, 15/*no promotion*/, flags);
                all_moves[move_index] = m;
                move_index++;
                clear_last_active_bit(attacks); // remove considered attack
            }
            // remove considered piece
            clear_last_active_bit(piece);
        }

        // Pawns 
        piece_index = 5;
        piece = pos.pieces[piece_index];
        while(piece){
            _BitScanForward64(&square, piece); // find position of piece and assign it to square
            // NORMAL CAPTURES
            attacks = white_pawn_covered_squares_bitboards[square]; // retrieve attack bitboard
            attacks &= pos.black_pieces | pos.en_passant_target_square; // only attacked squares occupied by enemy pieces are valid for movement
            while(attacks){
                flags = 1;
                _BitScanForward64(&target_square, attacks); // find the target square
                is_capture = true; // this is necessarily a capture!
                flags += 8; // capture flag
                // check what black piece has been captured
                for(int index = 6; index < 12; index++){
                    if(bit_get(pos.pieces[index], target_square)){
                        captured_piece_index = index;
                        break;
                    }
                }
                // in case of promotion, loop over all possible promoted pieces
                if(target_square / 8 == 0){
                    for(uint8_t promoted_piece_index = 1; promoted_piece_index < 5; promoted_piece_index++){
                        // Generate new position applying the move
                        m.position = pos;
                        // move the piece in white pieces bitboards
                        bit_clear(m.position.pieces[piece_index], square); bit_set(m.position.pieces[promoted_piece_index], target_square);
                        bit_clear(m.position.white_pieces, square); bit_set(m.position.white_pieces, target_square);
                        //m.position.white_material_value += PIECES_VALUES[promoted_piece_index] - WHITE_PAWN_VALUE;
                        //m.position.black_material_value -= PIECES_VALUES[captured_piece_index];
                        m.position.half_move_counter = 0;
                        bit_clear(m.position.pieces[captured_piece_index], target_square);
                        bit_clear(m.position.black_pieces, target_square);                        
                        // compute all pieces bitboard and the covered squares
                        m.position.all_pieces = m.position.white_pieces | m.position.black_pieces;
                        m.position.white_covered_squares = GetCoveredSquares(m.position.pieces, m.position.all_pieces, true);
                        m.position.black_covered_squares = GetCoveredSquares(m.position.pieces, m.position.all_pieces, false);
                        // check if move is legal
                        is_illegal = m.position.pieces[0] & m.position.black_covered_squares;
                        if(is_illegal){ continue; }
                        // standard updates
                        m.position.white_to_move = false;
                        m.position.en_passant_target_square = 0ULL;
                        // check if the move is a check
                        is_check = m.position.pieces[6] & m.position.white_covered_squares;
                        if(is_check){ flags += 16; }
                        // encode move
                        m.move = EncodeMove(static_cast<uint8_t>(square), static_cast<uint8_t>(target_square), piece_index, captured_piece_index, promoted_piece_index, flags);
                        all_moves[move_index] = m;
                        move_index++;
                    }
                }
                else{
                    // Generate new position applying the move
                    m.position = pos;
                    // move the piece in white pieces bitboards
                    bit_clear(m.position.pieces[piece_index], square); bit_set(m.position.pieces[piece_index], target_square);
                    bit_clear(m.position.white_pieces, square); bit_set(m.position.white_pieces, target_square);
                    // also change bitboards of black pieces and recompute values (this is always a capture)
                    //m.position.black_material_value -= PIECES_VALUES[captured_piece_index];
                    m.position.half_move_counter = 0;
                    if((pos.en_passant_target_square & (1ULL << target_square)) == 0){ // no en passant
                        bit_clear(m.position.pieces[captured_piece_index], target_square);
                        bit_clear(m.position.black_pieces, target_square);   
                    } 
                    else{ // en passant capture
                        captured_piece_index = 11;
                        target_square += 8;
                        bit_clear(m.position.pieces[captured_piece_index], target_square);
                        bit_clear(m.position.black_pieces, target_square);   
                        target_square -= 8;
                    }               
                    // compute all pieces bitboard and the covered squares
                    m.position.all_pieces = m.position.white_pieces | m.position.black_pieces;
                    m.position.white_covered_squares = GetCoveredSquares(m.position.pieces, m.position.all_pieces, true);
                    m.position.black_covered_squares = GetCoveredSquares(m.position.pieces, m.position.all_pieces, false);
                    // check if move is legal
                    is_illegal = m.position.pieces[0] & m.position.black_covered_squares;
                    if(is_illegal){ clear_last_active_bit(attacks); continue; }
                    // standard updates
                    m.position.white_to_move = false;
                    m.position.en_passant_target_square = 0ULL;
                    // check if the move is a check
                    is_check = m.position.pieces[6] & m.position.white_covered_squares;
                    if(is_check){ flags += 16; }
                    // encode move
                    m.move = EncodeMove(static_cast<uint8_t>(square), static_cast<uint8_t>(target_square), piece_index, captured_piece_index, 15/*no promotion*/, flags);
                    all_moves[move_index] = m;
                    move_index++;
                }
                clear_last_active_bit(attacks); // remove considered attack
            }

            // PUSH
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
            if((square/8 == 6) && !bit_get(attacks, square - 8) && bit_get(attacks, square - 16)){
                attacks = 0ULL;
            }
            while(attacks){
                _BitScanForward64(&target_square, attacks); // find the target square
                is_capture = false; // this is necessarily NOT a capture!
                // in case of promotion, loop over all possible promoted pieces
                if(target_square / 8 == 0){
                    for(uint8_t promoted_piece_index = 1; promoted_piece_index < 5; promoted_piece_index++){
                        flags = 1;
                        // Generate new position applying the move
                        m.position = pos;
                        // move the piece in white pieces bitboards
                        bit_clear(m.position.pieces[piece_index], square); bit_set(m.position.pieces[promoted_piece_index], target_square);
                        bit_clear(m.position.white_pieces, square); bit_set(m.position.white_pieces, target_square);
                        // spawn the new piece 
                        //m.position.white_material_value += PIECES_VALUES[promoted_piece_index] - WHITE_PAWN_VALUE;
                        // compute all pieces bitboard and the covered squares
                        m.position.all_pieces = m.position.white_pieces | m.position.black_pieces;
                        m.position.white_covered_squares = GetCoveredSquares(m.position.pieces, m.position.all_pieces, true);
                        m.position.black_covered_squares = GetCoveredSquares(m.position.pieces, m.position.all_pieces, false);
                        // check if move is legal
                        is_illegal = m.position.pieces[0] & m.position.black_covered_squares;
                        if(is_illegal){ clear_last_active_bit(attacks); continue; }
                        m.position.half_move_counter = 0;
                        // standard updates
                        m.position.white_to_move = false;
                        m.position.en_passant_target_square = 0ULL;
                        // check if the move is a check
                        is_check = m.position.pieces[6] & m.position.white_covered_squares;
                        if(is_check){ flags += 16; }             
                        // flag that this is a promotion ...
                        m.move = EncodeMove(static_cast<uint8_t>(square), static_cast<uint8_t>(target_square), piece_index, 15/*no capture*/, promoted_piece_index, flags);
                        all_moves[move_index] = m;
                        move_index++;
                    }
                }
                // if this is a double push, flag it to manage en-passant target squares later
                else if(target_square / 8 == 4 && square / 8 == 6){
                    flags = 1;
                    // Generate new position applying the move
                    m.position = pos;
                    // move the piece in white pieces bitboards
                    bit_clear(m.position.pieces[piece_index], square); bit_set(m.position.pieces[piece_index], target_square);
                    bit_clear(m.position.white_pieces, square); bit_set(m.position.white_pieces, target_square);
                    // compute all pieces bitboard and the covered squares
                    m.position.all_pieces = m.position.white_pieces | m.position.black_pieces;
                    m.position.white_covered_squares = GetCoveredSquares(m.position.pieces, m.position.all_pieces, true);
                    m.position.black_covered_squares = GetCoveredSquares(m.position.pieces, m.position.all_pieces, false);
                    // check if move is legal
                    is_illegal = m.position.pieces[0] & m.position.black_covered_squares;
                    if(is_illegal){ clear_last_active_bit(attacks); continue; }
                    m.position.half_move_counter = 0;
                    // standard updates
                    m.position.white_to_move = false;
                    m.position.en_passant_target_square = 0ULL;
                    m.position.en_passant_target_square |= (1ULL << (target_square + 8));
                    // check if the move is a check
                    is_check = m.position.pieces[6] & m.position.white_covered_squares;
                    if(is_check){ flags += 16; }   
                    flags += 2;
                    m.move = EncodeMove(static_cast<uint8_t>(square), static_cast<uint8_t>(target_square), piece_index, 15/*no capture*/, 15/*no promotion*/, flags);
                    all_moves[move_index] = m;
                    move_index++;
                }
                else{
                    flags = 1;
                    // Generate new position applying the move
                    m.position = pos;
                    // move the piece in white pieces bitboards
                    bit_clear(m.position.pieces[piece_index], square); bit_set(m.position.pieces[piece_index], target_square);
                    bit_clear(m.position.white_pieces, square); bit_set(m.position.white_pieces, target_square);
                    // compute all pieces bitboard and the covered squares
                    m.position.all_pieces = m.position.white_pieces | m.position.black_pieces;
                    m.position.white_covered_squares = GetCoveredSquares(m.position.pieces, m.position.all_pieces, true);
                    m.position.black_covered_squares = GetCoveredSquares(m.position.pieces, m.position.all_pieces, false);
                    // check if move is legal
                    is_illegal = m.position.pieces[0] & m.position.black_covered_squares;
                    if(is_illegal){ clear_last_active_bit(attacks); continue; }
                    m.position.half_move_counter = 0;
                    // standard updates
                    m.position.white_to_move = false;
                    m.position.en_passant_target_square = 0ULL;
                    // check if the move is a check
                    is_check = m.position.pieces[6] & m.position.white_covered_squares;
                    if(is_check){ flags += 16; }   
                    // encode move if legal 
                    m.move = EncodeMove(static_cast<uint8_t>(square), static_cast<uint8_t>(target_square), piece_index, 15/*no capture*/, 15/*no promotion*/, flags);
                    all_moves[move_index] = m;
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
            if(!bit_get(pos.all_pieces, 61) && 
                !bit_get(pos.all_pieces, 62)){ // 2. the in-between squares are empty
                if(!bit_get(pos.black_covered_squares, 60) &&
                    !bit_get(pos.black_covered_squares, 61) &&
                    !bit_get(pos.black_covered_squares, 62)){ // 3. king does not pass through a square covered by opponent
                   if(bit_get(pos.pieces[0], 60) &&
                        bit_get(pos.pieces[2], 63)){ // 4. king and rook are in the correct position
                        flags = 4;
                        // Generate new position applying the move
                        m.position = pos;
                        // move king and rook in white pieces bitboards
                        m.position.pieces[0] &= ~(1ULL << 60);
                        m.position.pieces[0] |= (1ULL << 62);
                        m.position.pieces[2] &= ~(1ULL << 63);
                        m.position.pieces[2] |= (1ULL << 61);
                        m.position.white_pieces &= ~(1ULL << 60);
                        m.position.white_pieces &= ~(1ULL << 63);
                        m.position.white_pieces |= (1ULL << 61);
                        m.position.white_pieces |= (1ULL << 62);
                        // compute all pieces bitboard and the covered squares
                        m.position.all_pieces = m.position.white_pieces | m.position.black_pieces;
                        m.position.white_covered_squares = GetCoveredSquares(m.position.pieces, m.position.all_pieces, true);
                        m.position.black_covered_squares = GetCoveredSquares(m.position.pieces, m.position.all_pieces, false);
                        m.position.half_move_counter++;
                        // standard updates
                        m.position.white_to_move = false;
                        m.position.can_white_castle_kingside = false;
                        m.position.can_white_castle_queenside = false;
                        m.position.en_passant_target_square = 0ULL;
                        // check if the move is a check
                        is_check = m.position.pieces[6] & m.position.white_covered_squares;
                        if(is_check){ flags += 16; }   
                        // no need to check legality: the move is always legal within these conditions
                        // encode move, conventionally considered as a king move from square = 60 to target_square = 62
                        m.move = EncodeMove(60, 62, 0/*king*/, 15/*no capture*/, 15/*no promotion*/, flags); 
                        all_moves[move_index] = m;
                        move_index++;
                   } 
                }
            }   
        }
        // Castles queenside
        if(pos.can_white_castle_queenside){ 
            if(!bit_get(pos.all_pieces, 59) && 
                !bit_get(pos.all_pieces, 58) &&
                !bit_get(pos.all_pieces, 57)){ 
                if(!bit_get(pos.black_covered_squares, 60) &&
                    !bit_get(pos.black_covered_squares, 59) &&
                    !bit_get(pos.black_covered_squares, 58)){
                   if(bit_get(pos.pieces[0], 60) &&
                        bit_get(pos.pieces[2], 56)){ 
                        flags = 4;
                        // Generate new position applying the move
                        m.position = pos;
                        // move king and rook in white pieces bitboards
                        m.position.pieces[0] &= ~(1ULL << 60); // bit clear (faster like this)
                        m.position.pieces[0] |= (1ULL << 58);
                        m.position.pieces[2] &= ~(1ULL << 56);
                        m.position.pieces[2] |= (1ULL << 59); // bit set (faster like this)
                        m.position.white_pieces &= ~(1ULL << 60);
                        m.position.white_pieces &= ~(1ULL << 56);
                        m.position.white_pieces |= (1ULL << 58);
                        m.position.white_pieces |= (1ULL << 59);
                        // compute all pieces bitboard and the covered squares
                        m.position.all_pieces = m.position.white_pieces | m.position.black_pieces;
                        m.position.white_covered_squares = GetCoveredSquares(m.position.pieces, m.position.all_pieces, true);
                        m.position.black_covered_squares = GetCoveredSquares(m.position.pieces, m.position.all_pieces, false);
                        m.position.half_move_counter++;
                        // standard updates
                        m.position.white_to_move = false;
                        m.position.can_white_castle_kingside = false;
                        m.position.can_white_castle_queenside = false;
                        m.position.en_passant_target_square = 0ULL;
                        // check if the move is a check
                        is_check = m.position.pieces[6] & m.position.white_covered_squares;
                        if(is_check){ flags += 16; }   
                        m.move = EncodeMove(60, 58, 0/*king*/, 15/*no capture*/, 15/*no promotion*/, flags); 
                        all_moves[move_index] = m;
                        move_index++;
                   } 
                }
            }   
        }
        
    }
    
    // BLACK TO MOVE
    else{

        // King
        piece_index = 6;
        piece = pos.pieces[piece_index]; // retrieve bitboard of king
        while(piece){ 
            _BitScanForward64(&square, piece); // find position of piece and assign it to square
            attacks = king_covered_squares_bitboards[square]; // retrieve attack bitboard
            attacks &= ~pos.black_pieces; // exclude self-capture
            while(attacks){
                flags = 0;
                _BitScanForward64(&target_square, attacks); // find the target square
                is_capture = bit_get(pos.white_pieces, target_square);
                captured_piece_index = 15; // no capture 
                // generate new position
                m.position = pos;
                // move the piece in black pieces bitboards
                bit_clear(m.position.pieces[piece_index], square); bit_set(m.position.pieces[piece_index], target_square);
                bit_clear(m.position.black_pieces, square); bit_set(m.position.black_pieces, target_square);
                // if capture, also change bitboards of white pieces and recompute values
                if(is_capture){
                    flags += 8; // capture flag
                    // check what black piece has been captured
                    for(uint8_t index = 0; index < 6; index++){
                        if(bit_get(pos.pieces[index], target_square)){
                            captured_piece_index = index;
                            break;
                        }
                    }
                    //m.position.white_material_value -= PIECES_VALUES[captured_piece_index];
                    m.position.half_move_counter = 0;
                    bit_clear(m.position.pieces[captured_piece_index], target_square);
                    bit_clear(m.position.white_pieces, target_square);
                }
                else{ m.position.half_move_counter++; }
                // compute all pieces bitboard and the covered squares
                m.position.all_pieces = m.position.white_pieces | m.position.black_pieces;
                m.position.white_covered_squares = GetCoveredSquares(m.position.pieces, m.position.all_pieces, true);
                m.position.black_covered_squares = GetCoveredSquares(m.position.pieces, m.position.all_pieces, false);
                // check if move is legal
                is_illegal = m.position.pieces[6] & m.position.white_covered_squares;
                if(is_illegal){ clear_last_active_bit(attacks); continue; }
                // castling rights are lost
                m.position.can_black_castle_kingside = false;
                m.position.can_black_castle_queenside = false;
                // standard updates
                m.position.white_to_move = true;
                m.position.en_passant_target_square = 0ULL;
                // check if the move is a check
                is_check = m.position.pieces[0] & m.position.black_covered_squares;
                if(is_check){ flags += 16; }
                // encode move 
                m.move = EncodeMove(static_cast<uint8_t>(square), static_cast<uint8_t>(target_square), piece_index, captured_piece_index, 15/*no promotion*/, flags);
                all_moves[move_index] = m;
                move_index++;
                clear_last_active_bit(attacks); // remove considered attack
            }
            // remove considered piece
            clear_last_active_bit(piece);
        }

        // Queen
        piece_index = 7;
        piece = pos.pieces[piece_index]; // retrieve bitboard of queens
        while(piece){ // consider all the queens
            _BitScanForward64(&square, piece); // find position of queen and assign it to square
            // retrieve bitboard of queen moves
            hash_index_rook = rook_hash_index(pos.all_pieces, square, n_attacks_rook);
            hash_index_bishop = bishop_hash_index(pos.all_pieces, square, n_attacks_bishop);
            attacks = rook_covered_squares_bitboards[hash_index_rook] | bishop_covered_squares_bitboards[hash_index_bishop];
            attacks &= ~pos.black_pieces; // excluse self-capture
            // loop over all the attacks
            while(attacks){
                flags = 0;
                _BitScanForward64(&target_square, attacks); // find the target square
                is_capture = bit_get(pos.white_pieces, target_square);
                captured_piece_index = 15; // no capture 
                // Generate new position applying the move
                m.position = pos;
                // move the piece in white pieces bitboards
                bit_clear(m.position.pieces[piece_index], square); bit_set(m.position.pieces[piece_index], target_square);
                bit_clear(m.position.black_pieces, square); bit_set(m.position.black_pieces, target_square);
                // if capture, also change bitboards of black pieces and recompute values
                if(is_capture){
                    flags += 8; // capture flag
                    // check what black piece has been captured
                    for(uint8_t index = 0; index < 6; index++){
                        if(bit_get(pos.pieces[index], target_square)){
                            captured_piece_index = index;
                            break;
                        }
                    }
                    //m.position.white_material_value -= PIECES_VALUES[captured_piece_index];
                    m.position.half_move_counter = 0;
                    bit_clear(m.position.pieces[captured_piece_index], target_square);
                    bit_clear(m.position.white_pieces, target_square);
                }
                else{ m.position.half_move_counter++; }
                // compute all pieces bitboard and the covered squares
                m.position.all_pieces = m.position.white_pieces | m.position.black_pieces;
                m.position.white_covered_squares = GetCoveredSquares(m.position.pieces, m.position.all_pieces, true);
                m.position.black_covered_squares = GetCoveredSquares(m.position.pieces, m.position.all_pieces, false);
                // check if move is legal
                is_illegal = m.position.pieces[6] & m.position.white_covered_squares;
                if(is_illegal){ clear_last_active_bit(attacks); continue; }
                // standard updates
                m.position.white_to_move = true;
                m.position.en_passant_target_square = 0ULL;
                // check if the move is a check
                is_check = m.position.pieces[0] & m.position.black_covered_squares;
                if(is_check){ flags += 16; }
                // encode move 
                m.move = EncodeMove(static_cast<uint8_t>(square), static_cast<uint8_t>(target_square), piece_index, captured_piece_index, 15/*no promotion*/, flags);
                all_moves[move_index] = m;
                move_index++;
                clear_last_active_bit(attacks); // remove considered attack
            }
            // remove considered piece
            clear_last_active_bit(piece);
        }

        // Rook
        piece_index = 8;
        piece = pos.pieces[piece_index];
        while(piece){ // consider all the queens
            _BitScanForward64(&square, piece); // find position of queen and assign it to square
            // retrieve bitboard of queen moves
            hash_index_rook = rook_hash_index(pos.all_pieces, square, n_attacks_rook);
            attacks = rook_covered_squares_bitboards[hash_index_rook];
            attacks &= ~pos.black_pieces; // excluse self-capture
            // loop over all the attacks
            while(attacks){
                flags = 0;
                _BitScanForward64(&target_square, attacks); // find the target square
                is_capture = bit_get(pos.white_pieces, target_square);
                captured_piece_index = 15; // no capture 
                // Generate new position applying the move
                m.position = pos;
                // move the piece in white pieces bitboards
                bit_clear(m.position.pieces[piece_index], square); bit_set(m.position.pieces[piece_index], target_square);
                bit_clear(m.position.black_pieces, square); bit_set(m.position.black_pieces, target_square);
                // if capture, also change bitboards of black pieces and recompute values
                if(is_capture){
                    flags += 8; // capture flag
                    // check what black piece has been captured
                    for(uint8_t index = 0; index < 6; index++){
                        if(bit_get(pos.pieces[index], target_square)){
                            captured_piece_index = index;
                            break;
                        }
                    }
                    //m.position.white_material_value -= PIECES_VALUES[captured_piece_index];
                    m.position.half_move_counter = 0;
                    bit_clear(m.position.pieces[captured_piece_index], target_square);
                    bit_clear(m.position.white_pieces, target_square);
                }
                else{ m.position.half_move_counter++; }
                // compute all pieces bitboard and the covered squares
                m.position.all_pieces = m.position.white_pieces | m.position.black_pieces;
                m.position.white_covered_squares = GetCoveredSquares(m.position.pieces, m.position.all_pieces, true);
                m.position.black_covered_squares = GetCoveredSquares(m.position.pieces, m.position.all_pieces, false);
                // check if move is legal
                is_illegal = m.position.pieces[6] & m.position.white_covered_squares;
                if(is_illegal){ clear_last_active_bit(attacks); continue; }
                // castling rights are lost
                if(square == 7) { m.position.can_black_castle_kingside = false; }
                else if(square == 0) { m.position.can_black_castle_queenside = false; }
                // standard updates
                m.position.white_to_move = true;
                m.position.en_passant_target_square = 0ULL;
                // check if the move is a check
                is_check = m.position.pieces[0] & m.position.black_covered_squares;
                if(is_check){ flags += 16; }
                // encode move 
                m.move = EncodeMove(static_cast<uint8_t>(square), static_cast<uint8_t>(target_square), piece_index, captured_piece_index, 15/*no promotion*/, flags);
                all_moves[move_index] = m;
                move_index++;
                clear_last_active_bit(attacks); // remove considered attack
            }
            // remove considered piece
            clear_last_active_bit(piece);
        }

        // Bishop
        piece_index = 9;
        piece = pos.pieces[piece_index]; // retrieve bitboard of bishops
        while(piece){ // consider all the bishops
            _BitScanForward64(&square, piece); // find position of queen and assign it to square
            // retrieve bitboard of bishop moves
            hash_index_bishop = bishop_hash_index(pos.all_pieces, square, n_attacks_bishop);
            attacks = bishop_covered_squares_bitboards[hash_index_bishop];
            attacks &= ~pos.black_pieces; // excluse self-capture
            // loop over all the attacks
            while(attacks){
                flags = 0;
                _BitScanForward64(&target_square, attacks); // find the target square
                is_capture = bit_get(pos.white_pieces, target_square);
                captured_piece_index = 15; // no capture 
                // Generate new position applying the move
                m.position = pos;
                // move the piece in white pieces bitboards
                bit_clear(m.position.pieces[piece_index], square); bit_set(m.position.pieces[piece_index], target_square);
                bit_clear(m.position.black_pieces, square); bit_set(m.position.black_pieces, target_square);
                // if capture, also change bitboards of black pieces and recompute values
                if(is_capture){
                    flags += 8; // capture flag
                    // check what black piece has been captured
                    for(uint8_t index = 0; index < 6; index++){
                        if(bit_get(pos.pieces[index], target_square)){
                            captured_piece_index = index;
                            break;
                        }
                    }
                    //m.position.white_material_value -= PIECES_VALUES[captured_piece_index];
                    m.position.half_move_counter = 0;
                    bit_clear(m.position.pieces[captured_piece_index], target_square);
                    bit_clear(m.position.white_pieces, target_square);
                }
                else{ m.position.half_move_counter++; }
                // compute all pieces bitboard and the covered squares
                m.position.all_pieces = m.position.white_pieces | m.position.black_pieces;
                m.position.white_covered_squares = GetCoveredSquares(m.position.pieces, m.position.all_pieces, true);
                m.position.black_covered_squares = GetCoveredSquares(m.position.pieces, m.position.all_pieces, false);
                // check if move is legal
                is_illegal = m.position.pieces[6] & m.position.white_covered_squares;
                if(is_illegal){ clear_last_active_bit(attacks); continue; }
                // standard updates
                m.position.white_to_move = true;
                m.position.en_passant_target_square = 0ULL;
                // check if the move is a check
                is_check = m.position.pieces[0] & m.position.black_covered_squares;
                if(is_check){ flags += 16; }
                // encode move 
                m.move = EncodeMove(static_cast<uint8_t>(square), static_cast<uint8_t>(target_square), piece_index, captured_piece_index, 15/*no promotion*/, flags);
                all_moves[move_index] = m;
                move_index++;
                clear_last_active_bit(attacks); // remove considered attack
            }
            // remove considered piece
            clear_last_active_bit(piece);
        }

        // Knight
        piece_index = 10;
        piece = pos.pieces[piece_index]; // retrieve bitboard of knights
        while(piece){ // consider all the knights
            _BitScanForward64(&square, piece); // find position of piece and assign it to square
            attacks = knight_covered_squares_bitboards[square]; // retrieve attack bitboard
            attacks &= ~pos.black_pieces; // exclude self-capture
            while(attacks){
                flags = 0;
                _BitScanForward64(&target_square, attacks); // find the target square
                is_capture = bit_get(pos.white_pieces, target_square);
                captured_piece_index = 15; // no capture 
                // Generate new position applying the move
                m.position = pos;
                // move the piece in white pieces bitboards
                bit_clear(m.position.pieces[piece_index], square); bit_set(m.position.pieces[piece_index], target_square);
                bit_clear(m.position.black_pieces, square); bit_set(m.position.black_pieces, target_square);
                // if capture, also change bitboards of black pieces and recompute values
                if(is_capture){
                    flags += 8; // capture flag
                    // check what black piece has been captured
                    for(uint8_t index = 0; index < 6; index++){
                        if(bit_get(pos.pieces[index], target_square)){
                            captured_piece_index = index;
                            break;
                        }
                    }
                    //m.position.white_material_value -= PIECES_VALUES[captured_piece_index];
                    m.position.half_move_counter = 0;
                    bit_clear(m.position.pieces[captured_piece_index], target_square);
                    bit_clear(m.position.white_pieces, target_square);
                }
                else{ m.position.half_move_counter++; }
                // compute all pieces bitboard and the covered squares
                m.position.all_pieces = m.position.white_pieces | m.position.black_pieces;
                m.position.white_covered_squares = GetCoveredSquares(m.position.pieces, m.position.all_pieces, true);
                m.position.black_covered_squares = GetCoveredSquares(m.position.pieces, m.position.all_pieces, false);
                // check if move is legal
                is_illegal = m.position.pieces[6] & m.position.white_covered_squares;
                if(is_illegal){ clear_last_active_bit(attacks); continue; }
                // standard updates
                m.position.white_to_move = true;
                m.position.en_passant_target_square = 0ULL;
                // check if the move is a check
                is_check = m.position.pieces[0] & m.position.black_covered_squares;
                if(is_check){ flags += 16; }
                // encode move 
                m.move = EncodeMove(static_cast<uint8_t>(square), static_cast<uint8_t>(target_square), piece_index, captured_piece_index, 15/*no promotion*/, flags);
                all_moves[move_index] = m;
                move_index++;
                clear_last_active_bit(attacks); // remove considered attack
            }
            // remove considered piece
            clear_last_active_bit(piece);
        }

        // Pawns 
        piece_index = 11;
        piece = pos.pieces[piece_index];
        while(piece){
            _BitScanForward64(&square, piece); // find position of piece and assign it to square
            // CAPTURES
            attacks = black_pawn_covered_squares_bitboards[square]; // retrieve attack bitboard
            attacks &= pos.white_pieces | pos.en_passant_target_square; // only attacked squares occupied by enemy pieces are valid for movement
            while(attacks){
                _BitScanForward64(&target_square, attacks); // find the target square
                is_capture = true; // this is necessarily a capture!
                flags += 8; // capture flag
                // check what white piece has been captured
                for(uint8_t index = 0; index < 6; index++){
                    if(bit_get(pos.pieces[index], target_square)){
                        captured_piece_index = index;
                        break;
                    }
                }
                // in case of promotion, loop over all possible promoted pieces
                if(target_square / 8 == 7){
                    for(uint8_t promoted_piece_index = 7; promoted_piece_index < 11; promoted_piece_index++){
                        flags = 1;
                        // Generate new position applying the move
                        m.position = pos;
                        // move the piece in black pieces bitboards
                        bit_clear(m.position.pieces[piece_index], square); bit_set(m.position.pieces[promoted_piece_index], target_square);
                        bit_clear(m.position.black_pieces, square); bit_set(m.position.black_pieces, target_square);
                        // capture the piece 
                        bit_clear(m.position.pieces[captured_piece_index], target_square);
                        bit_clear(m.position.white_pieces, target_square);
                        // update material value
                        //m.position.black_material_value += PIECES_VALUES[promoted_piece_index] - BLACK_PAWN_VALUE;
                        //m.position.white_material_value -= PIECES_VALUES[captured_piece_index];
                        // compute all pieces bitboard and the covered squares
                        m.position.all_pieces = m.position.white_pieces | m.position.black_pieces;
                        m.position.white_covered_squares = GetCoveredSquares(m.position.pieces, m.position.all_pieces, true);
                        m.position.black_covered_squares = GetCoveredSquares(m.position.pieces, m.position.all_pieces, false);
                        // check if move is legal
                        is_illegal = m.position.pieces[6] & m.position.white_covered_squares;
                        if(is_illegal){ continue; }
                        m.position.half_move_counter = 0;
                        // standard updates
                        m.position.white_to_move = true;
                        m.position.en_passant_target_square = 0ULL;
                        // check if the move is a check
                        is_check = m.position.pieces[0] & m.position.black_covered_squares;
                        if(is_check){ flags += 16; }            
                        m.move = EncodeMove(static_cast<uint8_t>(square), static_cast<uint8_t>(target_square), piece_index, captured_piece_index, promoted_piece_index, flags);
                        all_moves[move_index] = m;
                        move_index++;
                    }
                }
                else{
                    flags = 1;
                    // Generate new position applying the move
                    m.position = pos;
                    // move the piece in white pieces bitboards
                    bit_clear(m.position.pieces[piece_index], square); bit_set(m.position.pieces[piece_index], target_square);
                    bit_clear(m.position.black_pieces, square); bit_set(m.position.black_pieces, target_square);
                    // also change bitboards of enemy pieces and recompute values (this is always a capture)
                    //m.position.white_material_value -= PIECES_VALUES[captured_piece_index];
                    m.position.half_move_counter = 0;
                    if((pos.en_passant_target_square & (1ULL << target_square)) == 0){ // NO en passant
                        bit_clear(m.position.pieces[captured_piece_index], target_square);
                        bit_clear(m.position.white_pieces, target_square);    
                    }
                    else{ // en passant
                        captured_piece_index = 5;
                        target_square -= 8;
                        bit_clear(m.position.pieces[captured_piece_index], target_square);
                        bit_clear(m.position.white_pieces, target_square);   
                        target_square += 8;                 
                    }   
                    // compute all pieces bitboard and the covered squares
                    m.position.all_pieces = m.position.white_pieces | m.position.black_pieces;
                    m.position.white_covered_squares = GetCoveredSquares(m.position.pieces, m.position.all_pieces, true);
                    m.position.black_covered_squares = GetCoveredSquares(m.position.pieces, m.position.all_pieces, false);
                    // check if move is legal
                    is_illegal = m.position.pieces[6] & m.position.white_covered_squares;
                    if(is_illegal){ clear_last_active_bit(attacks); continue; }
                    // standard updates
                    m.position.white_to_move = true;
                    m.position.en_passant_target_square = 0ULL;
                    // check if the move is a check
                    is_check = m.position.pieces[0] & m.position.black_covered_squares;
                    if(is_check){ flags += 16; }
                    m.move = EncodeMove(static_cast<uint8_t>(square), static_cast<uint8_t>(target_square), piece_index, captured_piece_index, 15/*no promotion*/, flags);
                    all_moves[move_index] = m;
                    move_index++;
                }
                clear_last_active_bit(attacks); // remove considered attack
            }

            // PUSH
            attacks = black_pawn_advance_squares_bitboards[square]; // retrieve attack bitboard
            attacks &= ~pos.all_pieces; // control that there are no blockers in front
            if((square/8 == 1) && !bit_get(attacks, square + 8) && bit_get(attacks, square + 16)){
                attacks = 0ULL;
            }
            while(attacks){
                _BitScanForward64(&target_square, attacks); // find the target square
                is_capture = false; // this is necessarily NOT a capture!
                // in case of promotion, loop over all possible promoted pieces
                if(target_square / 8 == 7){
                    for(uint8_t promoted_piece_index = 7; promoted_piece_index < 11; promoted_piece_index++){
                        flags = 1;
                        // Generate new position applying the move
                        m.position = pos;
                        // move the piece in white pieces bitboards
                        bit_clear(m.position.pieces[piece_index], square); bit_set(m.position.pieces[promoted_piece_index], target_square);
                        bit_clear(m.position.black_pieces, square); bit_set(m.position.black_pieces, target_square);
                        //m.position.black_material_value += PIECES_VALUES[promoted_piece_index] - BLACK_PAWN_VALUE;
                        // compute all pieces bitboard and the covered squares
                        m.position.all_pieces = m.position.white_pieces | m.position.black_pieces;
                        m.position.white_covered_squares = GetCoveredSquares(m.position.pieces, m.position.all_pieces, true);
                        m.position.black_covered_squares = GetCoveredSquares(m.position.pieces, m.position.all_pieces, false);
                        // check if move is legal
                        is_illegal = m.position.pieces[6] & m.position.white_covered_squares;
                        if(is_illegal){ clear_last_active_bit(attacks); continue; }
                        m.position.half_move_counter = 0;
                        // standard updates
                        m.position.white_to_move = true;
                        m.position.en_passant_target_square = 0ULL;
                        // check if the move is a check
                        is_check = m.position.pieces[0] & m.position.black_covered_squares;
                        if(is_check){ flags += 16; }             
                        // flag that this is a promotion ...
                        m.move = EncodeMove(static_cast<uint8_t>(square), static_cast<uint8_t>(target_square), piece_index, 15/*no capture*/, promoted_piece_index, flags);
                        all_moves[move_index] = m;
                        move_index++;
                    }
                }
                // if this is a double push, flag it to manage en-passant target squares
                else if(target_square / 8 == 3 && square / 8 == 1){
                    flags = 1;
                    // Generate new position applying the move
                    m.position = pos;
                    // move the piece in white pieces bitboards
                    bit_clear(m.position.pieces[piece_index], square); bit_set(m.position.pieces[piece_index], target_square);
                    bit_clear(m.position.black_pieces, square); bit_set(m.position.black_pieces, target_square);
                    // compute all pieces bitboard and the covered squares
                    m.position.all_pieces = m.position.white_pieces | m.position.black_pieces;
                    m.position.white_covered_squares = GetCoveredSquares(m.position.pieces, m.position.all_pieces, true);
                    m.position.black_covered_squares = GetCoveredSquares(m.position.pieces, m.position.all_pieces, false);
                    // check if move is legal
                    is_illegal = m.position.pieces[6] & m.position.white_covered_squares;
                    if(is_illegal){ clear_last_active_bit(attacks); continue; }
                    m.position.half_move_counter = 0;
                    // update side to move and add en-passant target!
                    m.position.white_to_move = true;
                    m.position.en_passant_target_square = 0ULL; 
                    m.position.en_passant_target_square |= (1ULL << (target_square - 8));
                    // check if the move is a check
                    is_check = m.position.pieces[0] & m.position.black_covered_squares;
                    if(is_check){ flags += 16; }   
                    // encode move
                    flags += 2;
                    m.move = EncodeMove(static_cast<uint8_t>(square), static_cast<uint8_t>(target_square), piece_index, 15/*no capture*/, 15/*no promotion*/, flags);
                    all_moves[move_index] = m;
                    move_index++;
                }
                else{
                    flags = 1;
                    // Generate new position applying the move
                    m.position = pos;
                    // move the piece in white pieces bitboards
                    bit_clear(m.position.pieces[piece_index], square); bit_set(m.position.pieces[piece_index], target_square);
                    bit_clear(m.position.black_pieces, square); bit_set(m.position.black_pieces, target_square);
                    // compute all pieces bitboard and the covered squares
                    m.position.all_pieces = m.position.white_pieces | m.position.black_pieces;
                    m.position.white_covered_squares = GetCoveredSquares(m.position.pieces, m.position.all_pieces, true);
                    m.position.black_covered_squares = GetCoveredSquares(m.position.pieces, m.position.all_pieces, false);
                    // check if move is legal
                    is_illegal = m.position.pieces[6] & m.position.white_covered_squares;
                    if(is_illegal){ clear_last_active_bit(attacks); continue; }
                    m.position.half_move_counter = 0;
                    // standard updates
                    m.position.white_to_move = true;
                    m.position.en_passant_target_square = 0ULL;
                    // check if the move is a check
                    is_check = m.position.pieces[0] & m.position.black_covered_squares;
                    if(is_check){ flags += 16; }   
                    // encode move if legal 
                    m.move = EncodeMove(static_cast<uint8_t>(square), static_cast<uint8_t>(target_square), piece_index, 15/*no capture*/, 15/*no promotion*/, flags);
                    all_moves[move_index] = m;
                    move_index++;
                }
                clear_last_active_bit(attacks); // remove considered attack
            }
            // remove considered piece
            clear_last_active_bit(piece);
        }

        // Castles kingside
        if(pos.can_black_castle_kingside){ 
            if(!bit_get(pos.all_pieces, 5) && 
                !bit_get(pos.all_pieces, 6)){ 
                if(!bit_get(pos.white_covered_squares, 4) &&
                    !bit_get(pos.white_covered_squares, 5) &&
                    !bit_get(pos.white_covered_squares, 6)){ 
                   if(bit_get(pos.pieces[6], 4) &&
                        bit_get(pos.pieces[8], 7)){ 
                        flags = 4;
                        // Generate new position applying the move
                        m.position = pos;
                        // move king and rook in white pieces bitboards
                        m.position.pieces[6] &= ~(1ULL << 4);
                        m.position.pieces[6] |= (1ULL << 6);
                        m.position.pieces[8] &= ~(1ULL << 7);
                        m.position.pieces[8] |= (1ULL << 5);
                        m.position.black_pieces &= ~(1ULL << 4);
                        m.position.black_pieces &= ~(1ULL << 7);
                        m.position.black_pieces |= (1ULL << 5);
                        m.position.black_pieces |= (1ULL << 6);
                        // compute all pieces bitboard and the covered squares
                        m.position.all_pieces = m.position.white_pieces | m.position.black_pieces;
                        m.position.white_covered_squares = GetCoveredSquares(m.position.pieces, m.position.all_pieces, true);
                        m.position.black_covered_squares = GetCoveredSquares(m.position.pieces, m.position.all_pieces, false);
                        m.position.half_move_counter++;
                        // standard updates -> loose castling rights
                        m.position.white_to_move = true;
                        m.position.can_black_castle_kingside = false;
                        m.position.can_black_castle_queenside = false;
                        m.position.en_passant_target_square = 0ULL;
                        // check if the move is a check
                        is_check = m.position.pieces[0] & m.position.black_covered_squares;
                        if(is_check){ flags += 16; }   
                        m.move = EncodeMove(4, 6, 6/*king*/, 15/*no capture*/, 15/*no promotion*/, flags); 
                        all_moves[move_index] = m;
                        move_index++;
                   } 
                }
            }   
        }
        // Castles queenside
        if(pos.can_black_castle_queenside){ 
            if(!bit_get(pos.all_pieces, 3) && 
                !bit_get(pos.all_pieces, 2) &&
                !bit_get(pos.all_pieces, 1)){ 
                if(!bit_get(pos.white_covered_squares, 4) &&
                    !bit_get(pos.white_covered_squares, 3) &&
                    !bit_get(pos.white_covered_squares, 2)){
                   if(bit_get(pos.pieces[6], 4) &&
                        bit_get(pos.pieces[8], 0)){ 
                        flags = 4;
                        // Generate new position applying the move
                        m.position = pos;
                        // move king and rook in white pieces bitboards
                        m.position.pieces[6] &= ~(1ULL << 4);
                        m.position.pieces[6] |= (1ULL << 2);
                        m.position.pieces[8] &= ~1ULL;
                        m.position.pieces[8] |= (1ULL << 3);
                        m.position.black_pieces &= ~(1ULL << 4);
                        m.position.black_pieces &= ~1ULL;
                        m.position.black_pieces |= (1ULL << 2);
                        m.position.black_pieces |= (1ULL << 3);
                        // compute all pieces bitboard and the covered squares
                        m.position.all_pieces = m.position.white_pieces | m.position.black_pieces;
                        m.position.white_covered_squares = GetCoveredSquares(m.position.pieces, m.position.all_pieces, true);
                        m.position.black_covered_squares = GetCoveredSquares(m.position.pieces, m.position.all_pieces, false);
                        m.position.half_move_counter++;
                        // standard updates
                        m.position.white_to_move = true;
                        m.position.can_black_castle_kingside = false;
                        m.position.can_black_castle_queenside = false;
                        m.position.en_passant_target_square = 0ULL;
                        // check if the move is a check
                        is_check = m.position.pieces[0] & m.position.black_covered_squares;
                        if(is_check){ flags += 16; }   
                        m.move = EncodeMove(4, 2, 6/*king*/, 15/*no capture*/, 15/*no promotion*/, flags); 
                        all_moves[move_index] = m;
                        move_index++;
                   } 
                }
            }   
        }
    }

    pos.n_legal_moves = move_index;
}