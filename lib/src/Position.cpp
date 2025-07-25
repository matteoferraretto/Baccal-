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
            pos.white_material_value += WHITE_KING_VALUE;
            bit_set(pos.pieces[0], i, j);
            bit_set(pos.white_pieces, i, j);
            pos.white_covered_squares |= king_covered_squares_bitboards[square];
        }
        else if(c == 'Q'){ 
            pos.white_material_value += WHITE_QUEEN_VALUE;
            bit_set(pos.pieces[1], i, j);
            bit_set(pos.white_pieces, i, j);
            // calculation of covered squares is delayed for sliding pieces --> see below
        }
        else if(c == 'R'){ 
            pos.white_material_value += WHITE_ROOK_VALUE;
            bit_set(pos.pieces[2], i, j);
            bit_set(pos.white_pieces, i, j);
            // calculation of covered squares is delayed for sliding pieces --> see below
        }
        else if(c == 'B'){ 
            pos.white_material_value += WHITE_BISHOP_VALUE;
            bit_set(pos.pieces[3], i, j);
            bit_set(pos.white_pieces, i, j);
            // calculation of covered squares is delayed for sliding pieces --> see below
        }
        else if(c == 'N'){ 
            pos.white_material_value += WHITE_KNIGHT_VALUE;
            bit_set(pos.pieces[4], i, j);
            bit_set(pos.white_pieces, i, j);
            pos.white_covered_squares |= knight_covered_squares_bitboards[square];
        }
        else if(c == 'P'){ 
            pos.white_material_value += WHITE_PAWN_VALUE;
            bit_set(pos.pieces[5], i, j);
            bit_set(pos.white_pieces, i, j);
            pos.white_covered_squares |= white_pawn_covered_squares_bitboards[square];
        }
        else if(c == 'k'){ 
            pos.black_material_value += BLACK_KING_VALUE;
            bit_set(pos.pieces[6], i, j);
            bit_set(pos.black_pieces, i, j);
            pos.black_covered_squares |= king_covered_squares_bitboards[square];
        }
        else if(c == 'q'){ 
            pos.black_material_value += BLACK_QUEEN_VALUE;
            bit_set(pos.pieces[7], i, j);
            bit_set(pos.black_pieces, i, j);
            // calculation of covered squares is delayed for sliding pieces --> see below
        }
        else if(c == 'r'){ 
            pos.black_material_value += BLACK_ROOK_VALUE;
            bit_set(pos.pieces[8], i, j);
            bit_set(pos.black_pieces, i, j);
            // calculation of covered squares is delayed for sliding pieces --> see below
        }
        else if(c == 'b'){ 
            pos.black_material_value += BLACK_BISHOP_VALUE;
            bit_set(pos.pieces[9], i, j);
            bit_set(pos.black_pieces, i, j);
            // calculation of covered squares is delayed for sliding pieces --> see below
        }
        else if(c == 'n'){ 
            pos.black_material_value += BLACK_KNIGHT_VALUE;
            bit_set(pos.pieces[10], i, j);
            bit_set(pos.black_pieces, i, j);
            pos.black_covered_squares |= knight_covered_squares_bitboards[square];
        }
        else if(c == 'p'){ 
            pos.black_material_value += BLACK_PAWN_VALUE;
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
        pos.en_passant_target_file = (unsigned int)(words[3].at(0)) - 97;
    }

    // set half move counter and move counter from fen
    pos.half_move_counter = std::stoi(words[4]);
    pos.move_counter = std::stoi(words[5]);

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
int Score(Position pos){
    // we assume that material value is pre-calculated! It should be done when a position is generated
    int score = pos.white_material_value + pos.black_material_value;
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
        score += pawnPST[square];
        clear_last_active_bit(piece);   
        // add malus for doubled or isolated pawns ...
        // bonus for passed pawns ...
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
        score -= pawnPST[56 - square + 2*(square%8)];
        clear_last_active_bit(piece);
        // malus for doubled or isolated pawns...
        // bonus for passed pawns
    }
    
    return score;
}

// FIND LIST OF ALL MOVES (regardless if the move leaves the king in check)
std::vector<Move> AllMoves(const Position& pos){
    std::vector<Move> all_moves;
    all_moves.reserve(216); // max estimated moves
    Move move = 0;
    bool is_capture;
    uint64_t piece;
    uint64_t hash_index_rook, hash_index_bishop;
    uint64_t attacks = 0ULL;
    unsigned long square, target_square;
    uint8_t flags = 0;
    uint8_t piece_index, captured_piece_index;

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
                _BitScanForward64(&target_square, attacks); // find the target square
                is_capture = bit_get(pos.black_pieces, target_square);
                captured_piece_index = 15; // no capture 
                if(is_capture){
                    flags = 8; // capture flag
                    // check what black piece has been captured
                    for(uint8_t index = 6; index < 11; index++){
                        if(bit_get(pos.pieces[index], target_square)){
                            captured_piece_index = index;
                            break;
                        }
                    }
                }
                // encode move 
                move = EncodeMove(static_cast<uint8_t>(square), static_cast<uint8_t>(target_square), piece_index, captured_piece_index, 15/*no promotion*/, flags);
                all_moves.push_back(move);
                clear_last_active_bit(attacks); // remove considered attack
            }
            // remove considered piece
            clear_last_active_bit(piece);
        }

        // Queen move
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
                _BitScanForward64(&target_square, attacks); // find the target square
                is_capture = bit_get(pos.black_pieces, target_square);
                captured_piece_index = 15; // no capture 
                if(is_capture){
                    flags = 8; // capture flag
                    // check what black piece has been captured
                    for(uint8_t index = 6; index < 11; index++){
                        if(bit_get(pos.pieces[index], target_square)){
                            captured_piece_index = index;
                            break;
                        }
                    }
                }
                // encode move 
                move = EncodeMove(static_cast<uint8_t>(square), static_cast<uint8_t>(target_square), piece_index, captured_piece_index, 15/*no promotion*/, flags);
                all_moves.push_back(move);
                clear_last_active_bit(attacks); // remove considered attack
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
                _BitScanForward64(&target_square, attacks); // find the target square
                is_capture = bit_get(pos.black_pieces, target_square);
                captured_piece_index = 15; // no capture 
                if(is_capture){
                    flags = 8; // capture flag
                    // check what black piece has been captured
                    for(uint8_t index = 6; index < 11; index++){
                        if(bit_get(pos.pieces[index], target_square)){
                            captured_piece_index = index;
                            break;
                        }
                    }
                }
                // encode move 
                move = EncodeMove(static_cast<uint8_t>(square), static_cast<uint8_t>(target_square), piece_index, captured_piece_index, 15/*no promotion*/, flags);
                all_moves.push_back(move);
                clear_last_active_bit(attacks); // remove considered attack
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
                _BitScanForward64(&target_square, attacks); // find the target square
                is_capture = bit_get(pos.black_pieces, target_square);
                captured_piece_index = 15; // no capture 
                if(is_capture){
                    flags = 8; // capture flag
                    // check what black piece has been captured
                    for(uint8_t index = 6; index < 11; index++){
                        if(bit_get(pos.pieces[index], target_square)){
                            captured_piece_index = index;
                            break;
                        }
                    }
                }
                // encode move 
                move = EncodeMove(static_cast<uint8_t>(square), static_cast<uint8_t>(target_square), piece_index, captured_piece_index, 15/*no promotion*/, flags);
                all_moves.push_back(move);
                clear_last_active_bit(attacks); // remove considered attack
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
                _BitScanForward64(&target_square, attacks); // find the target square
                is_capture = bit_get(pos.black_pieces, target_square);
                captured_piece_index = 15; // no capture 
                if(is_capture){
                    flags = 8; // capture flag
                    // check what black piece has been captured
                    for(uint8_t index = 6; index < 11; index++){
                        if(bit_get(pos.pieces[index], target_square)){
                            captured_piece_index = index;
                            break;
                        }
                    }
                }
                // encode move 
                move = EncodeMove(static_cast<uint8_t>(square), static_cast<uint8_t>(target_square), piece_index, captured_piece_index, 15/*no promotion*/, flags);
                all_moves.push_back(move);
                clear_last_active_bit(attacks); // remove considered attack
            }
            // remove considered piece
            clear_last_active_bit(piece);
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
                _BitScanForward64(&target_square, attacks); // find the target square
                is_capture = bit_get(pos.white_pieces, target_square);
                captured_piece_index = 15; // no capture 
                if(is_capture){
                    flags = 8; // capture flag
                    // check what black piece has been captured
                    for(uint8_t index = 6; index < 11; index++){
                        if(bit_get(pos.pieces[index], target_square)){
                            captured_piece_index = index;
                            break;
                        }
                    }
                }
                // encode move 
                move = EncodeMove(static_cast<uint8_t>(square), static_cast<uint8_t>(target_square), piece_index, captured_piece_index, 15/*no promotion*/, flags);
                all_moves.push_back(move);
                clear_last_active_bit(attacks); // remove considered attack
            }
            // remove considered piece
            clear_last_active_bit(piece);
        }

        // Queen move
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
                _BitScanForward64(&target_square, attacks); // find the target square
                is_capture = bit_get(pos.white_pieces, target_square);
                captured_piece_index = 15; // no capture 
                if(is_capture){
                    flags = 8; // capture flag
                    // check what black piece has been captured
                    for(uint8_t index = 0; index < 6; index++){
                        if(bit_get(pos.pieces[index], target_square)){
                            captured_piece_index = index;
                            break;
                        }
                    }
                }
                // encode move 
                move = EncodeMove(static_cast<uint8_t>(square), static_cast<uint8_t>(target_square), piece_index, captured_piece_index, 15/*no promotion*/, flags);
                all_moves.push_back(move);
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
                _BitScanForward64(&target_square, attacks); // find the target square
                is_capture = bit_get(pos.white_pieces, target_square);
                captured_piece_index = 15; // no capture 
                if(is_capture){
                    flags = 8; // capture flag
                    // check what black piece has been captured
                    for(uint8_t index = 0; index < 6; index++){
                        if(bit_get(pos.pieces[index], target_square)){
                            captured_piece_index = index;
                            break;
                        }
                    }
                }
                // encode move 
                move = EncodeMove(static_cast<uint8_t>(square), static_cast<uint8_t>(target_square), piece_index, captured_piece_index, 15/*no promotion*/, flags);
                all_moves.push_back(move);
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
                _BitScanForward64(&target_square, attacks); // find the target square
                is_capture = bit_get(pos.white_pieces, target_square);
                captured_piece_index = 15; // no capture 
                if(is_capture){
                    flags = 8; // capture flag
                    // check what black piece has been captured
                    for(uint8_t index = 0; index < 6; index++){
                        if(bit_get(pos.pieces[index], target_square)){
                            captured_piece_index = index;
                            break;
                        }
                    }
                }
                // encode move 
                move = EncodeMove(static_cast<uint8_t>(square), static_cast<uint8_t>(target_square), piece_index, captured_piece_index, 15/*no promotion*/, flags);
                all_moves.push_back(move);
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
                _BitScanForward64(&target_square, attacks); // find the target square
                is_capture = bit_get(pos.white_pieces, target_square);
                captured_piece_index = 15; // no capture 
                if(is_capture){
                    flags = 8; // capture flag
                    // check what black piece has been captured
                    for(uint8_t index = 0; index < 6; index++){
                        if(bit_get(pos.pieces[index], target_square)){
                            captured_piece_index = index;
                            break;
                        }
                    }
                }
                // encode move 
                move = EncodeMove(static_cast<uint8_t>(square), static_cast<uint8_t>(target_square), piece_index, captured_piece_index, 15/*no promotion*/, flags);
                all_moves.push_back(move);
                clear_last_active_bit(attacks); // remove considered attack
            }
            // remove considered piece
            clear_last_active_bit(piece);
        }

    }

    return all_moves;
}