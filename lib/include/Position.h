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
    bool white_to_move = false;
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
    uint64_t zobrist_key = 0ULL;
    uint8_t n_pseudolegal_moves = 0;

    bool operator==(const Position& other) const {
        if (white_to_move != other.white_to_move) return false;
        if (can_white_castle_kingside != other.can_white_castle_kingside) return false;
        if (can_white_castle_queenside != other.can_white_castle_queenside) return false;
        if (can_black_castle_kingside != other.can_black_castle_kingside) return false;
        if (can_black_castle_queenside != other.can_black_castle_queenside) return false;
        if (en_passant_target_square != other.en_passant_target_square) return false;
        if (half_move_counter != other.half_move_counter) return false;
        if (zobrist_key != other.zobrist_key) return false;

        if (white_pieces != other.white_pieces) return false;
        if (black_pieces != other.black_pieces) return false;
        if (all_pieces   != other.all_pieces) return false;

        for (int i = 0; i < 12; i++) {
            if (pieces[i] != other.pieces[i]) return false;
        }
        return true;
    }

    bool operator!=(const Position& other) const {
        return !(*this == other);
    }
};

Position PositionFromFen(std::string fen);

// A playable position is a position where the rules of chess don't break
// and from which it is possible to play on using the rules. 
// This is different from a LEGAL position, which is a position that should be compatible with the starting position
// for example, a position with 20 white queens is playable, but illegal, because we can apply the rules and play the game from here,
// but it can't stem from the starting position (max 9 white queens if all 8 pawns are promoted)
bool PositionIsPlayable(Position pos);

void PrintBoard(Position pos, bool white_perspective);

int PositionScore(Position& pos);


// Generate all the PSEUDOLEGAL moves, which means:
//  - move a piece from a square to another square following the rules
//  - if the square is occupied by a friendly piece, don't consider the move
//  - ignore if the move leaves the king in danger (whence PSEUDOlegal)
void PseudoLegalMoves(Position& pos, Move* moves);

// Fast generation of aggressive moves only, useful for quiescence search
void AggressiveMoves(Position& pos, Move* moves);

void MakeMove(Position& pos, const Move& move, StateMemory& state);

bool SquareIsAttacked(Position& pos, const unsigned long int square);

bool IsLegal(Position& pos, const Move& move);

void UnmakeMove(Position& pos, const Move& move, const StateMemory& state);

// SORTING MOVES HEURISTICS
// ---------------------------------------------------- bonus
// 1. Promotion to queen with capture                   10 707
// 2. Promotion to queen                                10 706
// 3. Captures with MVV - LVA heuristics (see below)    9905 -> [5, 10 705]
// 4. Underpromotion with capture                       4
// 5. Underpromotion                                    3
// 6. Killer moves                                      2
// 7. Castling (good for king safety)                   1
// 8. Quiet moves                                       0
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
// -------------------------------------------------------
// KILLER MOVES
// These are QUIET moves that generate a beta-cutoff 
// the idea is to try them before other quiet moves at a given search depth 
// the killer moves might lead to other cutoffs in other positions!
// 

const int BONUS_KING_SAFETY = 1;
const int BONUS_KILLER_MOVE = 2;
const int BONUS_UNDERPROMO = 3;
const int BONUS_UNDERPROMO_WITH_CAPTURE = 4;
const int BONUS_CAPTURE = 9905;
const int BONUS_QUEEN_PROMO = 10706;
const int BONUS_QUEEN_PROMO_WITH_CAPTURE = 10707;
const int BONUS_TT_BEST_MOVE = 11000;

inline int ScoreMove(const Position& pos, const Move& move){
    int score = 0;

    uint64_t from = static_cast<uint64_t>(move & 0b0000000000111111);
    uint64_t to = static_cast<uint64_t>((move >> 6) & 0b0000000000111111);
    uint16_t flags = (move >> 12);

    // quiet move -> no bonus
    if(flags == QUIET_MOVE || flags == DOUBLE_PAWN_PUSH || flags == NOTHING_1 || flags == NOTHING_2)
        return 0;
    else if(flags == PROMOTION_QUEEN_CAPTURE)
        score += BONUS_QUEEN_PROMO_WITH_CAPTURE;
    else if(flags == PROMOTION_QUEEN)
        score += BONUS_QUEEN_PROMO;
    // capture: MVV - LVA
    else if(flags == CAPTURE){
        score += BONUS_CAPTURE;
        if(pos.white_to_move){
            // check what piece is on the target square:
            for(int piece_index = 6; piece_index < 12; piece_index++){
                if(pos.pieces[piece_index] & (1ULL << to)){
                    score -= PIECES_VALUES[piece_index]; // subtract a negative number --> add that number
                    break;
                }
            }
            // check what piece is on the starting square:
            for(int piece_index = 0; piece_index < 6; piece_index++){
                if(pos.pieces[piece_index] & (1ULL << from)){
                    score -= PIECES_VALUES[piece_index]; // subtract a positive number
                    break;
                }
            }
        }
        else{
            // check what piece is on the target square:
            for(int piece_index = 1; piece_index < 6; piece_index++){
                if(pos.pieces[piece_index] & (1ULL << to)){
                    score += PIECES_VALUES[piece_index]; 
                    break;
                }
            }
            // check what piece is on the starting square:
            for(int piece_index = 6; piece_index < 12; piece_index++){
                if(pos.pieces[piece_index] & (1ULL << from)){
                    score += PIECES_VALUES[piece_index]; // add a negative number --> subtract the abs
                    break;
                }
            }
        }
    }
    // en passant capture -> victim is pawn, attacker is pawn -> ranked as a pawn-pawn capture
    else if(flags == EN_PASSANT)
        score += BONUS_CAPTURE;
    else if(flags < PROMOTION_QUEEN_CAPTURE && flags > PROMOTION_QUEEN)
        score += BONUS_UNDERPROMO_WITH_CAPTURE; 
    else if(flags < PROMOTION_QUEEN && flags > NOTHING_2)
        score += BONUS_UNDERPROMO;
    else if(flags == O_O || flags == O_O_O)
        score += BONUS_KING_SAFETY;
    
    return score;
}

// Null moves
void MakeNullMove(Position& pos, StateMemory& state);
void UnmakeNullMove(Position& pos, StateMemory& state);
// is the side to move in check?
bool InCheck(const Position& pos);
// are there only pawns remaining in the position? -> used to detect probability of zwugzwang 
bool OnlyPawnsRemaining(const Position& pos);
// verify draw for insufficient check-mating material
bool InsufficientMaterial(const Position& pos);


// Print moves
inline std::string AlgebraicNotation(Position pos, const Move move){
    uint8_t from = static_cast<uint8_t>(move & 0b0000000000111111);
    uint8_t to = static_cast<uint8_t>((move >> 6) & 0b0000000000111111);
    Bitboard from_bb = (1ULL << from);
    Bitboard to_bb = (1ULL << to);
    uint8_t to_rank = to / 8, to_file = to % 8;
    uint8_t from_rank = from / 8, from_file = from % 8;
    int flags = (move >> 12);
    std::string move_str = std::string();

    // castling
    if(flags == O_O){
        return "O-O";
    }
    else if(flags == O_O_O){
        return "O-O-O";
    }

    MaskAndMagic mm;
    uint64_t hash;
    Bitboard piece, attacks, horizontal_attackers, vertical_attackers;
    unsigned long square;
    Move other_move = move;
    StateMemory state;

    // retrieve moved piece
    int moved_piece_index = NO_PIECE;
    for(int idx = 0; idx < 12; idx++){
        if(pos.pieces[idx] & from_bb){
            moved_piece_index = idx;
            break;
        }
    }
    if(moved_piece_index != WHITE_PAWN && moved_piece_index != BLACK_PAWN)
        move_str += "KQRBNPKQRBNP"[moved_piece_index];

    // disambiguation of the starting square
    if(moved_piece_index == WHITE_ROOK || moved_piece_index == BLACK_ROOK){
        // find other rooks (exclude the moved rook)
        piece = pos.pieces[pos.white_to_move ? WHITE_ROOK : BLACK_ROOK] ^ from_bb;
        // loop over these pieces to find if one of them can go to the "to" square
        while(piece){
            _BitScanForward64(&square, piece);
            mm = rook_mm[square];
            hash = ((pos.all_pieces & mm.mask) * mm.magic) >> SHIFT_ROOK;
            attacks = rook_covered_squares_bb[square][hash];
            attacks &= to_bb; 
            // is the "to" squares attacked by this other rook? 
            // If yes, generate the pseudomove and check legality.
            if(attacks){
                other_move = move & 0b1111111111000000; // delete "from" bits
                other_move |= static_cast<uint16_t>(square) & 0b0000000000111111; // substitute the bits with the new "from"
                // check move legality
                MakeMove(pos, other_move, state);
                if(IsLegal(pos, other_move)){
                    UnmakeMove(pos, other_move, state);
                    // ambiguity type: do from and square have different files? If so, specify file.
                    if(static_cast<uint8_t>(square % 8) != from_file)
                        move_str += "abcdefgh"[from_file];
                    // if same file, remove ambiguity with rank
                    else 
                        move_str += "87654321"[from_rank];
                }
                UnmakeMove(pos, other_move, state);
            }
            clear_last_active_bit(piece);
        }
        // pretend there's a rook on the "to" square
        // compute mask of horizontal attacks (do not filter out occupied squares yet)
        // count the bits of (horizontal_attacks & your_pieces) -> if > 1: ambiguity
        // if the "from" square is within the resulting mask, print file
        /*mm = rook_mm[to];
        hash = ((pos.all_pieces & mm.mask) * mm.magic) >> SHIFT_ROOK;
        attacks = rook_covered_squares_bb[to][hash];
        attacks &= pos.pieces[pos.white_to_move ? WHITE_ROOK : BLACK_ROOK];
        // if ambiguity
        if(pop_count(attacks) > 1){
            // disambiguate by starting rank only if 2 rooks are vertically aligned
            vertical_attackers = attacks & files_bitboards[to_file];
            if(pop_count(vertical_attackers) > 1) move_str += "87654321"[from_rank];
            // disambiguate by starting file in any other case
            else move_str += "abcdefgh"[from_file];
        } */
        /*horizontal_attackers = attacks & ranks_bitboards[to_rank] & pos.pieces[pos.white_to_move ? WHITE_ROOK : BLACK_ROOK];
        if(horizontal_attackers & from_bb){
            if(pop_count(horizontal_attackers) > 1)
                move_str += "abcdefgh"[from_file];
        }
        // similar logic for vertical attacks
        vertical_attackers = attacks & files_bitboards[to_file] & pos.pieces[pos.white_to_move ? WHITE_ROOK : BLACK_ROOK];
        if(vertical_attackers & from_bb){
            if(pop_count(vertical_attackers) > 1)
                move_str += "87654321"[from_rank];
        }*/
    }
    else if(moved_piece_index == WHITE_BISHOP || moved_piece_index == BLACK_BISHOP){
        // start from the "to" square and generate bishop attacks
        // AND with the bb of same color bishops 
        // if in the resulting bb we have 2 bishops on the same rank, specify the file
        // if in the resulting bb we have 2 bishops on the same file, specify the rank
        mm = bishop_mm[to];
        hash = ((pos.all_pieces & mm.mask) * mm.magic) >> SHIFT_BISHOP;
        attacks = bishop_covered_squares_bb[to][hash];
        horizontal_attackers = attacks & ranks_bitboards[from_rank] & pos.pieces[pos.white_to_move ? WHITE_BISHOP : BLACK_BISHOP];
        if(horizontal_attackers & from_bb){
            if(pop_count(horizontal_attackers) > 1)
                move_str += "abcdefgh"[from_file];
        }
        vertical_attackers = attacks & files_bitboards[from_file] & pos.pieces[pos.white_to_move ? WHITE_BISHOP : BLACK_BISHOP];
        if(vertical_attackers & from_bb){
            if(pop_count(vertical_attackers) > 1)
                move_str += "87654321"[from_rank];
        }
    }
    else if(moved_piece_index == WHITE_QUEEN || moved_piece_index == BLACK_QUEEN){
        // start from the "to" square and generate bishop attacks
        // AND with the bb of same color bishops 
        // if in the resulting bb we have 2 bishops on the same rank, specify the file
        // if in the resulting bb we have 2 bishops on the same file, specify the rank
        mm = rook_mm[to];
        hash = ((pos.all_pieces & mm.mask) * mm.magic) >> SHIFT_ROOK;
        attacks = rook_covered_squares_bb[to][hash];
        mm = bishop_mm[to];
        hash = ((pos.all_pieces & mm.mask) * mm.magic) >> SHIFT_BISHOP;
        attacks |= bishop_covered_squares_bb[to][hash];
        horizontal_attackers = attacks & ranks_bitboards[from_rank] & pos.pieces[pos.white_to_move ? WHITE_QUEEN : BLACK_QUEEN];
        if(horizontal_attackers & from_bb){
            if(pop_count(horizontal_attackers) > 1)
                move_str += "abcdefgh"[from_file];
        }
        vertical_attackers = attacks & files_bitboards[from_file] & pos.pieces[pos.white_to_move ? WHITE_QUEEN : BLACK_QUEEN];
        if(vertical_attackers & from_bb){
            if(pop_count(vertical_attackers) > 1)
                move_str += "87654321"[from_rank];
        }
    }
    else if(moved_piece_index == WHITE_KNIGHT || moved_piece_index == BLACK_KNIGHT){
        attacks = knight_covered_squares_bitboards[to];
        horizontal_attackers = attacks & ranks_bitboards[from_rank] & pos.pieces[pos.white_to_move ? WHITE_KNIGHT : BLACK_KNIGHT];
        if(horizontal_attackers & from_bb){
            if(pop_count(horizontal_attackers) > 1)
                move_str += "abcdefgh"[from_file];
        }
        vertical_attackers = attacks & files_bitboards[from_file] & pos.pieces[pos.white_to_move ? WHITE_KNIGHT : BLACK_KNIGHT];
        if(vertical_attackers & from_bb){
            if(pop_count(vertical_attackers) > 1)
                move_str += "87654321"[from_rank];
        }
    }
    else if(moved_piece_index == WHITE_PAWN || moved_piece_index == BLACK_PAWN){
        if(flags == CAPTURE || flags == EN_PASSANT || flags == PROMOTION_QUEEN_CAPTURE || flags == PROMOTION_ROOK_CAPTURE || flags == PROMOTION_BISHOP_CAPTURE || flags == PROMOTION_KNIGHT_CAPTURE){
            move_str += "abcdefgh"[from_file];
        }
    }

    // detect if the move is a capture
    for(int idx = 0; idx < 12; idx++){
        if(pos.pieces[idx] & to_bb || flags == EN_PASSANT){
            move_str += "x";
            break;
        }
    }

    move_str += SquareToAlphabet(to);
    // manage promotions
    if(flags == PROMOTION_QUEEN_CAPTURE || flags == PROMOTION_QUEEN)
        move_str += "=Q";
    else if(flags == PROMOTION_ROOK_CAPTURE || flags == PROMOTION_ROOK)
        move_str += "=R";
    else if(flags == PROMOTION_BISHOP_CAPTURE || flags == PROMOTION_BISHOP)
        move_str += "=B";
    else if(flags == PROMOTION_KNIGHT_CAPTURE || flags == PROMOTION_KNIGHT)
        move_str += "=N";

    // verify if move is check
    MakeMove(pos, move, state);
    if(InCheck(pos)) move_str += "+";
    UnmakeMove(pos, move, state);

    // print final result
    return move_str;
}


inline void PrintLegalMoves(Position pos){
    std::cout << "Legal moves: \n";
    Move move = 0;
    Move moves[MAX_NUMBER_OF_MOVES] = { };
    PseudoLegalMoves(pos, moves);
    for(int idx = 0; idx < MAX_NUMBER_OF_MOVES; idx++){
        move = moves[idx];
        if(move == 0) break;
        StateMemory state;
        MakeMove(pos, move, state);
        if(!IsLegal(pos, move)){
            UnmakeMove(pos, move, state);
            continue;
        }
        UnmakeMove(pos, move, state);
        std::cout << AlgebraicNotation(pos, move) << ", ";
    }
    std::cout << "\n";
}