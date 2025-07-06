#include "Position.h"
#include "Baccala.h"
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <typeinfo>
#include <algorithm>


Move::Move(Square current_square, Square target_square, char piece, bool is_capture){
    this->current_square = current_square; 
    this->target_square = target_square; 
    this->piece = piece;
    this->is_capture = is_capture;
    this->new_piece = 'z'; // conventionally means that this move was not a pawn promotion.
}

Move::Move(Square target_square, char new_piece){
    this->target_square = target_square;
    this->new_piece = new_piece;
    this->is_capture = false;
    this->current_square.j = target_square.j;
    if(target_square.i == 0){ this->piece = 'P'; this->current_square.i = target_square.i+1; }
    else if(target_square.i == 7){ this->piece = 'p'; this->current_square.i = target_square.i-1; }
}

Move::Move(char castling_side){
    this->castling_side = castling_side;
    this->is_castling = true;
}

void Move::PrintMove(){
    if(this->is_castling){
        if(this->castling_side == 'K' || this->castling_side == 'k'){ std::cout << "O-O" << "\n"; }
        else if(this->castling_side == 'Q' || this->castling_side == 'q'){ std::cout << "O-O-O" << "\n"; }
        return;
    }
    std::cout << this->piece << " (" << this->current_square.i << ", " << this->current_square.j << ") -> (" << this->target_square.i << ", " << this->target_square.j << ") ";
    // if pawn promotion
    if(this->new_piece != 'z'){
        std::cout << this->new_piece << ". ";
    }
    if(this->is_capture){ std::cout << "This is a capture."; }
    std::cout << "\n";
}

std::string Move::AlgebraicNotation(){
    std::string algebraic_notation;
    // castling
    if(this->is_castling){
        if(this->castling_side == 'K' || this->castling_side == 'k'){ algebraic_notation = "O-O"; }
        else if(this->castling_side == 'Q' || this->castling_side == 'q'){ algebraic_notation = "O-O-O"; }
    }
    // normal move
    else{
        // pawn moves
        if(this->piece=='P' || this->piece=='p'){
            algebraic_notation = std::string(); 
            if(this->is_capture){ algebraic_notation += SquareToAlphabet(this->current_square)[0]; algebraic_notation += 'x'; }
            algebraic_notation += SquareToAlphabet(this->target_square);
            if(this->new_piece != 'z'){ algebraic_notation += '='; algebraic_notation += this->new_piece; }
        }
        else{
            algebraic_notation = std::string() + this->piece + SquareToAlphabet(this->current_square);
            if(this->is_capture){ algebraic_notation += 'x'; }
            algebraic_notation += SquareToAlphabet(this->target_square);
        }
    }
    return algebraic_notation;
}

Move::~Move(){};


void SortMoves(std::vector<Move>& moves){
    struct greater_than{
        inline bool operator() (const Move& move1, const Move& move2)
        {
            return (move1.rank > move2.rank);
        }
    };
    std::sort(moves.begin(), moves.end(), greater_than());
}


// --------------------------------------------
// ------------ USEFUL FUNCTIONS --------------
// --------------------------------------------
void PrintMask(Mask mask){
    for(int i=0; i<8; i++){
        for(int j=0; j<8; j++){
            std::cout << mask[i][j] << " ";
        }
        std::cout << "\n";
    }
};

void PrintMoves(std::vector<Square> moves){
    for(Square move: moves){
        std::cout << "(" << move.i << ", " << move.j << ");\t";
    }
};

int PieceValue(char piece){
    int value;
    switch (piece)
    {
    case 'R': value = +5; break;
    case 'r': value = -5; break;
    case 'B': value = +3; break;
    case 'b': value = -3; break;
    case 'N': value = +3; break;
    case 'n': value = -3; break;
    case 'Q': value = +9; break;
    case 'q': value = -9; break;
    case 'K': value = +1000; break;
    case 'k': value = -1000; break;
    case 'P': value = +1; break;
    case 'p': value = -1; break;
    case ' ': value = 0; break;
    }
    return value;
};

// convert a Square object to a string like a1, b2, etc.
std::string SquareToAlphabet(Square square){
    char rank = (char)(56 - square.i);
    char file = static_cast<char>('a' + square.j);
    return (std::string() + file + rank);
}

// 
Square AlphabetToSquare(std::basic_string<char> square_string){
    char file = square_string.at(0);
    char rank = square_string.at(1);
    Square square = {7 - ((int)(rank) - 49), (int)file - 97};
    return square; 
}

// ---------------------------------------------
// --------    MANAGE KNIGTHS   ----------------
// ---------------------------------------------
Mask KnightMovesMask(Square current_square, Mask& your_pieces_mask) {
    Mask mask = empty_mask;
    Square target_square;
    for(int d = 0; d < 8; d++){
        target_square.i = current_square.i + knight_deltas[d][0];
        target_square.j = current_square.j + knight_deltas[d][1];
        // check if target square is inside the board and not occupied by your own pieces
        if(target_square.i < 0 || target_square.i > 7 || target_square.j < 0 || target_square.j > 7 || your_pieces_mask[target_square.i][target_square.j]) { continue; }
        mask[target_square.i][target_square.j] = 1;
    }
    return mask;
}

Mask KnightCoveredSquaresMask(Square current_square) {
    Mask mask = empty_mask;
    Square target_square;
    for(int d = 0; d < 8; d++){
        target_square.i = current_square.i + knight_deltas[d][0];
        target_square.j = current_square.j + knight_deltas[d][1];
        // check if target square is inside the board and not occupied by your own pieces
        if(target_square.i < 0 || target_square.i > 7 || target_square.j < 0 || target_square.j > 7) { continue; }
        mask[target_square.i][target_square.j] = 1;
    }
    return mask;
}

std::vector<Square> KnightTargetSquares(Square current_square, Mask& your_pieces_mask){
    std::vector<Square> target_squares;
    target_squares.reserve(8);
    Square target_square;
    for(int d = 0; d < 8; d++){
        target_square.i = current_square.i + knight_deltas[d][0];
        target_square.j = current_square.j + knight_deltas[d][1];
        // check if target square is inside the board and not occupied by your own pieces
        if(target_square.i < 0 || target_square.i > 7 || target_square.j < 0 || target_square.j > 7 || your_pieces_mask[target_square.i][target_square.j]) { continue; }
        target_squares.push_back(target_square);
    }
    return target_squares;
}


// ---------------------------------------------
// ----------    MANAGE ROOKS   ----------------
// ---------------------------------------------
Mask RookMovesMask(Square current_square, Mask& your_pieces_mask, Mask& opponent_pieces_mask) {
    Mask mask = empty_mask;
    // bottom sliding
    for(int x = current_square.i+1; x < 8; x++){
        // if you encounter your own piece in the sliding, stop
        if(your_pieces_mask[x][current_square.j]) { break; }
        // if you encounter an opponent's piece in the sliding, consider the move and stop
        if(opponent_pieces_mask[x][current_square.j]) { mask[x][current_square.j] = 1; break; }
        // else just consider the move
        mask[x][current_square.j] = 1;
    }
    // top sliding
    for(int x = current_square.i-1; x >= 0; x--){
        if(your_pieces_mask[x][current_square.j]) { break; }
        if(opponent_pieces_mask[x][current_square.j] ) { mask[x][current_square.j] = 1; break; }
        mask[x][current_square.j] = 1;
    }
    // right sliding
    for(int y = current_square.j+1; y < 8; y++){
        if(your_pieces_mask[current_square.i][y]) { break; }
        if(opponent_pieces_mask[current_square.i][y] ) { mask[current_square.i][y] = 1; break; }
        mask[current_square.i][y] = 1;
    }
    // left sliding
    for(int y = current_square.j-1; y >= 0; y--){
        if(your_pieces_mask[current_square.i][y]) { break; }
        if(opponent_pieces_mask[current_square.i][y] ) { mask[current_square.i][y] = 1; break; }
        mask[current_square.i][y] = 1;
    }
    return mask;
}

Mask RookCoveredSquaresMask(Square current_square, Mask& all_pieces_mask) {
    Mask mask = empty_mask;
    // bottom sliding
    for(int x = current_square.i+1; x < 8; x++){
        if(all_pieces_mask[x][current_square.j]) { mask[x][current_square.j] = 1; break; }
        mask[x][current_square.j] = 1;
    }
    // top sliding
    for(int x = current_square.i-1; x >= 0; x--){
        if(all_pieces_mask[x][current_square.j] ) { mask[x][current_square.j] = 1; break; }
        mask[x][current_square.j] = 1;
    }
    // right sliding
    for(int y = current_square.j+1; y < 8; y++){
        if(all_pieces_mask[current_square.i][y] ) { mask[current_square.i][y] = 1; break; }
        mask[current_square.i][y] = 1;
    }
    // left sliding
    for(int y = current_square.j-1; y >= 0; y--){
        if(all_pieces_mask[current_square.i][y] ) { mask[current_square.i][y] = 1; break; }
        mask[current_square.i][y] = 1;
    }
    return mask;
}

std::vector<Square> RookTargetSquares(Square current_square, Mask& your_pieces_mask, Mask& opponent_pieces_mask){
    std::vector<Square> target_squares;
    Square target_square = current_square;
    // bottom sliding
    for(int x = current_square.i+1; x < 8; x++){
        target_square.i = x;
        // if you encounter your own piece in the sliding, stop
        if(your_pieces_mask[x][current_square.j]) { break; }
        // if you encounter an opponent's piece in the sliding, consider the move and stop
        if(opponent_pieces_mask[x][current_square.j]) { target_squares.push_back(target_square); break; }
        // else just consider the move
        target_squares.push_back(target_square);
    }
    // top sliding
    for(int x = current_square.i-1; x >= 0; x--){
        target_square.i = x;
        if(your_pieces_mask[x][current_square.j]) { break; }
        if(opponent_pieces_mask[x][current_square.j] ) { target_squares.push_back(target_square); break; }
        target_squares.push_back(target_square);
    }
    target_square.i = current_square.i;
    // right sliding
    for(int y = current_square.j+1; y < 8; y++){
        target_square.j = y;
        if(your_pieces_mask[current_square.i][y]) { break; }
        if(opponent_pieces_mask[current_square.i][y] ) { target_squares.push_back(target_square); break; }
        target_squares.push_back(target_square);
    }
    // left sliding
    for(int y = current_square.j-1; y >= 0; y--){
        target_square.j = y;
        if(your_pieces_mask[current_square.i][y]) { break; }
        if(opponent_pieces_mask[current_square.i][y] ) { target_squares.push_back(target_square); break; }
        target_squares.push_back(target_square);
    }
    return target_squares;
}

// ---------------------------------------------
// ----------    MANAGE BiSHOPS   --------------
// ---------------------------------------------
Mask BishopMovesMask(Square current_square, Mask& your_pieces_mask, Mask& opponent_pieces_mask) {
    Mask mask = empty_mask;
    Square target_square = current_square;
    // slide to bottom right
    for(int step=1; step<8; step++){
        target_square.i++;
        target_square.j++;
        if(target_square.i > 7 || target_square.j > 7 || your_pieces_mask[target_square.i][target_square.j]){ break; }
        if(opponent_pieces_mask[target_square.i][target_square.j]){ mask[target_square.i][target_square.j] = 1; break; }
        mask[target_square.i][target_square.j] = 1;
    }
    // slide to top left
    target_square = current_square;
    for(int step=1; step<8; step++){
        target_square.i--;
        target_square.j--;
        if(target_square.i < 0 || target_square.j < 0 || your_pieces_mask[target_square.i][target_square.j]){ break; }
        if(opponent_pieces_mask[target_square.i][target_square.j]){ mask[target_square.i][target_square.j] = 1; break; }
        mask[target_square.i][target_square.j] = 1;
    }
    // slide to bottom left
    target_square = current_square;
    for(int step=1; step<8; step++){
        target_square.i++;
        target_square.j--;
        if(target_square.i > 7 || target_square.j < 0 || your_pieces_mask[target_square.i][target_square.j]){ break; }
        if(opponent_pieces_mask[target_square.i][target_square.j]){ mask[target_square.i][target_square.j] = 1; break; }
        mask[target_square.i][target_square.j] = 1;
    }
    // slide to top right
    target_square = current_square;
    for(int step=1; step<8; step++){
        target_square.i--;
        target_square.j++;
        if(target_square.i < 0 || target_square.j > 7 || your_pieces_mask[target_square.i][target_square.j]){ break; }
        if(opponent_pieces_mask[target_square.i][target_square.j]){ mask[target_square.i][target_square.j] = 1; break; }
        mask[target_square.i][target_square.j] = 1;
    }
    return mask;
}

Mask BishopCoveredSquaresMask(Square current_square, Mask& all_pieces_mask) {
    Mask mask = empty_mask;
    Square target_square = current_square;
    // slide to bottom right
    for(int step=1; step<8; step++){
        target_square.i++;
        target_square.j++;
        if(target_square.i > 7 || target_square.j > 7){ break; }
        if(all_pieces_mask[target_square.i][target_square.j]){ mask[target_square.i][target_square.j] = 1; break; }
        mask[target_square.i][target_square.j] = 1;
    }
    // slide to top left
    target_square = current_square;
    for(int step=1; step<8; step++){
        target_square.i--;
        target_square.j--;
        if(target_square.i < 0 || target_square.j < 0){ break; }
        if(all_pieces_mask[target_square.i][target_square.j]){ mask[target_square.i][target_square.j] = 1; break; }
        mask[target_square.i][target_square.j] = 1;
    }
    // slide to bottom left
    target_square = current_square;
    for(int step=1; step<8; step++){
        target_square.i++;
        target_square.j--;
        if(target_square.i > 7 || target_square.j < 0){ break; }
        if(all_pieces_mask[target_square.i][target_square.j]){ mask[target_square.i][target_square.j] = 1; break; }
        mask[target_square.i][target_square.j] = 1;
    }
    // slide to top right
    target_square = current_square;
    for(int step=1; step<8; step++){
        target_square.i--;
        target_square.j++;
        if(target_square.i < 0 || target_square.j > 7){ break; }
        if(all_pieces_mask[target_square.i][target_square.j]){ mask[target_square.i][target_square.j] = 1; break; }
        mask[target_square.i][target_square.j] = 1;
    }
    return mask;
}

std::vector<Square> BishopTargetSquares(Square current_square, Mask& your_pieces_mask, Mask& opponent_pieces_mask){
    std::vector<Square> target_squares;
    Square target_square = current_square;
    // slide to bottom right
    for(int step=1; step<8; step++){
        target_square.i++;
        target_square.j++;
        if(target_square.i > 7 || target_square.j > 7 || your_pieces_mask[target_square.i][target_square.j]){ break; }
        if(opponent_pieces_mask[target_square.i][target_square.j]){ target_squares.push_back(target_square); break; }
        target_squares.push_back(target_square);
    }
    // slide to top left
    target_square = current_square;
    for(int step=1; step<8; step++){
        target_square.i--;
        target_square.j--;
        if(target_square.i < 0 || target_square.j < 0 || your_pieces_mask[target_square.i][target_square.j]){ break; }
        if(opponent_pieces_mask[target_square.i][target_square.j]){ target_squares.push_back(target_square); break; }
        target_squares.push_back(target_square);
    }
    // slide to bottom left
    target_square = current_square;
    for(int step=1; step<8; step++){
        target_square.i++;
        target_square.j--;
        if(target_square.i > 7 || target_square.j < 0 || your_pieces_mask[target_square.i][target_square.j]){ break; }
        if(opponent_pieces_mask[target_square.i][target_square.j]){ target_squares.push_back(target_square); break; }
        target_squares.push_back(target_square);
    }
    // slide to top right
    target_square = current_square;
    for(int step=1; step<8; step++){
        target_square.i--;
        target_square.j++;
        if(target_square.i < 0 || target_square.j > 7 || your_pieces_mask[target_square.i][target_square.j]){ break; }
        if(opponent_pieces_mask[target_square.i][target_square.j]){ target_squares.push_back(target_square); break; }
        target_squares.push_back(target_square);
    }
    return target_squares;
}

// ---------------------------------------------
// -----------    MANAGE QUEENS   --------------
// ---------------------------------------------
Mask QueenMovesMask(Square current_square, Mask& your_pieces_mask, Mask& opponent_pieces_mask) {
    return (RookMovesMask(current_square, your_pieces_mask, opponent_pieces_mask) + BishopMovesMask(current_square, your_pieces_mask, opponent_pieces_mask));
}

Mask QueenCoveredSquaresMask(Square current_square, Mask& all_pieces_mask) {
    return (RookCoveredSquaresMask(current_square, all_pieces_mask) + BishopCoveredSquaresMask(current_square, all_pieces_mask));
}

std::vector<Square> QueenTargetSquares(Square current_square, Mask& your_pieces_mask, Mask& opponent_pieces_mask){
    std::vector<Square> moves = RookTargetSquares(current_square, your_pieces_mask, opponent_pieces_mask);
    std::vector<Square> bishop_moves = BishopTargetSquares(current_square, your_pieces_mask, opponent_pieces_mask);
    moves.insert(moves.end(), bishop_moves.begin(), bishop_moves.end());
    return moves;
}

// ---------------------------------------------
// -----------    MANAGE KINGS   ---------------
// ---------------------------------------------
Mask KingMovesMask(Square current_square, Mask& your_pieces_mask) {
    Mask mask = empty_mask;
    Square target_square;
    for(int d = 0; d < 8; d++){
        target_square.i = current_square.i + king_deltas[d][0];
        target_square.j = current_square.j + king_deltas[d][1];
        // check if target square is inside the board and not occupied by your own pieces
        if(target_square.i < 0 || target_square.i > 7 || target_square.j < 0 || target_square.j > 7 || your_pieces_mask[target_square.i][target_square.j]) { continue; }
        mask[target_square.i][target_square.j] = 1;
    }
    return mask;
}

Mask KingCoveredSquaresMask(Square current_square) {
    Mask mask = empty_mask;
    Square target_square;
    for(int d = 0; d < 8; d++){
        target_square.i = current_square.i + king_deltas[d][0];
        target_square.j = current_square.j + king_deltas[d][1];
        // check if target square is inside the board and not occupied by your own pieces
        if(target_square.i < 0 || target_square.i > 7 || target_square.j < 0 || target_square.j > 7) { continue; }
        mask[target_square.i][target_square.j] = 1;
    }
    return mask;
}

std::vector<Square> KingTargetSquares(Square current_square, Mask& your_pieces_mask){
    std::vector<Square> target_squares;
    target_squares.reserve(8);
    Square target_square;
    for(int d = 0; d < 8; d++){
        target_square.i = current_square.i + king_deltas[d][0];
        target_square.j = current_square.j + king_deltas[d][1];
        // check if target square is inside the board and not occupied by your own pieces
        if(target_square.i < 0 || target_square.i > 7 || target_square.j < 0 || target_square.j > 7 || your_pieces_mask[target_square.i][target_square.j]) { continue; }
        target_squares.push_back(target_square);
    }
    return target_squares;
}

// ---------------------------------------------
// -----------    MANAGE PAWNS   ---------------
// ---------------------------------------------
Mask WhitePawnMovesMask(Square current_square, Mask& your_pieces_mask, Mask& opponent_pieces_mask, Square en_passant_target_square){
    Mask mask = empty_mask;
    if(current_square.i != 0){
        // normal captures
        if(current_square.j > 0){
            if(opponent_pieces_mask[current_square.i-1][current_square.j-1]){
                mask[current_square.i-1][current_square.j-1] = 1;
            }
        }
        if(current_square.j < 7){
            if(opponent_pieces_mask[current_square.i-1][current_square.j+1]){
                mask[current_square.i-1][current_square.j+1] = 1;
            }
        }
        // en passant captures: if pawn is in 5th rank and faces diagonally an en passant target square  
        if(current_square.i == 3 && en_passant_target_square.i == 2 && abs(current_square.j - en_passant_target_square.j) == 1){
            mask[en_passant_target_square.i][en_passant_target_square.j] = 1;
        }
        // normal advance
        if(!your_pieces_mask[current_square.i-1][current_square.j] && !opponent_pieces_mask[current_square.i-1][current_square.j]){
            mask[current_square.i-1][current_square.j] = 1;
            if(current_square.i==6 && !your_pieces_mask[current_square.i-2][current_square.j] && !opponent_pieces_mask[current_square.i-2][current_square.j]){
                mask[current_square.i-2][current_square.j] = 1;
            }
        }
    }
    return mask;
}

Mask WhitePawnCoveredSquaresMask(Square current_square){
    Mask mask = empty_mask;
    if(current_square.i != 0){
        // normal captures
        if(current_square.j > 0){
            mask[current_square.i-1][current_square.j-1] = 1;
        }
        if(current_square.j < 7){
            mask[current_square.i-1][current_square.j+1] = 1;
        }
    }
    return mask;
}

std::vector<Square> WhitePawnTargetSquares(Square current_square, Mask& your_pieces_mask, Mask& opponent_pieces_mask, Square en_passant_target_square){
    std::vector<Square> target_squares;
    Square target_square;
    if(current_square.i != 0){
        // normal captures
        if(current_square.j > 0){
            if(opponent_pieces_mask[current_square.i-1][current_square.j-1]){
                target_square.i = current_square.i-1; target_square.j = current_square.j-1;
                target_squares.push_back(target_square);
            }
        }
        if(current_square.j < 7){
            if(opponent_pieces_mask[current_square.i-1][current_square.j+1]){
                target_square.i = current_square.i-1; target_square.j = current_square.j+1;
                target_squares.push_back(target_square);
            }
        }
        // en passant captures: if pawn is in 5th rank and faces diagonally an en passant target square  
        if(current_square.i == 3 && en_passant_target_square.i == 2 && abs(current_square.j - en_passant_target_square.j) == 1){
            target_squares.push_back(en_passant_target_square);
        }
        // normal advance
        if(!your_pieces_mask[current_square.i-1][current_square.j] && !opponent_pieces_mask[current_square.i-1][current_square.j]){
            target_square.i = current_square.i-1; target_square.j = current_square.j;
            target_squares.push_back(target_square);
            if(current_square.i==6 && !your_pieces_mask[current_square.i-2][current_square.j] && !opponent_pieces_mask[current_square.i-2][current_square.j]){
                target_square.i = current_square.i-2; target_square.j = current_square.j;
                target_squares.push_back(target_square);
            }
        }
    }
    return target_squares;
}

Mask BlackPawnMovesMask(Square current_square, Mask& your_pieces_mask, Mask& opponent_pieces_mask){
    Mask mask = empty_mask;
    if(current_square.i != 7){
        // normal captures
        if(current_square.j > 0){
            if(opponent_pieces_mask[current_square.i+1][current_square.j-1]){
                mask[current_square.i+1][current_square.j-1] = 1;
            }
        }
        if(current_square.j < 7){
            if(opponent_pieces_mask[current_square.i+1][current_square.j+1]){
                mask[current_square.i+1][current_square.j+1] = 1;
            }
        }
        // normal advance
        if(!your_pieces_mask[current_square.i+1][current_square.j] && !opponent_pieces_mask[current_square.i+1][current_square.j]){
            mask[current_square.i+1][current_square.j] = 1;
            if(current_square.i==1 && !your_pieces_mask[current_square.i+2][current_square.j] && !opponent_pieces_mask[current_square.i+2][current_square.j]){
                mask[current_square.i+2][current_square.j] = 1;
            }
        }
    }
    return mask;
}

Mask BlackPawnCoveredSquaresMask(Square current_square){
    Mask mask = empty_mask;
    if(current_square.i != 7){
        // normal captures
        if(current_square.j > 0){
            mask[current_square.i+1][current_square.j-1] = 1;
        }
        if(current_square.j < 7){
            mask[current_square.i+1][current_square.j+1] = 1;
        }
    }
    return mask;
}

std::vector<Square> BlackPawnTargetSquares(Square current_square, Mask& your_pieces_mask, Mask& opponent_pieces_mask){
    std::vector<Square> target_squares;
    Square target_square;
    if(current_square.i != 7){
        // normal captures
        if(current_square.j > 0){
            if(opponent_pieces_mask[current_square.i+1][current_square.j-1]){
                target_square.i = current_square.i+1; target_square.j = current_square.j-1;
                target_squares.push_back(target_square);
            }
        }
        if(current_square.j < 7){
            if(opponent_pieces_mask[current_square.i+1][current_square.j+1]){
                target_square.i = current_square.i+1; target_square.j = current_square.j+1;
                target_squares.push_back(target_square);
            }
        }
        // normal advance
        if(!your_pieces_mask[current_square.i+1][current_square.j] && !opponent_pieces_mask[current_square.i+1][current_square.j]){
            target_square.i = current_square.i+1; target_square.j = current_square.j;
            target_squares.push_back(target_square);
            if(current_square.i==1 && !your_pieces_mask[current_square.i+2][current_square.j] && !opponent_pieces_mask[current_square.i+2][current_square.j]){
                target_square.i = current_square.i+2; target_square.j = current_square.j;
                target_squares.push_back(target_square);
            }
        }
    }
    return target_squares;
}

// ---------------------------------------------
// -----------    MANAGE MASKS   ---------------
// ---------------------------------------------
Mask WhitePiecesMask(Board board){
    Mask white_pieces_mask = empty_mask;
    for(int i=0; i<8; i++){
        for(int j=0; j<8; j++){
            char c = board[i][j];
            if(c==' ') { continue; }
            else if(c=='K' || c=='Q' || c=='B' || c=='R' || c=='N' || c=='P'){
                white_pieces_mask[i][j] = 1; 
            }
        }
    }
    return white_pieces_mask;
}
Mask BlackPiecesMask(Board board){
    Mask black_pieces_mask = empty_mask;
    for(int i=0; i<8; i++){
        for(int j=0; j<8; j++){
            char c = board[i][j];
            if(c==' ') { continue; }
            else if(c=='k' || c=='q' || c=='b' || c=='r' || c=='n' || c=='p'){
                black_pieces_mask[i][j] = 1; 
            }
        }
    }
    return black_pieces_mask;
}
Mask AllPiecesMask(Board board){
    Mask all_pieces_mask = empty_mask;
    for(int i=0; i<8; i++){
        for(int j=0; j<8; j++){
            char c = board[i][j];
            if(c==' ') { continue; }
            else{ all_pieces_mask[i][j] = 1; }
        }
    }
    return all_pieces_mask;
}
Mask WhiteCoveredSquaresMask(Board board){
    Mask white_covered_squares_mask = empty_mask;
    Mask all_pieces_mask = AllPiecesMask(board);
    for(int i=0; i<8; i++){
        for(int j=0; j<8; j++){
            char c = board[i][j];
            Square current_square; 
            current_square.i = i; current_square.j = j;
            // Rooks
            if(c == 'R'){ white_covered_squares_mask += RookCoveredSquaresMask(current_square, all_pieces_mask); }
            // Bishops
            else if(c == 'B'){ white_covered_squares_mask += BishopCoveredSquaresMask(current_square, all_pieces_mask); }
            // Queens
            else if(c == 'Q'){ white_covered_squares_mask += QueenCoveredSquaresMask(current_square, all_pieces_mask); }
            // Kings
            else if(c == 'K'){ white_covered_squares_mask += KingCoveredSquaresMask(current_square); }
            // Knights
            else if(c == 'N'){ white_covered_squares_mask += KnightCoveredSquaresMask(current_square); }
            // Pawns
            else if(c == 'P'){ white_covered_squares_mask += WhitePawnCoveredSquaresMask(current_square); }
        }
    }
    return white_covered_squares_mask;
}
Mask BlackCoveredSquaresMask(Board board){
    Mask black_covered_squares_mask = empty_mask;
    Mask all_pieces_mask = AllPiecesMask(board);
    Square current_square; 
    for(int i=0; i<8; i++){
        for(int j=0; j<8; j++){
            char c = board[i][j];
            current_square.i = i; current_square.j = j;
            // Rooks
            if(c == 'r'){ black_covered_squares_mask += RookCoveredSquaresMask(current_square, all_pieces_mask); }
            // Bishops
            else if(c == 'b'){ black_covered_squares_mask += BishopCoveredSquaresMask(current_square, all_pieces_mask); }
            // Queens
            else if(c == 'q'){ black_covered_squares_mask += QueenCoveredSquaresMask(current_square, all_pieces_mask); }
            // Kings
            else if(c == 'k'){ black_covered_squares_mask += KingCoveredSquaresMask(current_square); }
            // Knights
            else if(c == 'n'){ black_covered_squares_mask += KnightCoveredSquaresMask(current_square); }
            // Pawns
            else if(c == 'p'){ black_covered_squares_mask += BlackPawnCoveredSquaresMask(current_square); }
        }
    }
    return black_covered_squares_mask;
}
