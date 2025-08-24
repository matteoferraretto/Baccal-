#pragma once
#include <Position.h>
#include <Utilities.h>
#include <Bitboards.h>
#include <TranspositionTable.h>
#include <iostream>
#include <sstream>
#include <cstdint>
#include <intrin.h>
#include <vector>
#include <assert.h>

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
    int c_casted = 0; 
    // loop over characters of the FEN string
    for(char& c: words[0]){
        c_casted = c - '0';
        if(c_casted == 8) { continue; } // if c is 8, ignore
        if(c_casted == -1) { i++; j = 0; continue; } // if c is '/', increment row index and reset column index to 0
        if(c_casted > 0 && c_casted < 8) { j += c_casted; continue; }
        square = 8*i + j;
        // depending on the piece, determine the masks
        if(c == 'K'){ 
            bit_set(pos.pieces[WHITE_KING], square);
            bit_set(pos.white_pieces, square);
        }
        else if(c == 'Q'){ 
            bit_set(pos.pieces[WHITE_QUEEN], square);
            bit_set(pos.white_pieces, square);
        }
        else if(c == 'R'){ 
            bit_set(pos.pieces[WHITE_ROOK], square);
            bit_set(pos.white_pieces, square);
        }
        else if(c == 'B'){ 
            bit_set(pos.pieces[WHITE_BISHOP], square);
            bit_set(pos.white_pieces, square);
        }
        else if(c == 'N'){ 
            bit_set(pos.pieces[WHITE_KNIGHT], square);
            bit_set(pos.white_pieces, square);
        }
        else if(c == 'P'){ 
            bit_set(pos.pieces[WHITE_PAWN], square);
            bit_set(pos.white_pieces, square);
        }
        else if(c == 'k'){ 
            bit_set(pos.pieces[BLACK_KING], square);
            bit_set(pos.black_pieces, square);
        }
        else if(c == 'q'){ 
            bit_set(pos.pieces[BLACK_QUEEN], square);
            bit_set(pos.black_pieces, square);
        }
        else if(c == 'r'){ 
            bit_set(pos.pieces[BLACK_ROOK], square);
            bit_set(pos.black_pieces, square);
        }
        else if(c == 'b'){ 
            bit_set(pos.pieces[BLACK_BISHOP], square);
            bit_set(pos.black_pieces, square);
        }
        else if(c == 'n'){ 
            bit_set(pos.pieces[BLACK_KNIGHT], square);
            bit_set(pos.black_pieces, square);
        }
        else if(c == 'p'){ 
            bit_set(pos.pieces[BLACK_PAWN], square);
            bit_set(pos.black_pieces, square);
        }
        j++; 
    }

    // set side to move
    if(words[1] == "w") pos.white_to_move = true; 
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
    //pos.move_counter = std::stoi(words[5]);

    // mask of all pieces
    pos.all_pieces = pos.white_pieces | pos.black_pieces;
    // Zobrist key
    pos.zobrist_key = ZobristHashing(pos);
    
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
    Bitboard piece = 0ULL;
    uint8_t n_pieces = 0; // counts only queens, rooks, bishops, knights. Used for detecting finals
        
    // white queen
    piece = pos.pieces[WHITE_QUEEN];
    while(piece){ // loop until all the white queens are considered
        _BitScanForward64(&square, piece);
        score += QUEEN_PST[square];
        n_pieces++;
        clear_last_active_bit(piece);   
    }
    // white rook
    piece = pos.pieces[WHITE_ROOK];
    while(piece){ // loop until all the white queens are considered
        _BitScanForward64(&square, piece);
        score += WHITE_ROOK_PST[square];
        n_pieces++;
        clear_last_active_bit(piece);   
    }
    // white bishop
    piece = pos.pieces[WHITE_BISHOP];
    while(piece){ // loop until all the white queens are considered
        _BitScanForward64(&square, piece);
        score += BISHOP_PST[square];
        n_pieces++;
        clear_last_active_bit(piece);   
    }
    // white knight
    piece = pos.pieces[WHITE_KNIGHT];
    while(piece){ // loop until all the white queens are considered
        _BitScanForward64(&square, piece);
        score += WHITE_KNIGHT_PST[square];
        n_pieces++;
        clear_last_active_bit(piece);   
        // bonus for outpost 
        if(mask_white_passed_pawn[square] & pos.pieces[BLACK_PAWN])
            continue;
        else
            score += BONUS_FOR_OUTPOST;
    }
    // white pawns
    piece = pos.pieces[WHITE_PAWN];
    while(piece){ // loop until all the white queens are considered
        _BitScanForward64(&square, piece);
        score += WHITE_PAWN_PST[square];
        clear_last_active_bit(piece);   
        // bonus for passed pawns ...
        if(mask_white_passed_pawn[square] & pos.pieces[BLACK_PAWN]) 
            continue; 
        else
            score += BONUS_FOR_PASSED_PAWNS;
    }
    // black queen
    piece = pos.pieces[BLACK_QUEEN];
    while(piece){ // loop until all the white queens are considered
        _BitScanForward64(&square, piece);
        score -= QUEEN_PST[square];
        n_pieces++;
        clear_last_active_bit(piece);   
    }
    // black rook
    piece = pos.pieces[BLACK_ROOK];
    while(piece){ // loop until all the white queens are considered
        _BitScanForward64(&square, piece);
        score -= BLACK_ROOK_PST[square];
        n_pieces++;
        clear_last_active_bit(piece);   
    }
    // black bishop
    piece = pos.pieces[BLACK_BISHOP];
    while(piece){ // loop until all the white queens are considered
        _BitScanForward64(&square, piece);
        score -= BISHOP_PST[square];
        n_pieces++;
        clear_last_active_bit(piece);   
    }
    // black knight
    piece = pos.pieces[BLACK_KNIGHT];
    while(piece){ // loop until all the white queens are considered
        _BitScanForward64(&square, piece);
        score -= BLACK_KNIGHT_PST[square];
        n_pieces++;
        clear_last_active_bit(piece);   
        // bonus for outpost 
        if(mask_black_passed_pawn[square] & pos.pieces[WHITE_PAWN])
            continue;
        else
            score -= BONUS_FOR_OUTPOST;
    }
    // black pawns
    piece = pos.pieces[BLACK_PAWN];
    while(piece){ // loop until all the white queens are considered
        _BitScanForward64(&square, piece);
        score -= BLACK_PAWN_PST[square];
        n_pieces++;
        clear_last_active_bit(piece);
        // bonus for passed pawns: the black pawn looks forward and if no white pawns are found, it is a passer
        if(mask_black_passed_pawn[square] & pos.pieces[WHITE_PAWN])
            continue;
        else
            score -= BONUS_FOR_PASSED_PAWNS;
    }

    // we use 14 - n_pieces to detect if we are entering an endgame
    // 14 - n_pieces = 0 --> beginning of the game
    // 14 - n_pieces = 14 --> only pawns remaining (endgame)
    // so we can define:
    //   lambda = n_pieces / 14
    //   lambda * middlegame_PST + (1 - lambda) * endgame_PST
    float lambda = n_pieces/14.0f;
    // white king
    piece = pos.pieces[WHITE_KING];
    if(piece){
        _BitScanForward64(&square, piece);
        score += (int)( lambda * WHITE_KING_PST_MIDDLEGAME[square] + (1.0 - lambda) * KING_PST_ENDGAME[square] );
    }
    // black king
    piece = pos.pieces[BLACK_KING];
    if(piece){
        _BitScanForward64(&square, piece);
        score -= static_cast<int>( lambda * BLACK_KING_PST_MIDDLEGAME[square] + (1.0 - lambda) * KING_PST_ENDGAME[square] );
    }

    // malus for doubled pawns (or bonus for opponent's doubled pawns):
    score += static_cast<int>((
        count_doubled_pawns(pos.pieces[BLACK_PAWN]) - count_doubled_pawns(pos.pieces[WHITE_PAWN])
    ) * MALUS_FOR_DOUBLED_PAWNS); 

    return score;
}

const Bitboard SQUARES_BETWEEN_WHITE_QUEENSIDE_CASTLING = 7ULL << 57;
const Bitboard SQUARES_BETWEEN_WHITE_KINGSIDE_CASTLING = 3ULL << 61;
const Bitboard WHITE_H_ROOK_STARTING_SQUARE_BB = 1ULL << 63;
const Bitboard WHITE_A_ROOK_STARTING_SQUARE_BB = 1ULL << 56;

void PseudoLegalMoves(Position& pos, Move* moves){
    int move_index = 0;
    Bitboard piece = 0ULL;
    unsigned long square, target_square;
    Bitboard attacks = 0ULL;
    uint16_t flags = 0;
    MaskAndMagic mm;
    Bitboard not_your_pieces = 0ULL;
    uint64_t hash = 0ULL;

    // WHITE MOVES
    if(pos.white_to_move){

        not_your_pieces = ~pos.white_pieces;

        // King
        piece = pos.pieces[WHITE_KING];
        while(piece){
            // find position of piece and assign it to square
            _BitScanForward64(&square, piece); 
            attacks = king_covered_squares_bitboards[square]; // retrieve attack bitboard
            attacks &= not_your_pieces; // exclude self-capture
            while(attacks){
                _BitScanForward64(&target_square, attacks); // find the target square
                flags = bit_get(pos.black_pieces, target_square) ? 4 : 0;
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
        piece = pos.pieces[WHITE_QUEEN]; // retrieve bitboard of queens
        while(piece){ // consider all the queens
            _BitScanForward64(&square, piece); // find position of queen and assign it to square
            // retrieve bitboard of queen moves
            mm = rook_mm[square];
            hash = ((pos.all_pieces & mm.mask) * mm.magic) >> SHIFT_ROOK;
            attacks = rook_covered_squares_bb[square][hash];
            mm = bishop_mm[square];
            hash = ((pos.all_pieces & mm.mask) * mm.magic) >> SHIFT_BISHOP;
            attacks |= bishop_covered_squares_bb[square][hash];
            attacks &= not_your_pieces; // excluse self-capture
            while(attacks){
                _BitScanForward64(&target_square, attacks); // find the target square
                flags = bit_get(pos.black_pieces, target_square) ? 4 : 0;
                moves[move_index] = EncodeMove(square, target_square, flags);
                move_index++;
                clear_last_active_bit(attacks); 
            }
            clear_last_active_bit(piece);
        }

        // Rook
        piece = pos.pieces[WHITE_ROOK];
        while(piece){
            _BitScanForward64(&square, piece); // find position of queen and assign it to square
            // retrieve bitboard of queen moves
            mm = rook_mm[square];
            hash = ((pos.all_pieces & mm.mask) * mm.magic) >> SHIFT_ROOK;
            attacks = rook_covered_squares_bb[square][hash];
            attacks &= not_your_pieces; // excluse self-capture
            while(attacks){
                _BitScanForward64(&target_square, attacks); // find the target square
                flags = bit_get(pos.black_pieces, target_square) ? 4 : 0;
                moves[move_index] = EncodeMove(square, target_square, flags);
                move_index++;
                clear_last_active_bit(attacks); 
            }
            clear_last_active_bit(piece);
        }

        // Bishop
        piece = pos.pieces[WHITE_BISHOP]; // retrieve bitboard of queens
        while(piece){ // consider all the queens
            _BitScanForward64(&square, piece); // find position of queen and assign it to square
            mm = bishop_mm[square];
            hash = ((pos.all_pieces & mm.mask) * mm.magic) >> SHIFT_BISHOP;
            attacks = bishop_covered_squares_bb[square][hash];
            attacks &= not_your_pieces; // excluse self-capture
            while(attacks){
                _BitScanForward64(&target_square, attacks); // find the target square
                flags = bit_get(pos.black_pieces, target_square) ? 4 : 0;
                moves[move_index] = EncodeMove(square, target_square, flags);
                move_index++;
                clear_last_active_bit(attacks); 
            }
            clear_last_active_bit(piece);
        }

        // Knight
        piece = pos.pieces[WHITE_KNIGHT]; // retrieve bitboard of knights
        while(piece){ // consider all the knights
            _BitScanForward64(&square, piece); // find position of piece and assign it to square
            attacks = knight_covered_squares_bitboards[square]; // retrieve attack bitboard
            attacks &= not_your_pieces; // exclude self-capture
            while(attacks){
                _BitScanForward64(&target_square, attacks); // find the target square
                flags = bit_get(pos.black_pieces, target_square) ? 4 : 0;
                moves[move_index] = EncodeMove(square, target_square, flags);
                move_index++;
                clear_last_active_bit(attacks); 
            }
            // remove considered piece
            clear_last_active_bit(piece);
        }

        // Pawn capture
        piece = pos.pieces[WHITE_PAWN];
        while(piece){
            _BitScanForward64(&square, piece); // find position of piece and assign it to square
            // NORMAL CAPTURES
            attacks = white_pawn_covered_squares_bitboards[square]; // retrieve attack bitboard
            attacks &= pos.black_pieces; // only attacked squares occupied by enemy pieces are valid for movement
            while(attacks){
                _BitScanForward64(&target_square, attacks); // find the target square
                // in case of promotion, loop over all possible promoted pieces
                if(WHITE_PAWN_IN_FINAL_RANK[target_square]){
                    for(int promoted_piece_index = WHITE_QUEEN; promoted_piece_index < WHITE_PAWN; promoted_piece_index++){
                        flags = 16 - promoted_piece_index;
                        moves[move_index] = EncodeMove(square, target_square, flags);
                        move_index++;
                    }
                }
                else{ 
                    moves[move_index] = EncodeMove(square, target_square, 4);
                    move_index++;
                }
                clear_last_active_bit(attacks); // remove considered attack
            }
            // EN PASSANT CAPTURES
            attacks = white_pawn_covered_squares_bitboards[square]; // retrieve attack bitboard
            attacks &= pos.en_passant_target_square; // only attacked squares occupied by enemy pieces are valid for movement
            while(attacks){
                _BitScanForward64(&target_square, attacks); // find the target square            
                //flags = 5;
                moves[move_index] = EncodeMove(square, target_square, 5);
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
            if(WHITE_PAWN_IN_STARTING_RANK[square]){
                if(!bit_get(attacks, square - 8) && bit_get(attacks, square - 16))
                    attacks = 0ULL;
            }
            while(attacks){
                _BitScanForward64(&target_square, attacks); // find the target square
                // in case of promotion, loop over all possible promoted pieces
                if(WHITE_PAWN_IN_FINAL_RANK[target_square]){
                    for(int promoted_piece_index = WHITE_QUEEN; promoted_piece_index < WHITE_PAWN; promoted_piece_index++){
                        moves[move_index] = EncodeMove(square, target_square, 12 - promoted_piece_index);
                        move_index++;
                    }
                }
                // if this is a double push, flag it to manage en-passant target squares later
                else if(RANK_OF_SQUARE[target_square] == 4 && WHITE_PAWN_IN_STARTING_RANK[square]){
                    moves[move_index] = EncodeMove(square, target_square, 1);
                    move_index++;
                }
                else{
                    moves[move_index] = EncodeMove(square, target_square, 0);
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
            if(pos.pieces[WHITE_ROOK] & WHITE_H_ROOK_STARTING_SQUARE_BB){
                if(!(pos.all_pieces & SQUARES_BETWEEN_WHITE_KINGSIDE_CASTLING)){
                    moves[move_index] = EncodeMove(60, 62, 2); 
                    move_index++;
                } 
            }   
        }
        // Castles queenside
        if(pos.can_white_castle_queenside){ 
            if(pos.pieces[WHITE_ROOK] & WHITE_A_ROOK_STARTING_SQUARE_BB){
                if(!(pos.all_pieces & SQUARES_BETWEEN_WHITE_QUEENSIDE_CASTLING)){
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
        piece = pos.pieces[BLACK_KING];
        while(piece){
            // find position of piece and assign it to square
            _BitScanForward64(&square, piece); 
            attacks = king_covered_squares_bitboards[square]; // retrieve attack bitboard
            attacks &= not_your_pieces; // exclude self-capture
            while(attacks){
                _BitScanForward64(&target_square, attacks); // find the target square
                flags = bit_get(pos.white_pieces, target_square) ? 4 : 0;
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
        piece = pos.pieces[BLACK_QUEEN]; // retrieve bitboard of queens
        while(piece){ // consider all the queens
            _BitScanForward64(&square, piece); // find position of queen and assign it to square
            // retrieve bitboard of queen moves
            mm = rook_mm[square];
            hash = ((pos.all_pieces & mm.mask) * mm.magic) >> SHIFT_ROOK;
            attacks = rook_covered_squares_bb[square][hash];
            mm = bishop_mm[square];
            hash = ((pos.all_pieces & mm.mask) * mm.magic) >> SHIFT_BISHOP;
            attacks |= bishop_covered_squares_bb[square][hash];
            attacks &= not_your_pieces; // excluse self-capture
            while(attacks){
                _BitScanForward64(&target_square, attacks); // find the target square
                flags = bit_get(pos.white_pieces, target_square) ? 4 : 0;
                moves[move_index] = EncodeMove(square, target_square, flags);
                move_index++;
                clear_last_active_bit(attacks); 
            }
            clear_last_active_bit(piece);
        }

        // Rook
        piece = pos.pieces[BLACK_ROOK];
        while(piece){
            _BitScanForward64(&square, piece); // find position of queen and assign it to square
            // retrieve bitboard of queen moves
            mm = rook_mm[square];
            hash = ((pos.all_pieces & mm.mask) * mm.magic) >> SHIFT_ROOK;
            attacks = rook_covered_squares_bb[square][hash];
            attacks &= not_your_pieces; // excluse self-capture
            while(attacks){
                _BitScanForward64(&target_square, attacks); // find the target square
                flags = bit_get(pos.white_pieces, target_square) ? 4 : 0;
                moves[move_index] = EncodeMove(square, target_square, flags);
                move_index++;
                clear_last_active_bit(attacks); 
            }
            clear_last_active_bit(piece);
        }

        // Bishop
        piece = pos.pieces[BLACK_BISHOP]; // retrieve bitboard of queens
        while(piece){ // consider all the queens
            _BitScanForward64(&square, piece); // find position of queen and assign it to square
            mm = bishop_mm[square];
            hash = ((pos.all_pieces & mm.mask) * mm.magic) >> SHIFT_BISHOP;
            attacks = bishop_covered_squares_bb[square][hash];
            attacks &= not_your_pieces; // excluse self-capture
            while(attacks){
                _BitScanForward64(&target_square, attacks); // find the target square
                flags = bit_get(pos.white_pieces, target_square) ? 4 : 0;
                moves[move_index] = EncodeMove(square, target_square, flags);
                move_index++;
                clear_last_active_bit(attacks); 
            }
            clear_last_active_bit(piece);
        }

        // Knight
        piece = pos.pieces[BLACK_KNIGHT]; // retrieve bitboard of knights
        while(piece){ // consider all the knights
            _BitScanForward64(&square, piece); // find position of piece and assign it to square
            attacks = knight_covered_squares_bitboards[square]; // retrieve attack bitboard
            attacks &= not_your_pieces; // exclude self-capture
            while(attacks){
                _BitScanForward64(&target_square, attacks); // find the target square
                flags = bit_get(pos.white_pieces, target_square) ? 4 : 0;  
                moves[move_index] = EncodeMove(square, target_square, flags);
                move_index++;
                clear_last_active_bit(attacks); 
            }
            // remove considered piece
            clear_last_active_bit(piece);
        }

        // Pawn capture
        piece = pos.pieces[BLACK_PAWN];
        while(piece){
            _BitScanForward64(&square, piece); // find position of piece and assign it to square
            // NORMAL CAPTURES
            attacks = black_pawn_covered_squares_bitboards[square]; // retrieve attack bitboard
            attacks &= pos.white_pieces; // only attacked squares occupied by enemy pieces are valid for movement
            while(attacks){
                _BitScanForward64(&target_square, attacks); // find the target square
                // in case of promotion, loop over all possible promoted pieces
                if(BLACK_PAWN_IN_FINAL_RANK[target_square]){
                    for(int promoted_piece_index = BLACK_QUEEN; promoted_piece_index < BLACK_PAWN; promoted_piece_index++){
                        moves[move_index] = EncodeMove(square, target_square, 22 - promoted_piece_index);
                        move_index++;
                    }
                }
                else{ 
                    moves[move_index] = EncodeMove(square, target_square, 4);
                    move_index++;
                }
                clear_last_active_bit(attacks); // remove considered attack
            }
            // EN PASSANT CAPTURES
            attacks = black_pawn_covered_squares_bitboards[square]; // retrieve attack bitboard
            attacks &= pos.en_passant_target_square; // only attacked squares occupied by enemy pieces are valid for movement
            while(attacks){
                _BitScanForward64(&target_square, attacks); // find the target square
                moves[move_index] = EncodeMove(square, target_square, 5);
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
                    for(int promoted_piece_index = BLACK_QUEEN; promoted_piece_index < BLACK_PAWN; promoted_piece_index++){
                        flags = 22 - promoted_piece_index;
                        moves[move_index] = EncodeMove(square, target_square, flags);
                        move_index++;
                    }
                }
                // if this is a double push, flag it to manage en-passant target squares later
                else if(RANK_OF_SQUARE[target_square] == 3 && BLACK_PAWN_IN_STARTING_RANK[square]){
                    moves[move_index] = EncodeMove(square, target_square, 1);
                    move_index++;
                }
                else{
                    moves[move_index] = EncodeMove(square, target_square, 0);
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
            if(pos.pieces[BLACK_ROOK] & 128ULL){
                if(!(pos.all_pieces & 96ULL)){
                    moves[move_index] = EncodeMove(4, 6, 2);
                    move_index++;
                }
            }   
        }

        // Castles queenside
        if(pos.can_black_castle_queenside){ 
            if(pos.pieces[BLACK_ROOK] & 1ULL){
                if(!(pos.all_pieces & 14ULL)){ // no pieces between king and rook 
                    moves[move_index] = EncodeMove(4, 2, 3); 
                    move_index++;
                }
            }   
        }
    }
    pos.n_pseudolegal_moves = move_index;
}



const Bitboard WHITE_ROOK_KINGSIDE_CASTLE_MASK = (1ULL << 61) | (1ULL << 63);
const Bitboard WHITE_ROOK_QUEENSIDE_CASTLE_MASK = (1ULL << 56) | (1ULL << 59);
const Bitboard BLACK_ROOK_KINGSIDE_CASTLE_MASK = (1ULL << 7) | (1ULL << 5);
const Bitboard BLACK_ROOK_QUEENSIDE_CASTLE_MASK = (1ULL << 3) | 1ULL;


void MakeMove(Position& pos, const Move& move, StateMemory& state){
    int moved_piece_index = NO_PIECE, captured_piece_index = NO_PIECE, promoted_piece_index = NO_PIECE;
    // retrieve info from the move
    uint64_t from = static_cast<uint64_t>(move & 0b0000000000111111);
    uint64_t to = static_cast<uint64_t>((move >> 6) & 0b0000000000111111);
    int flags = (move >> 12);
    // starting square and ending square bitboards
    Bitboard from_bb = (1ULL << from);
    Bitboard to_bb = (1ULL << to); 
    unsigned long ep_square;
    //state.zobrist_key = pos.zobrist_key;

    // WHITE TO MOVE
    if(pos.white_to_move){
        // retrieve what piece has moved
        for(int piece_index = WHITE_KING; piece_index <= WHITE_PAWN; piece_index++){
            if(pos.pieces[piece_index] & from_bb){
                moved_piece_index = piece_index;
                break;
            }
        }
        // retrieve info about captured piece (if any)
        for(int piece_index = BLACK_QUEEN; piece_index <= BLACK_PAWN; piece_index++){
            // standard capture
            if(pos.pieces[piece_index] & to_bb){
                captured_piece_index = piece_index;
                break;
            } 
            // en-passant capture
            else if(flags == EN_PASSANT){
                captured_piece_index = BLACK_PAWN; // <--- necessarily a pawn
                break;
            }
        }

        // EN - PASSANT TARGET
        // if there's an active e.p. target square, un-hash it
        if(pos.en_passant_target_square){ // un-hash
            _BitScanForward64(&ep_square, pos.en_passant_target_square);
            pos.zobrist_key ^= zobrist_table.en_passant_file[ep_square % 8];
        }
        // save current target square in memory
        state.en_passant_target_square = pos.en_passant_target_square;
        // if double pawn push, set new target
        if(flags == DOUBLE_PAWN_PUSH){
            // update e.p. target
            pos.en_passant_target_square = 1ULL << (to + 8);
            // hash the new target
            pos.zobrist_key ^= zobrist_table.en_passant_file[to % 8]; // to % 8 = (to + 8) % 8
        }
        else{
            pos.en_passant_target_square = 0ULL;
        }

        // PROMOTIONS
        // retrieve info about promoted piece (if any)
        if(flags == PROMOTION_QUEEN || flags == PROMOTION_QUEEN_CAPTURE)
            promoted_piece_index = WHITE_QUEEN; 
        else if(flags == PROMOTION_ROOK || flags == PROMOTION_ROOK_CAPTURE)
            promoted_piece_index = WHITE_ROOK; 
        else if(flags == PROMOTION_BISHOP || flags == PROMOTION_BISHOP_CAPTURE)
            promoted_piece_index = WHITE_BISHOP;
        else if(flags == PROMOTION_KNIGHT || flags == PROMOTION_KNIGHT_CAPTURE)
            promoted_piece_index = WHITE_KNIGHT;
        // save current state
        state.moved_piece_index = moved_piece_index;
        state.captured_piece_index = captured_piece_index;
        state.promoted_piece_index = promoted_piece_index;
        // move the piece 
        pos.pieces[moved_piece_index] ^= from_bb | to_bb;
        pos.white_pieces ^= from_bb | to_bb;
        pos.zobrist_key ^= zobrist_table.pieces_and_squares[moved_piece_index][from];
        pos.zobrist_key ^= zobrist_table.pieces_and_squares[moved_piece_index][to];
        // remove captured piece, if any
        if(captured_piece_index != NO_PIECE){
            // if en passant, piece is not in the target square
            if(flags == EN_PASSANT){
                bit_clear(pos.pieces[BLACK_PAWN], to + 8);
                bit_clear(pos.black_pieces, to + 8);
                pos.zobrist_key ^= zobrist_table.pieces_and_squares[BLACK_PAWN][to + 8];
            }
            // if normal capture:
            else{
                bit_clear(pos.pieces[captured_piece_index], to);
                bit_clear(pos.black_pieces, to);
                pos.zobrist_key ^= zobrist_table.pieces_and_squares[captured_piece_index][to];
            }
        }
        // spawn the promoted piece in case of promotion and remove the pawn
        if(promoted_piece_index != NO_PIECE){
            bit_clear(pos.pieces[WHITE_PAWN], to);
            bit_set(pos.pieces[promoted_piece_index], to);
            pos.zobrist_key ^= zobrist_table.pieces_and_squares[WHITE_PAWN][to];
            pos.zobrist_key ^= zobrist_table.pieces_and_squares[promoted_piece_index][to];
        }
        // Handle castling
        if(flags == O_O){// kingside castle
            // transfer the rook
            pos.pieces[WHITE_ROOK] ^= WHITE_ROOK_KINGSIDE_CASTLE_MASK;
            pos.white_pieces ^= WHITE_ROOK_KINGSIDE_CASTLE_MASK;
            pos.zobrist_key ^= zobrist_table.pieces_and_squares[WHITE_ROOK][63];
            pos.zobrist_key ^= zobrist_table.pieces_and_squares[WHITE_ROOK][61];
        }
        else if(flags == O_O_O){// queenside castle
            // transfer the rook
            pos.pieces[WHITE_ROOK] ^= WHITE_ROOK_QUEENSIDE_CASTLE_MASK;
            pos.white_pieces ^= WHITE_ROOK_QUEENSIDE_CASTLE_MASK;
            pos.zobrist_key ^= zobrist_table.pieces_and_squares[WHITE_ROOK][56];
            pos.zobrist_key ^= zobrist_table.pieces_and_squares[WHITE_ROOK][59];
        }

        // CASTLING RIGHTS
        // save current castling rights 
        state.can_white_castle_kingside = pos.can_white_castle_kingside;
        state.can_white_castle_queenside = pos.can_white_castle_queenside;
        state.can_black_castle_kingside = pos.can_black_castle_kingside;
        state.can_black_castle_queenside = pos.can_black_castle_queenside;
        // loose castling rights if current move is castling or king move 
        if(flags == O_O || flags == O_O_O || moved_piece_index == WHITE_KING){
            // un-hash of the previous castling rights in Zobrist key
            pos.zobrist_key ^= zobrist_table.castling_rights[CastlingHashing(pos)];
            // update castling rights
            pos.can_white_castle_kingside = false;
            pos.can_white_castle_queenside = false;
            // hash new rights
            pos.zobrist_key ^= zobrist_table.castling_rights[CastlingHashing(pos)];
        }
        // if you move the rook on h1 
        else if(moved_piece_index == WHITE_ROOK && from == 63){
            // un-hash old rights
            pos.zobrist_key ^= zobrist_table.castling_rights[CastlingHashing(pos)];
            // update rights
            pos.can_white_castle_kingside = false;
            // hash new rights
            pos.zobrist_key ^= zobrist_table.castling_rights[CastlingHashing(pos)];
        }
        // if you move the rook on a1
        else if(moved_piece_index == WHITE_ROOK && from == 56){
            // un-hash old rights
            pos.zobrist_key ^= zobrist_table.castling_rights[CastlingHashing(pos)];
            // update rights
            pos.can_white_castle_queenside = false;
            // hash new rights
            pos.zobrist_key ^= zobrist_table.castling_rights[CastlingHashing(pos)];
        }

        // increment half move counter in case of capture or pawn move
        if(captured_piece_index != NO_PIECE || moved_piece_index == WHITE_PAWN){
            state.half_move_counter = pos.half_move_counter;
            pos.half_move_counter = 0;
        }
        else{ pos.half_move_counter++; }
        // update bitboards
        pos.all_pieces = pos.white_pieces | pos.black_pieces;
        // update side to move
        pos.white_to_move = false;
        pos.zobrist_key ^= zobrist_table.white_to_move;

    }

    // BLACK TO MOVE
    else{
        // retrieve what piece has moved
        for(int piece_index = BLACK_KING; piece_index < 12; piece_index++){
            if(pos.pieces[piece_index] & from_bb){
                moved_piece_index = piece_index;
                break;
            }
        }
        // retrieve info about captured piece (if any)
        for(int piece_index = WHITE_QUEEN; piece_index <= WHITE_PAWN; piece_index++){
            // standard capture
            if(pos.pieces[piece_index] & to_bb){
                captured_piece_index = piece_index;
                break;
            } 
            // en-passant capture
            else if(flags == EN_PASSANT){
                captured_piece_index = WHITE_PAWN; // <--- necessarily a pawn
                break;
            }
        }

        // EN - PASSANT TARGET
        // if there's an active e.p. square, un-hash it (any move will delete this target)
        if(pos.en_passant_target_square){ // un-hash
            _BitScanForward64(&ep_square, pos.en_passant_target_square);
            pos.zobrist_key ^= zobrist_table.en_passant_file[ep_square % 8];
        }
        state.en_passant_target_square = pos.en_passant_target_square;
        // if double pawn push, update target
        if(flags == DOUBLE_PAWN_PUSH){
            pos.en_passant_target_square = 1ULL << (to - 8);
            pos.zobrist_key ^= zobrist_table.en_passant_file[to % 8]; // (to - 8) % 8 = to % 8
        }
        else{ // any other move will simply remove any target
            pos.en_passant_target_square = 0ULL;
        }

        // PROMOTIONS
        // retrieve info about promoted piece (if any)
        if(flags == PROMOTION_QUEEN || flags == PROMOTION_QUEEN_CAPTURE)
            promoted_piece_index = BLACK_QUEEN; 
        else if(flags == PROMOTION_ROOK || flags == PROMOTION_ROOK_CAPTURE)
            promoted_piece_index = BLACK_ROOK; 
        else if(flags == PROMOTION_BISHOP || flags == PROMOTION_BISHOP_CAPTURE)
            promoted_piece_index = BLACK_BISHOP;
        else if(flags == PROMOTION_KNIGHT || flags == PROMOTION_KNIGHT_CAPTURE)
            promoted_piece_index = BLACK_KNIGHT;
        // memorize current state
        state.moved_piece_index = moved_piece_index;
        state.captured_piece_index = captured_piece_index;
        state.promoted_piece_index = promoted_piece_index;
        // move the piece
        pos.pieces[moved_piece_index] ^= from_bb | to_bb;
        pos.black_pieces ^= from_bb | to_bb;
        pos.zobrist_key ^= zobrist_table.pieces_and_squares[moved_piece_index][from];
        pos.zobrist_key ^= zobrist_table.pieces_and_squares[moved_piece_index][to];
        // remove captured piece, if any
        if(captured_piece_index != NO_PIECE){
            // if en passant, piece is not in the target square
            if(flags == EN_PASSANT){
                bit_clear(pos.pieces[WHITE_PAWN], to - 8);
                bit_clear(pos.white_pieces, to - 8);
                pos.zobrist_key ^= zobrist_table.pieces_and_squares[WHITE_PAWN][to - 8];
            }
            // if normal capture
            else{
                bit_clear(pos.pieces[captured_piece_index], to);
                bit_clear(pos.white_pieces, to);
                pos.zobrist_key ^= zobrist_table.pieces_and_squares[captured_piece_index][to];
            }
        }
        // spawn the promoted piece in case of promotion and remove the pawn
        if(promoted_piece_index != NO_PIECE){
            bit_clear(pos.pieces[BLACK_PAWN], to);
            bit_set(pos.pieces[promoted_piece_index], to);
            pos.zobrist_key ^= zobrist_table.pieces_and_squares[BLACK_PAWN][to];
            pos.zobrist_key ^= zobrist_table.pieces_and_squares[promoted_piece_index][to];
        }
        // Handle castling
        if(flags == O_O){// kingside castle
            // transfer the rook
            pos.pieces[BLACK_ROOK] ^= BLACK_ROOK_KINGSIDE_CASTLE_MASK;
            pos.black_pieces ^= BLACK_ROOK_KINGSIDE_CASTLE_MASK;
            pos.zobrist_key ^= zobrist_table.pieces_and_squares[BLACK_ROOK][5];
            pos.zobrist_key ^= zobrist_table.pieces_and_squares[BLACK_ROOK][7];
        }
        else if(flags == O_O_O){// queenside castle
            // transfer the rook
            pos.pieces[BLACK_ROOK] ^= BLACK_ROOK_QUEENSIDE_CASTLE_MASK;
            pos.black_pieces ^= BLACK_ROOK_QUEENSIDE_CASTLE_MASK;
            pos.zobrist_key ^= zobrist_table.pieces_and_squares[BLACK_ROOK][0];
            pos.zobrist_key ^= zobrist_table.pieces_and_squares[BLACK_ROOK][3];
        }

        // CASTLING RIGHTS
        // save current castling rights 
        state.can_white_castle_kingside = pos.can_white_castle_kingside;
        state.can_white_castle_queenside = pos.can_white_castle_queenside;
        state.can_black_castle_kingside = pos.can_black_castle_kingside;
        state.can_black_castle_queenside = pos.can_black_castle_queenside;
        // loose castling rights if current move is castling or king move 
        if(flags == O_O || flags == O_O_O || moved_piece_index == BLACK_KING){
            // un-hash old rights
            pos.zobrist_key ^= zobrist_table.castling_rights[CastlingHashing(pos)];
            // update rights
            pos.can_black_castle_kingside = false;
            pos.can_black_castle_queenside = false;
            // hash new rights
            pos.zobrist_key ^= zobrist_table.castling_rights[CastlingHashing(pos)];
        }
        // if you move the rook on h8 
        else if(moved_piece_index == BLACK_ROOK && from == 7){
            // un-hash
            pos.zobrist_key ^= zobrist_table.castling_rights[CastlingHashing(pos)];
            // update
            pos.can_black_castle_kingside = false;
            // hash
            pos.zobrist_key ^= zobrist_table.castling_rights[CastlingHashing(pos)];
        }
        // if you move the rook on a8
        else if(moved_piece_index == BLACK_ROOK && from == 0){
            // un-hash
            pos.zobrist_key ^= zobrist_table.castling_rights[CastlingHashing(pos)];
            // update
            pos.can_black_castle_queenside = false;
            // hash new rights
            pos.zobrist_key ^= zobrist_table.castling_rights[CastlingHashing(pos)];
        }

        // Reset half move counter in case of capture or pawn move
        if(captured_piece_index != NO_PIECE || moved_piece_index == BLACK_PAWN){
            state.half_move_counter = pos.half_move_counter;
            pos.half_move_counter = 0;
        }
        // ...or else increment it 
        else{ pos.half_move_counter++; }
        // update bitboards
        pos.all_pieces = pos.white_pieces | pos.black_pieces;
        // update side to move
        pos.white_to_move = true;
        pos.zobrist_key ^= zobrist_table.white_to_move;
    }

} 


void MakeMoveTest(Position& pos, const Move& move, StateMemory& state){
// RETRIEVE INFO FROM THE MOVE
    int from = static_cast<uint64_t>(move & 0b0000000000111111);
    int to = static_cast<uint64_t>((move >> 6) & 0b0000000000111111);
    int flags = (move >> 12);
    // starting square and ending square bitboards
    Bitboard from_bb = (1ULL << from);
    Bitboard to_bb = (1ULL << to); 

// RETRIEVE MOVED, CAPTURED AND PROMOTED PIECES
// + MAKE THE MOVE
    for(int idx = WHITE_KING; idx < NO_PIECE; idx++){
        // moved piece
        if(pos.pieces[idx] & from_bb){
            // save
            state.moved_piece_index = idx; 
            // update piece bb
            pos.pieces[idx] ^= from_bb | to_bb;
            pos.white_to_move ? pos.white_pieces ^= from_bb | to_bb : pos.black_pieces ^= from_bb | to_bb;
            // unhash old square + hash new square
            pos.zobrist_key ^= zobrist_table.pieces_and_squares[idx][from];
            pos.zobrist_key ^= zobrist_table.pieces_and_squares[idx][to];
            // if this piece has moved, it can't be the captured piece
            continue;
        }
        // captured piece (no e.p.)
        if(pos.pieces[idx] & to_bb){
            // save
            state.captured_piece_index = idx;
            // update piece bb
            bit_clear(pos.pieces[idx], to);
            pos.white_to_move ? bit_clear(pos.black_pieces, to) : bit_clear(pos.white_pieces, to);
            // unhash piece from "to"
            pos.zobrist_key ^= zobrist_table.pieces_and_squares[idx][to];
        }
    }
    // captured piece (e.p.)
    if(flags == EN_PASSANT){
        if(pos.white_to_move){
            // save
            state.captured_piece_index = BLACK_PAWN;
            // clean piece bb (piece is in to + 8)
            bit_clear(pos.pieces[BLACK_PAWN], to + 8);
            bit_clear(pos.black_pieces, to + 8);
            // unhash the pawn
            pos.zobrist_key ^= zobrist_table.pieces_and_squares[BLACK_PAWN][to + 8];
        } else {
            // save
            state.captured_piece_index = WHITE_PAWN;
            // clean piece bb (piece is in to - 8)
            bit_clear(pos.pieces[WHITE_PAWN], to - 8);
            bit_clear(pos.white_pieces, to - 8);
            // unhash the pawn
            pos.zobrist_key ^= zobrist_table.pieces_and_squares[WHITE_PAWN][to - 8];
        }
    }
    // promoted piece is encoded in the move flags
    else if(flags == PROMOTION_QUEEN || flags == PROMOTION_QUEEN_CAPTURE)
        state.promoted_piece_index = pos.white_to_move ? WHITE_QUEEN : BLACK_QUEEN; 
    else if(flags == PROMOTION_ROOK || flags == PROMOTION_ROOK_CAPTURE)
        state.promoted_piece_index = pos.white_to_move ? WHITE_ROOK : BLACK_ROOK; 
    else if(flags == PROMOTION_BISHOP || flags == PROMOTION_BISHOP_CAPTURE)
        state.promoted_piece_index = pos.white_to_move ? WHITE_BISHOP : BLACK_BISHOP;
    else if(flags == PROMOTION_KNIGHT || flags == PROMOTION_KNIGHT_CAPTURE)
        state.promoted_piece_index = pos.white_to_move ? WHITE_KNIGHT : BLACK_KNIGHT;
    // spawn the new piece
    if(state.promoted_piece_index != NO_PIECE){
        bit_set(pos.pieces[state.promoted_piece_index], to);
        // unhash the pawn + hash the new piece
        pos.zobrist_key ^= zobrist_table.pieces_and_squares[state.moved_piece_index][to];
        pos.zobrist_key ^= zobrist_table.pieces_and_squares[state.promoted_piece_index][to];
    }    

// CASTLING 
//  1. IF MOVE IS CASTLING, COMPLETE THE MOVE
    uint8_t old_castling_rights = CastlingHashing(pos);
    uint8_t new_castling_rights = old_castling_rights;
    // kingside castling
    if(flags == O_O){
        if(pos.white_to_move){
            // transfer the rook
            pos.pieces[WHITE_ROOK] ^= WHITE_ROOK_KINGSIDE_CASTLE_MASK;
            pos.white_pieces ^= WHITE_ROOK_KINGSIDE_CASTLE_MASK;
            pos.zobrist_key ^= zobrist_table.pieces_and_squares[WHITE_ROOK][63];
            pos.zobrist_key ^= zobrist_table.pieces_and_squares[WHITE_ROOK][61];
            // white loses castling rights
            new_castling_rights = old_castling_rights & 0b00001100;
        } else{
            // transfer the rook
            pos.pieces[BLACK_ROOK] ^= BLACK_ROOK_KINGSIDE_CASTLE_MASK;
            pos.white_pieces ^= BLACK_ROOK_KINGSIDE_CASTLE_MASK;
            pos.zobrist_key ^= zobrist_table.pieces_and_squares[BLACK_ROOK][7];
            pos.zobrist_key ^= zobrist_table.pieces_and_squares[BLACK_ROOK][5];
            // black loses castling rights
            new_castling_rights = old_castling_rights & 0b00000011;
        }
    }
    // queenside castling
    else if(flags == O_O_O){
        if(pos.white_to_move){
            pos.pieces[WHITE_ROOK] ^= WHITE_ROOK_QUEENSIDE_CASTLE_MASK;
            pos.white_pieces ^= WHITE_ROOK_QUEENSIDE_CASTLE_MASK;
            pos.zobrist_key ^= zobrist_table.pieces_and_squares[WHITE_ROOK][56];
            pos.zobrist_key ^= zobrist_table.pieces_and_squares[WHITE_ROOK][59];
            new_castling_rights = old_castling_rights & 0b00001100;
        } else {
            pos.pieces[BLACK_ROOK] ^= BLACK_ROOK_QUEENSIDE_CASTLE_MASK;
            pos.white_pieces ^= BLACK_ROOK_QUEENSIDE_CASTLE_MASK;
            pos.zobrist_key ^= zobrist_table.pieces_and_squares[BLACK_ROOK][3];
            pos.zobrist_key ^= zobrist_table.pieces_and_squares[BLACK_ROOK][0];
            new_castling_rights = old_castling_rights & 0b00000011;
        }
    }
    // WORK IN PROGRESS ...
    //else if(state.moved_piece_index == WHITE_KING)
    //    new_castling_rights = old_castling_rights & 0b00001100;
    //else if(state.moved_piece_index == BLACK_KING)
    //   new_castling_rights = old_castling_rights & 0b00000011; 
}


void UnmakeMove(Position& pos, const Move& move, const StateMemory& state){
    uint64_t from = static_cast<uint64_t>(move & 0b0000000000111111);
    uint64_t to = static_cast<uint64_t>((move >> 6) & 0b0000000000111111);
    uint16_t flags = (move >> 12);
    unsigned long ep_square;
    // reposition the moved piece
    bit_clear(pos.pieces[state.moved_piece_index], to);
    bit_set(pos.pieces[state.moved_piece_index], from);
    pos.zobrist_key ^= zobrist_table.pieces_and_squares[state.moved_piece_index][from];
    pos.zobrist_key ^= zobrist_table.pieces_and_squares[state.moved_piece_index][to];
    // un-hash e.p. target square if present
    if(pos.en_passant_target_square){ // un-hash
        _BitScanForward64(&ep_square, pos.en_passant_target_square);
        pos.zobrist_key ^= zobrist_table.en_passant_file[ep_square % 8];
    }
    // hash back previously existing e.p. target square if present
    if(state.en_passant_target_square){ // un-hash
        _BitScanForward64(&ep_square, state.en_passant_target_square);
        pos.zobrist_key ^= zobrist_table.en_passant_file[ep_square % 8];
    }
    //pos.zobrist_key = state.zobrist_key;

    // black made the pseudomove
    if(pos.white_to_move){
        pos.black_pieces ^= (1ULL << from) | (1ULL << to);
        // reposition the captured piece
        if(state.captured_piece_index != NO_PIECE){
            // if capture is en passant, respawn the white pawn in the correct position
            if(flags == 5){
                bit_set(pos.pieces[WHITE_PAWN], to - 8);
                bit_set(pos.white_pieces, to - 8);
                pos.zobrist_key ^= zobrist_table.pieces_and_squares[WHITE_PAWN][to - 8];
            }
            // for a normal capture, respawn the piece in the target square of the move
            else{
                bit_set(pos.pieces[state.captured_piece_index], to);
                bit_set(pos.white_pieces, to);
                pos.zobrist_key ^= zobrist_table.pieces_and_squares[state.captured_piece_index][to];
            }
        }

        // remove promoted piece and restore the pawn 
        if(state.promoted_piece_index != 12){
            bit_clear(pos.pieces[state.promoted_piece_index], to);
            bit_set(pos.pieces[BLACK_PAWN], from);
            pos.zobrist_key ^= zobrist_table.pieces_and_squares[state.promoted_piece_index][to];
            pos.zobrist_key ^= zobrist_table.pieces_and_squares[BLACK_PAWN][to];
        }
        // in case of castling, reposition the rook correctly
        if(flags == 2){ // kingside
            pos.pieces[BLACK_ROOK] ^= BLACK_ROOK_KINGSIDE_CASTLE_MASK;
            pos.black_pieces ^= BLACK_ROOK_KINGSIDE_CASTLE_MASK;
            pos.zobrist_key ^= zobrist_table.pieces_and_squares[BLACK_ROOK][5];
            pos.zobrist_key ^= zobrist_table.pieces_and_squares[BLACK_ROOK][7];
        }
        else if(flags == 3){ // queenside
            pos.pieces[BLACK_ROOK] ^= BLACK_ROOK_QUEENSIDE_CASTLE_MASK;
            pos.black_pieces ^= BLACK_ROOK_QUEENSIDE_CASTLE_MASK;
            pos.zobrist_key ^= zobrist_table.pieces_and_squares[BLACK_ROOK][0];
            pos.zobrist_key ^= zobrist_table.pieces_and_squares[BLACK_ROOK][3];
        }
        // restore en-passant target square
        pos.en_passant_target_square = state.en_passant_target_square;
        // restore group bitboards
        pos.all_pieces = pos.white_pieces | pos.black_pieces;
        // restore previous castling rights
        pos.zobrist_key ^= zobrist_table.castling_rights[CastlingHashing(pos)]; // un-hash current rights
        pos.can_white_castle_kingside = state.can_white_castle_kingside; // restore old
        pos.can_white_castle_queenside = state.can_white_castle_queenside;
        pos.can_black_castle_kingside = state.can_black_castle_kingside; // restore old rights
        pos.can_black_castle_queenside = state.can_black_castle_queenside;
        pos.zobrist_key ^= zobrist_table.castling_rights[CastlingHashing(pos)]; // hash again
        // restore half-move counter
        if(state.captured_piece_index != 12 || state.moved_piece_index == BLACK_PAWN){
            pos.half_move_counter = state.half_move_counter;
        }
        else{ pos.half_move_counter--; }
        // update side to move
        pos.white_to_move = false;
        pos.zobrist_key ^= zobrist_table.white_to_move;
    }

    // if white made the pseudomove 
    else{
        pos.white_pieces ^= (1ULL << from) | (1ULL << to);
        // reposition the captured piece
        if(state.captured_piece_index != NO_PIECE){
            // if the capture was en-passant, the captured pawn does not respawn in the target square of the move
            if(flags == 5){
                bit_set(pos.pieces[BLACK_PAWN], to + 8);
                bit_set(pos.black_pieces, to + 8);
                pos.zobrist_key ^= zobrist_table.pieces_and_squares[BLACK_PAWN][to + 8];
            }
            else{
                bit_set(pos.pieces[state.captured_piece_index], to);
                bit_set(pos.black_pieces, to);
                pos.zobrist_key ^= zobrist_table.pieces_and_squares[state.captured_piece_index][to];
            }
        }

        // remove promoted piece and restore the pawn 
        if(state.promoted_piece_index != 12){
            bit_clear(pos.pieces[state.promoted_piece_index], to);
            bit_set(pos.pieces[WHITE_PAWN], from);
            pos.zobrist_key ^= zobrist_table.pieces_and_squares[state.promoted_piece_index][to];
            pos.zobrist_key ^= zobrist_table.pieces_and_squares[WHITE_PAWN][to];
        }
        // in case of castling, reposition the rook correctly
        if(flags == 2){ // kingside
            pos.pieces[WHITE_ROOK] ^= WHITE_ROOK_KINGSIDE_CASTLE_MASK;
            pos.white_pieces ^= WHITE_ROOK_KINGSIDE_CASTLE_MASK;
            pos.zobrist_key ^= zobrist_table.pieces_and_squares[WHITE_ROOK][61];
            pos.zobrist_key ^= zobrist_table.pieces_and_squares[WHITE_ROOK][63];
        }
        else if(flags == 3){// queenside
            pos.pieces[WHITE_ROOK] ^= WHITE_ROOK_QUEENSIDE_CASTLE_MASK;
            pos.white_pieces ^= WHITE_ROOK_QUEENSIDE_CASTLE_MASK;
            pos.zobrist_key ^= zobrist_table.pieces_and_squares[WHITE_ROOK][56];
            pos.zobrist_key ^= zobrist_table.pieces_and_squares[WHITE_ROOK][59];
        }
        // restore en-passant target
        pos.en_passant_target_square = state.en_passant_target_square;
        // restore group bitboards
        pos.all_pieces = pos.white_pieces | pos.black_pieces;
        // restore previous castling rights
        pos.zobrist_key ^= zobrist_table.castling_rights[CastlingHashing(pos)]; // un-hash
        pos.can_white_castle_kingside = state.can_white_castle_kingside; // restore old
        pos.can_white_castle_queenside = state.can_white_castle_queenside;
        pos.can_black_castle_kingside = state.can_black_castle_kingside; // restore old
        pos.can_black_castle_queenside = state.can_black_castle_queenside;
        pos.zobrist_key ^= zobrist_table.castling_rights[CastlingHashing(pos)]; // hash
        // restore half-move counter
        if(state.captured_piece_index != 12 || state.moved_piece_index == WHITE_PAWN){
            pos.half_move_counter = state.half_move_counter;
        }
        else{ pos.half_move_counter--; }
        // update side to move
        pos.white_to_move = true;
        pos.zobrist_key ^= zobrist_table.white_to_move;
    }
}

bool SquareIsAttacked(Position& pos, const unsigned long int square){
    Bitboard attacks = 0ULL; 
    MaskAndMagic mm;
    // if white to move: white is the attacker
    if(pos.white_to_move){
        // 1. check attacks from opponent's king 
        attacks = king_covered_squares_bitboards[square];
        if(attacks & pos.pieces[0]) return true; 
        // 2: check attacks from  knight
        attacks = knight_covered_squares_bitboards[square];
        if(attacks & pos.pieces[4]) return true; 
        // 3: check attacks from  pawns
        attacks = black_pawn_covered_squares_bitboards[square];
        if(attacks & pos.pieces[5]) return true; 
        // 4: check attacks from diagonal directions
        mm = bishop_mm[square];
        attacks = bishop_covered_squares_bb[square][((pos.all_pieces & mm.mask) * mm.magic) >> SHIFT_BISHOP];
        if(attacks & (pos.pieces[1] | pos.pieces[3])) return true; 
        // 5: check attacks from horizontal or vertical directions
        mm = rook_mm[square];
        attacks = rook_covered_squares_bb[square][((pos.all_pieces & mm.mask) * mm.magic) >> SHIFT_ROOK];
        if(attacks & (pos.pieces[1] | pos.pieces[2])) return true; 
    }
    // else black is the attacker
    else{
        // 1. check attacks from opponent's king 
        attacks = king_covered_squares_bitboards[square];
        if(attacks & pos.pieces[6]) return true; 
        // 2: check attacks from  knight
        attacks = knight_covered_squares_bitboards[square];
        if(attacks & pos.pieces[10]) return true; 
        // 3: check attacks from  pawns
        attacks = white_pawn_covered_squares_bitboards[square];
        if(attacks & pos.pieces[11]) return true; 
        // 4: check attacks from diagonal directions
        mm = bishop_mm[square];
        attacks = bishop_covered_squares_bb[square][((pos.all_pieces & mm.mask) * mm.magic) >> SHIFT_BISHOP];
        if(attacks & (pos.pieces[7] | pos.pieces[9])) return true; 
        // 5: check attacks from horizontal or vertical directions
        mm = rook_mm[square];
        attacks = rook_covered_squares_bb[square][((pos.all_pieces & mm.mask) * mm.magic) >> SHIFT_ROOK];
        if(attacks & (pos.pieces[7] | pos.pieces[8])) return true; 
    }
    // if survived till here...
    return false;
}

bool IsLegal(Position& pos, const Move& move){ 
    Bitboard attacks = 0ULL;
    MaskAndMagic mm;
    unsigned long king_square;
    uint8_t flags = (move >> 12); 
    // if white to move and black's king is in check, pos is illegal
    if(pos.white_to_move){
        // step 1: get black's king position: if no king -> illegal
        if(pos.pieces[BLACK_KING])
            _BitScanForward64(&king_square, pos.pieces[BLACK_KING]);
        else return false;
        // step 2: check attacks from white king
        attacks = king_covered_squares_bitboards[king_square];
        if(attacks & pos.pieces[WHITE_KING]){ return false; }
        // step 3: check attacks from white knight
        attacks = knight_covered_squares_bitboards[king_square];
        if(attacks & pos.pieces[WHITE_KNIGHT]){ return false; }
        // step 4: check attacks from white pawns
        attacks = black_pawn_covered_squares_bitboards[king_square];
        if(attacks & pos.pieces[WHITE_PAWN]){ return false; }
        // step 5: check attacks from diagonal directions
        mm = bishop_mm[king_square];
        attacks = bishop_covered_squares_bb[king_square][((pos.all_pieces & mm.mask) * mm.magic) >> SHIFT_BISHOP];
        if(attacks & (pos.pieces[WHITE_QUEEN] | pos.pieces[WHITE_BISHOP])){ return false; }
        // step 6: check attacks from horizontal or vertical directions
        mm = rook_mm[king_square];
        attacks = rook_covered_squares_bb[king_square][((pos.all_pieces & mm.mask) * mm.magic) >> SHIFT_ROOK];
        if(attacks & (pos.pieces[WHITE_QUEEN] | pos.pieces[WHITE_ROOK])){ return false; }
        // if black just castled (so now is white to move), control that the black king was not passing through a square covered by white
        if(flags == 2){
            if(63624ULL & pos.pieces[0]){ return false; }
            if(16309248ULL & pos.pieces[4]){ return false; }
            if(63488ULL & pos.pieces[5]){ return false; }
            mm = bishop_mm[6];
            attacks = bishop_covered_squares_bb[6][((pos.all_pieces & mm.mask) * mm.magic) >> SHIFT_BISHOP];
            if(attacks & (pos.pieces[1] | pos.pieces[3])){ return false; }
            mm = rook_mm[6];
            attacks = rook_covered_squares_bb[6][((pos.all_pieces & mm.mask) * mm.magic) >> SHIFT_ROOK];
            if(attacks & (pos.pieces[1] | pos.pieces[2])){ return false; }

            mm = bishop_mm[5];
            attacks = bishop_covered_squares_bb[5][((pos.all_pieces & mm.mask) * mm.magic) >> SHIFT_BISHOP];
            if(attacks & (pos.pieces[1] | pos.pieces[3])){ return false; }
            mm = rook_mm[5];
            attacks = rook_covered_squares_bb[5][((pos.all_pieces & mm.mask) * mm.magic) >> SHIFT_ROOK];
            if(attacks & (pos.pieces[1] | pos.pieces[2])){ return false; }

            mm = bishop_mm[4];
            attacks = bishop_covered_squares_bb[4][((pos.all_pieces & mm.mask) * mm.magic) >> SHIFT_BISHOP];
            if(attacks & (pos.pieces[1] | pos.pieces[3])){ return false; }
            mm = rook_mm[4];
            attacks = rook_covered_squares_bb[4][((pos.all_pieces & mm.mask) * mm.magic) >> SHIFT_ROOK];
            if(attacks & (pos.pieces[1] | pos.pieces[2])){ return false; }
        }
        else if(flags == 3){
            if(15906ULL & pos.pieces[0]){ return false; }
            if(4093696ULL & pos.pieces[4]){ return false; }
            if(15872ULL & pos.pieces[5]){ return false; }
            mm = bishop_mm[2];
            attacks = bishop_covered_squares_bb[2][((pos.all_pieces & mm.mask) * mm.magic) >> SHIFT_BISHOP];
            if(attacks & (pos.pieces[1] | pos.pieces[3])){ return false; }
            mm = rook_mm[2];
            attacks = rook_covered_squares_bb[2][((pos.all_pieces & mm.mask) * mm.magic) >> SHIFT_ROOK];
            if(attacks & (pos.pieces[1] | pos.pieces[2])){ return false; }

            mm = bishop_mm[3];
            attacks = bishop_covered_squares_bb[3][((pos.all_pieces & mm.mask) * mm.magic) >> SHIFT_BISHOP];
            if(attacks & (pos.pieces[1] | pos.pieces[3])){ return false; }
            mm = rook_mm[3];
            attacks = rook_covered_squares_bb[3][((pos.all_pieces & mm.mask) * mm.magic) >> SHIFT_ROOK];
            if(attacks & (pos.pieces[1] | pos.pieces[2])){ return false; }

            mm = bishop_mm[4];
            attacks = bishop_covered_squares_bb[4][((pos.all_pieces & mm.mask) * mm.magic) >> SHIFT_BISHOP];
            if(attacks & (pos.pieces[1] | pos.pieces[3])){ return false; }
            mm = rook_mm[4];
            attacks = rook_covered_squares_bb[4][((pos.all_pieces & mm.mask) * mm.magic) >> SHIFT_ROOK];
            if(attacks & (pos.pieces[1] | pos.pieces[2])){ return false; }
        }
        // if all the previous legality checks are passed, return true
        return true;
    }
    // if black to move and white's king is in check, pos is illegal
    else{
        // step 1: get black's king position: if no king -> illegal
        if(pos.pieces[WHITE_KING])
            _BitScanForward64(&king_square, pos.pieces[WHITE_KING]);
        else return false;
        // step 2: check attacks from white king
        attacks = king_covered_squares_bitboards[king_square];
        if(attacks & pos.pieces[BLACK_KING]) return false; 
        // step 3: check attacks from white knight
        attacks = knight_covered_squares_bitboards[king_square];
        if(attacks & pos.pieces[BLACK_KNIGHT]){ return false; }
        // step 4: check attacks from white pawns
        attacks = white_pawn_covered_squares_bitboards[king_square];
        if(attacks & pos.pieces[BLACK_PAWN]){ return false; }
        // step 5: check attacks from diagonal directions
        mm = bishop_mm[king_square];
        attacks = bishop_covered_squares_bb[king_square][((pos.all_pieces & mm.mask) * mm.magic) >> SHIFT_BISHOP];
        if(attacks & (pos.pieces[BLACK_QUEEN] | pos.pieces[BLACK_BISHOP])){ return false; }
        // step 6: check attacks from horizontal or vertical directions
        mm = rook_mm[king_square];
        attacks = rook_covered_squares_bb[king_square][((pos.all_pieces & mm.mask) * mm.magic) >> SHIFT_ROOK];
        if(attacks & (pos.pieces[BLACK_QUEEN] | pos.pieces[BLACK_ROOK])){ return false; }
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



// NULL MOVE GENERATION
void MakeNullMove(Position& pos, StateMemory& state){
    // change side to move
    pos.white_to_move = !pos.white_to_move;
    pos.zobrist_key ^= zobrist_table.white_to_move;
    // increment half-moves ...? 
    pos.half_move_counter++;
    // if en passant was available, it is no longer available, but we need to restore it later!
    state.en_passant_target_square = pos.en_passant_target_square;
    pos.en_passant_target_square = 0ULL;
    unsigned long ep_square;
    _BitScanForward64(&ep_square, state.en_passant_target_square);
    pos.zobrist_key ^= zobrist_table.en_passant_file[ep_square % 8];
    // castling rights are not touched
    //
}

void UnmakeNullMove(Position& pos, StateMemory& state){
    // restore the en-passant target
    pos.en_passant_target_square = state.en_passant_target_square;
    unsigned long ep_square;
    _BitScanForward64(&ep_square, state.en_passant_target_square);
    pos.zobrist_key ^= zobrist_table.en_passant_file[ep_square % 8];
    // 
    pos.half_move_counter--;
    pos.white_to_move = !pos.white_to_move;
    pos.zobrist_key ^= zobrist_table.white_to_move;
}

bool InCheck(const Position& pos){
    uint64_t attacks;
    MaskAndMagic mm;
    // calculate king's square for the side to move
    unsigned long int king_square;
    _BitScanForward64(&king_square, pos.pieces[pos.white_to_move ? WHITE_KING : BLACK_KING]);
    // check all possible attackers
    //      1: check attacks from  knight
    attacks = knight_covered_squares_bitboards[king_square];
    if(attacks & pos.pieces[pos.white_to_move ? BLACK_KNIGHT : WHITE_KNIGHT])
        return true;
    //      2: check attacks from pawns
    attacks = pos.white_to_move ? white_pawn_covered_squares_bitboards[king_square] : black_pawn_covered_squares_bitboards[king_square];
    if(attacks & pos.pieces[pos.white_to_move ? BLACK_PAWN : WHITE_PAWN])
        return true;
    //      3: check attacks from diagonal directions
    mm = bishop_mm[king_square];
    attacks = bishop_covered_squares_bb[king_square][((pos.all_pieces & mm.mask) * mm.magic) >> SHIFT_BISHOP];
    if(attacks & (pos.pieces[pos.white_to_move ? BLACK_QUEEN : WHITE_QUEEN] | pos.pieces[pos.white_to_move ? BLACK_BISHOP : WHITE_BISHOP]))
        return true;
    // 5: check attacks from horizontal or vertical directions
    mm = rook_mm[king_square];
    attacks = rook_covered_squares_bb[king_square][((pos.all_pieces & mm.mask) * mm.magic) >> SHIFT_ROOK];
    if(attacks & (pos.pieces[pos.white_to_move ? BLACK_QUEEN : WHITE_QUEEN] | pos.pieces[pos.white_to_move ? BLACK_ROOK : WHITE_ROOK]))
        return true;
    // if survived till here...
    return false;
}

bool OnlyPawnsRemaining(const Position& pos){
    if(pos.pieces[WHITE_QUEEN]) return false;
    if(pos.pieces[WHITE_ROOK]) return false;
    if(pos.pieces[WHITE_BISHOP]) return false;
    if(pos.pieces[WHITE_KNIGHT]) return false;
    if(pos.pieces[BLACK_QUEEN]) return false;
    if(pos.pieces[BLACK_ROOK]) return false;
    if(pos.pieces[BLACK_BISHOP]) return false;
    if(pos.pieces[BLACK_KNIGHT]) return false;
    return true;
}

/*
bool OkToMakeNullMove(const Position& pos, bool previous_null){
    return !(InCheck(pos) || OnlyPawnsRemaining(pos) || previous_null); 
}*/


void AggressiveMoves(Position& pos, Move* moves){
    int move_index = 0;
    uint64_t piece = 0ULL;
    unsigned long square, target_square;
    uint64_t attacks = 0ULL;
    uint16_t flags;
    MaskAndMagic mm;

    // WHITE MOVES
    if(pos.white_to_move){

        // Queen
        piece = pos.pieces[WHITE_QUEEN]; // retrieve bitboard of queens
        while(piece){ // consider all the queens
            _BitScanForward64(&square, piece); // find position of queen and assign it to square
            // retrieve bitboard of queen moves
            mm = rook_mm[square];
            attacks = rook_covered_squares_bb[square][((pos.all_pieces & mm.mask) * mm.magic) >> SHIFT_ROOK];
            mm = bishop_mm[square];
            attacks |= bishop_covered_squares_bb[square][((pos.all_pieces & mm.mask) * mm.magic) >> SHIFT_BISHOP];
            attacks &= pos.black_pieces; // only captures!
            while(attacks){
                _BitScanForward64(&target_square, attacks); // find the target square
                moves[move_index] = EncodeMove(square, target_square, 4);
                move_index++;
                clear_last_active_bit(attacks); 
            }
            clear_last_active_bit(piece);
        }

        // Rook
        piece = pos.pieces[WHITE_ROOK];
        while(piece){
            _BitScanForward64(&square, piece); // find position of queen and assign it to square
            // retrieve bitboard of queen moves
            mm = rook_mm[square];
            attacks = rook_covered_squares_bb[square][((pos.all_pieces & mm.mask) * mm.magic) >> SHIFT_ROOK];
            attacks &= pos.black_pieces; 
            while(attacks){
                _BitScanForward64(&target_square, attacks); // find the target square
                moves[move_index] = EncodeMove(square, target_square, 4);
                move_index++;
                clear_last_active_bit(attacks); 
            }
            clear_last_active_bit(piece);
        }

        // Bishop
        piece = pos.pieces[WHITE_BISHOP]; // retrieve bitboard of queens
        while(piece){ // consider all the queens
            _BitScanForward64(&square, piece); // find position of queen and assign it to square
            mm = bishop_mm[square];
            attacks = bishop_covered_squares_bb[square][((pos.all_pieces & mm.mask) * mm.magic) >> SHIFT_BISHOP];
            attacks &= pos.black_pieces; 
            while(attacks){
                _BitScanForward64(&target_square, attacks); // find the target square
                moves[move_index] = EncodeMove(square, target_square, 4);
                move_index++;
                clear_last_active_bit(attacks); 
            }
            clear_last_active_bit(piece);
        }

        // Knight
        piece = pos.pieces[WHITE_KNIGHT]; // retrieve bitboard of knights
        while(piece){ // consider all the knights
            _BitScanForward64(&square, piece); // find position of piece and assign it to square
            attacks = knight_covered_squares_bitboards[square]; // retrieve attack bitboard
            attacks &= pos.black_pieces; // exclude self-capture
            while(attacks){
                _BitScanForward64(&target_square, attacks); // find the target square
                moves[move_index] = EncodeMove(square, target_square, 4);
                move_index++;
                clear_last_active_bit(attacks); 
            }
            // remove considered piece
            clear_last_active_bit(piece);
        }

        // Pawn capture
        piece = pos.pieces[WHITE_PAWN];
        while(piece){
            _BitScanForward64(&square, piece); // find position of piece and assign it to square
            // NORMAL CAPTURES
            attacks = white_pawn_covered_squares_bitboards[square]; // retrieve attack bitboard
            attacks &= pos.black_pieces; // only attacked squares occupied by enemy pieces are valid for movement
            while(attacks){
                _BitScanForward64(&target_square, attacks); // find the target square
                // in case of promotion, loop over all possible promoted pieces
                if(WHITE_PAWN_IN_FINAL_RANK[target_square]){
                    for(uint8_t promoted_piece_index = WHITE_QUEEN; promoted_piece_index < WHITE_PAWN; promoted_piece_index++){
                        flags = 16 - promoted_piece_index;
                        moves[move_index] = EncodeMove(square, target_square, flags);
                        move_index++;
                    }
                }
                else{ 
                    moves[move_index] = EncodeMove(square, target_square, 4);
                    move_index++;
                }
                clear_last_active_bit(attacks); // remove considered attack
            }
            // EN PASSANT CAPTURES
            attacks = white_pawn_covered_squares_bitboards[square]; // retrieve attack bitboard
            attacks &= pos.en_passant_target_square; // only attacked squares occupied by enemy pieces are valid for movement
            while(attacks){
                _BitScanForward64(&target_square, attacks); // find the target square            
                //flags = 5;
                moves[move_index] = EncodeMove(square, target_square, 5);
                move_index++;
                clear_last_active_bit(attacks); // remove considered attack
            }
            
            // remove considered piece
            clear_last_active_bit(piece);
        }

    }

    // BLACK MOVES
    else{

        // Queen
        piece = pos.pieces[BLACK_QUEEN]; // retrieve bitboard of queens
        while(piece){ // consider all the queens
            _BitScanForward64(&square, piece); // find position of queen and assign it to square
            // retrieve bitboard of queen moves
            mm = rook_mm[square];
            attacks = rook_covered_squares_bb[square][((pos.all_pieces & mm.mask) * mm.magic) >> SHIFT_ROOK];
            mm = bishop_mm[square];
            attacks |= bishop_covered_squares_bb[square][((pos.all_pieces & mm.mask) * mm.magic) >> SHIFT_BISHOP];
            attacks &= pos.white_pieces; // excluse self-capture
            while(attacks){
                _BitScanForward64(&target_square, attacks); // find the target square
                moves[move_index] = EncodeMove(square, target_square, 4);
                move_index++;
                clear_last_active_bit(attacks); 
            }
            clear_last_active_bit(piece);
        }

        // Rook
        piece = pos.pieces[BLACK_ROOK];
        while(piece){
            _BitScanForward64(&square, piece); // find position of queen and assign it to square
            // retrieve bitboard of queen moves
            mm = rook_mm[square];
            attacks = rook_covered_squares_bb[square][((pos.all_pieces & mm.mask) * mm.magic) >> SHIFT_ROOK];
            attacks &= pos.white_pieces; // excluse self-capture
            while(attacks){
                _BitScanForward64(&target_square, attacks); // find the target square
                moves[move_index] = EncodeMove(square, target_square, 4);
                move_index++;
                clear_last_active_bit(attacks); 
            }
            clear_last_active_bit(piece);
        }

        // Bishop
        piece = pos.pieces[BLACK_BISHOP]; // retrieve bitboard of queens
        while(piece){ // consider all the queens
            _BitScanForward64(&square, piece); // find position of queen and assign it to square
            mm = bishop_mm[square];
            attacks = bishop_covered_squares_bb[square][((pos.all_pieces & mm.mask) * mm.magic) >> SHIFT_BISHOP];
            attacks &= pos.white_pieces; // excluse self-capture
            while(attacks){
                _BitScanForward64(&target_square, attacks); // find the target square
                moves[move_index] = EncodeMove(square, target_square, 4);
                move_index++;
                clear_last_active_bit(attacks); 
            }
            clear_last_active_bit(piece);
        }

        // Knight
        piece = pos.pieces[BLACK_KNIGHT]; // retrieve bitboard of knights
        while(piece){ // consider all the knights
            _BitScanForward64(&square, piece); // find position of piece and assign it to square
            attacks = knight_covered_squares_bitboards[square]; // retrieve attack bitboard
            attacks &= pos.white_pieces; // exclude self-capture
            while(attacks){
                _BitScanForward64(&target_square, attacks); // find the target square
                moves[move_index] = EncodeMove(square, target_square, 4);
                move_index++;
                clear_last_active_bit(attacks); 
            }
            // remove considered piece
            clear_last_active_bit(piece);
        }

        // Pawn capture
        piece = pos.pieces[BLACK_PAWN];
        while(piece){
            _BitScanForward64(&square, piece); // find position of piece and assign it to square
            // NORMAL CAPTURES
            attacks = black_pawn_covered_squares_bitboards[square]; // retrieve attack bitboard
            attacks &= pos.white_pieces; // only attacked squares occupied by enemy pieces are valid for movement
            while(attacks){
                _BitScanForward64(&target_square, attacks); // find the target square
                // in case of promotion, loop over all possible promoted pieces
                if(BLACK_PAWN_IN_FINAL_RANK[target_square]){
                    for(uint8_t promoted_piece_index = BLACK_QUEEN; promoted_piece_index < BLACK_PAWN; promoted_piece_index++){
                        moves[move_index] = EncodeMove(square, target_square, 22 - promoted_piece_index);
                        move_index++;
                    }
                }
                else{ 
                    moves[move_index] = EncodeMove(square, target_square, 4);
                    move_index++;
                }
                clear_last_active_bit(attacks); // remove considered attack
            }
            // EN PASSANT CAPTURES
            attacks = black_pawn_covered_squares_bitboards[square]; // retrieve attack bitboard
            attacks &= pos.en_passant_target_square; // only attacked squares occupied by enemy pieces are valid for movement
            while(attacks){
                _BitScanForward64(&target_square, attacks); // find the target square
                moves[move_index] = EncodeMove(square, target_square, 5);
                move_index++;
                clear_last_active_bit(attacks); // remove considered attack
            }

            // remove considered piece
            clear_last_active_bit(piece);
        }

    }
    //pos.n_pseudolegal_moves = move_index;
}