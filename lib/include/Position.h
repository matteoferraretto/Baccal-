#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <Move.h>

// Class Position is made as follows:
// - 12 bitboards storing the position of the 12 types of pieces: K Q R B N P k q r b n p
// - 3 bitboards with mask of white pieces, black pieces and all pieces (meant to avoid repeated calculations in move generation)
// - 1 bitboard with mask of en passant target square
// - 1 uint8_t with relevant info:
//      bits      _            _                   _                  _                _                   _               _                _
//      info  [nothing] [black in check?] [white in check?] [can black O-O-O?] [can black O-O?] [can white O-O-O?] [can white O-O?] [white to move?]
// - 1 uint8_t half_move_counter to account for the 50 moves rule 
//      from 0 to 50 and we have 2 extra bits
// - 1 uint8_t n_legal_moves representing the number of legal moves
//      from 0 to 2^8 - 1 = 255 (max allowed size is 256)
// TOTAL MEMORY REQUIRED 
// 64 x 12 + 64 x 3 + 64 x 1 + 8 x 1 + 8 x 1 + 8 x 1 = 1048 bits = 131 bytes
struct Position
{
    uint64_t pieces[12] = {
        0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL
    }; // K Q R B N P k q r b n p
    bool white_to_move;
    bool can_white_castle_kingside = false;
    bool can_white_castle_queenside = false;
    bool can_black_castle_kingside = false;
    bool can_black_castle_queenside = false;
    uint8_t half_move_counter = 0;
    uint64_t en_passant_target_square = 0ULL;
    uint64_t white_pieces = 0ULL;
    uint64_t black_pieces = 0ULL;
    uint64_t all_pieces = 0ULL;
    uint64_t white_covered_squares = 0ULL;
    uint64_t black_covered_squares = 0ULL;
    uint8_t n_pseudolegal_moves = 0;
};

Position PositionFromFen(std::string fen);

void PrintBoard(Position pos);

int PositionScore(Position& pos);


// Generate all the PSEUDOLEGAL moves, which means:
//  - move a piece from a square to another square following the rules
//  - if the square is occupied by a friendly piece, don't consider the move
//  - ignore if the move leaves the king in danger (whence PSEUDOlegal)
void PseudoLegalMoves(Position& pos, Move* moves);

void MakeMove(Position& pos, const Move& move, StateMemory& state);

bool SquareIsAttacked(Position& pos, const unsigned long int square);

bool IsLegal(Position& pos, const Move& move);

void UnmakeMove(Position& pos, const Move& move, const StateMemory& state);

// Consider all the moves, filter out illegal moves that leave the king in check and generate the new position
// This function is optimized for the engine purposes:
// if we check the legality of a move first and then use it to update the position, we are forced to generate 
// the new bitboards of covered squares twice and check the king's position twice! With this approach we do it only ONCE
// We can later write another function for user purposes that only filters out illegal moves
//std::vector<Position> MakeLegalMoves(const Position& pos, const std::vector<Move>& move);


// SORTING MOVES HEURISTICS
// ---------------------------------------------------- bonus
// 1. Promotion to queen with capture                   10 706
// 2. Promotion to queen                                10 705
// 3. Captures with MVV - LVA heuristics (see below)    9904 -> [4, 10 704]
// 4. Underpromotion with capture                       3
// 5. Underpromotion                                    2
// 6. Castling (good for king safety)                   1
// 7. Quiet moves                                       0
// -------------------------------------------------------
// Sorting captures with
//    MVV - LVA: Most Valuable Victim - Least Valuable Attacker
//      score  =  bonus for capture  +  |value of victim|  -  |value of attacker|
//    e.g. 
//      pawn takes queen:    score = bonus for capture + 800
//      pawn takes rook:     score = bonus for capture + 400
//      pawn takes pawn:     score = bonus for capture
//      rook takes pawn:     score = bonus for capture - 400
//      queen takes pawn:    score = bonus for capture - 800
//      king takes pawn:     score = bonus for capture - 9900 
//
// it is not important that the score is uniform or representative of how a move is good relative to anothre, it is just a sorting tool!
//
// 
const int BONUS_KING_SAFETY = 1;
const int BONUS_UNDERPROMO = 2;
const int BONUS_UNDERPROMO_WITH_CAPTURE = 3;
const int BONUS_CAPTURE = 9904;
const int BONUS_QUEEN_PROMO = 10705;
const int BONUS_QUEEN_PROMO_WITH_CAPTURE = 10706;

inline int ScoreMove(const Position& pos, const Move& move){
    int score = 0;
    /*uint8_t piece = MovePiece(move);
    uint8_t captured_piece = MoveCaptured(move);
    uint8_t promoted_piece = MovePromotion(move);
    uint8_t flags = MoveFlags(move);
    bool is_check = MoveIsCheck(move);
    // capture 
    if(captured_piece != 15){
        score += BONUS_FOR_CAPTURE + abs(PIECES_VALUES[captured_piece]) - abs(PIECES_VALUES[piece]);
    }
    // promotion
    if(promoted_piece != 15){
        score += BONUS_FOR_PROMOTION + abs(PIECES_VALUES[promoted_piece]);
    }
    // check
    if(is_check){
        score += BONUS_FOR_CHECKS + abs(PIECES_VALUES[piece]);
    }*/

    uint8_t flags;
    flags = move >> 12;

    // quiet move -> no bonus
    if(flags == 0 || flags == 1){
        return 0;
    }
    else if(flags == 15){
        score += BONUS_QUEEN_PROMO_WITH_CAPTURE;
    }
    else if(flags == 11){
        score += BONUS_QUEEN_PROMO;
    }
    else if(flags == 4){
        uint8_t from = move & 0b00111111;
        uint8_t to = (move >> 6) & 0b00111111;
        score += BONUS_CAPTURE;
        if(pos.white_to_move){
            // check what piece is on the target square:
            for(uint8_t piece_index = 6; piece_index < 12; piece_index++){
                if(pos.pieces[piece_index] & (1ULL << to)){
                    score -= PIECES_VALUES[piece_index]; // subtract a negative number --> add that number
                    break;
                }
            }
            // check what piece is on the starting square:
            for(uint8_t piece_index = 0; piece_index < 6; piece_index++){
                if(pos.pieces[piece_index] & (1ULL << from)){
                    score -= PIECES_VALUES[piece_index]; // subtract a positive number
                    break;
                }
            }
        }
        else{
            // check what piece is on the target square:
            for(uint8_t piece_index = 1; piece_index < 6; piece_index++){
                if(pos.pieces[piece_index] & (1ULL << to)){
                    score += PIECES_VALUES[piece_index]; 
                    break;
                }
            }
            // check what piece is on the starting square:
            for(uint8_t piece_index = 6; piece_index < 12; piece_index++){
                if(pos.pieces[piece_index] & (1ULL << from)){
                    score += PIECES_VALUES[piece_index]; // add a negative number --> subtract the abs
                    break;
                }
            }
        }
    }
    else if(flags == 1){
        score += BONUS_CAPTURE;
    }
    else if(flags < 15 && flags > 11){
        score += BONUS_UNDERPROMO_WITH_CAPTURE; 
    }
    else if(flags < 11 && flags > 7){
        score += BONUS_UNDERPROMO;
    }
    else if(flags == 2 || flags == 3){
        score += BONUS_KING_SAFETY;
    }
    
    return score;
}
