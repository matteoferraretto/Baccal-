#include <Baccala.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>


Move::Move(Square current_square, Square target_square, char piece, bool is_capture){
    this->current_square = current_square; 
    this->target_square = target_square; 
    this->piece = piece;
    this->is_special_move = false; 
    this->is_capture = is_capture;
}

void Move::PrintMove(){
    std::cout << "the piece " << this->piece << " moves from (" << this->current_square.i << ", " << this->current_square.j << ") to (" << this->target_square.i << ", " << this->target_square.j << "). ";
    if(this->is_capture){ std::cout << "This is a capture."; }
    std::cout << "\n";
}

std::string Move::AlgebraicNotation(){
    return "not supported yet";
}

Move::~Move(){};

// CONSTRUCTOR FOR POSITION
// given a fen string, this computes all the info relevant to the position
Position::Position(std::string fen)
{
    this->fen = fen;

    // take the fen string and separate the "words" separated by spaces
    std::stringstream ss(fen);
    std::vector<std::string> words;
    std::string word;
    while (ss >> word) {
        words.push_back(word);
    }

    // get board in the form of a 8x8 char matrix
    int i = 0; int j = 0; int c_casted;
    for(char& c: words[0]){
        c_casted = c - '0';
        if(c_casted == 8) { continue; }
        if(c_casted == -1) { i++; j = 0; continue; } // if c is '/', increment row index and reset column index to 0
        if(c_casted > 0 && c_casted < 8) { j += c_casted; continue; }
        this->board[i][j] = c; j++;
    }

    // set side to move
    if(words[1] == "w") { this->white_to_move = 1; }
    else this->white_to_move = 1;

    // set castling rights
    std::cout << words[2] << "\n";
    for(char& c: words[2]){
        if(c == '-') { break; }
        else if(c == 'K') { this->can_white_castle_kingside = true; continue; }
        else if(c == 'Q') { this->can_white_castle_queenside = true; continue; }
        else if(c == 'k') { this->can_black_castle_kingside = true; continue; }
        else if(c == 'q') { this->can_black_castle_queenside = true; continue; }
    }

    // evaluate masks of white, black and all pieces, position of the kings and material value
    for(int i=0; i<8; i++){
        for(int j=0; j<8; j++){
            char c = this->board[i][j];
            if(c==' ') { continue; }
            else if(c=='K' || c=='Q' || c=='B' || c=='R' || c=='N' || c=='P'){
                this->white_pieces_mask[i][j] = 1; 
                this->all_pieces_mask[i][j] = 1;
                if(c=='K'){ this->white_king_square.i = i; this->white_king_square.j = j; }
                this->material_value += PieceValue(c);
            }
            else if(c=='k' || c=='q' || c=='b' || c=='r' || c=='n' || c=='p'){
                this->black_pieces_mask[i][j] = 1;
                this->all_pieces_mask[i][j] = 1;
                if(c=='k'){ this->black_king_square.i = i; this->black_king_square.j = j; }
                this->material_value += PieceValue(c);
            }
        }
    }

    // evaluate masks of covered squares by black and white
    for(int i=0; i<8; i++){
        for(int j=0; j<8; j++){
            char c = this->board[i][j];
            Square current_square; 
            current_square.i = i; current_square.j = j;
            if(c == ' ') { continue; }
            // Rooks
            else if(c == 'R'){ this->white_covered_squares_mask += RookCoveredSquaresMask(current_square, this->all_pieces_mask); }
            else if(c == 'r'){ this->black_covered_squares_mask += RookCoveredSquaresMask(current_square, this->all_pieces_mask); }
            // Bishops
            else if(c == 'B'){ this->white_covered_squares_mask += BishopCoveredSquaresMask(current_square, this->all_pieces_mask); }
            else if(c == 'b'){ this->black_covered_squares_mask += BishopCoveredSquaresMask(current_square, this->all_pieces_mask); }
            // Queens
            else if(c == 'Q'){ this->white_covered_squares_mask += QueenCoveredSquaresMask(current_square, this->all_pieces_mask); }
            else if(c == 'q'){ this->black_covered_squares_mask += QueenCoveredSquaresMask(current_square, this->all_pieces_mask); }
            // Kings
            else if(c == 'K'){ this->white_covered_squares_mask += KingCoveredSquaresMask(current_square); }
            else if(c == 'k'){ this->black_covered_squares_mask += KingCoveredSquaresMask(current_square); }
            // Knights
            else if(c == 'N'){ this->white_covered_squares_mask += KnightCoveredSquaresMask(current_square); }
            else if(c == 'n'){ this->black_covered_squares_mask += KnightCoveredSquaresMask(current_square); }
            // Pawns
            else if(c == 'P'){ this->white_covered_squares_mask += WhitePawnCoveredSquaresMask(current_square); }
            else if(c == 'p'){ this->black_covered_squares_mask += BlackPawnCoveredSquaresMask(current_square); }
        }
    }
}

Position::~Position()
{
}

void Position::PrintBoard(void){
    char c;
    for(int i=0; i<8; i++){
        for(int j=0; j<8; j++){
            c = this->board[i][j];
            if (c == ' ') { c = '0'; }
            std::cout << c << " ";
        }
        std::cout << "\n";
    }
};

bool Position::IsLegal(Move move){
    // create a new board after the move
    Board new_board = this->board;
    new_board[move.current_square.i][move.current_square.j] = ' ';
    new_board[move.target_square.i][move.target_square.j] = move.piece;
    // generate the mask of covered squares by the opponent after the move
    // then check if your king is in a square attacked by the opponent after the move
    Mask opponent_covered_squares_mask;
    bool is_legal;
    if(this->white_to_move){
        opponent_covered_squares_mask = BlackCoveredSquaresMask(new_board);
        is_legal = !(opponent_covered_squares_mask[this->white_king_square.i][this->white_king_square.j]);
    }
    else{
        opponent_covered_squares_mask = WhiteCoveredSquaresMask(new_board);      
        is_legal = !(opponent_covered_squares_mask[this->black_king_square.i][this->black_king_square.j]);
    }
    return is_legal;
}

std::vector<Move> Position::LegalMoves(){
    // initialize stuff
    std::vector<Move> moves;
    Square current_square; 
    std::vector<Square> target_squares;
    // scan the board to find the pieces
    for(int i=0; i<8; i++){
        for(int j=0; j<8; j++){
            char c = this->board[i][j];
            if(this->white_to_move){
                // if it's white to move, ignore empty squares or squares with black pieces
                if(c==' ' || c=='p' || c=='k' || c=='q' || c=='r' || c=='b' || c=='n'){
                    continue;
                }
                // instead, depending on the piece create the vector of target squares where the piece can go
                else{
                    current_square.i = i; current_square.j = j;
                    switch (c)
                    {
                    case 'R': // Rook moves: for every target square, build the corresponding move
                        target_squares = RookTargetSquares(current_square, this->white_pieces_mask, this->black_pieces_mask);
                        for(Square& target_square: target_squares){
                            Move move = Move(current_square, target_square, c, this->black_pieces_mask[target_square.i][target_square.j]);
                            moves.push_back(move);
                        }
                        break;
                    case 'Q':
                        break;
                    
                    default:
                        break;
                    }
                }
            }
        }
    }
    return moves;
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
    }
    return value;
};

// ---------------------------------------------
// --------    MANAGE KNIGTHS   ----------------
// ---------------------------------------------
Mask KnightMovesMask(Square current_square, Mask your_pieces_mask) {
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

std::vector<Square> KnightTargetSquares(Square current_square, Mask your_pieces_mask){
    std::vector<Square> target_squares;
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
Mask RookMovesMask(Square current_square, Mask your_pieces_mask, Mask opponent_pieces_mask) {
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

Mask RookCoveredSquaresMask(Square current_square, Mask all_pieces_mask) {
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

std::vector<Square> RookTargetSquares(Square current_square, Mask your_pieces_mask, Mask opponent_pieces_mask){
    std::vector<Square> moves;
    Square target_square = current_square;
    // bottom sliding
    for(int x = current_square.i+1; x < 8; x++){
        target_square.i = x;
        // if you encounter your own piece in the sliding, stop
        if(your_pieces_mask[x][current_square.j]) { break; }
        // if you encounter an opponent's piece in the sliding, consider the move and stop
        if(opponent_pieces_mask[x][current_square.j]) { moves.push_back(target_square); break; }
        // else just consider the move
        moves.push_back(target_square);
    }
    // top sliding
    for(int x = current_square.i-1; x >= 0; x--){
        target_square.i = x;
        if(your_pieces_mask[x][current_square.j]) { break; }
        if(opponent_pieces_mask[x][current_square.j] ) { moves.push_back(target_square); break; }
        moves.push_back(target_square);
    }
    target_square.i = current_square.i;
    // right sliding
    for(int y = current_square.j+1; y < 8; y++){
        target_square.j = y;
        if(your_pieces_mask[current_square.i][y]) { break; }
        if(opponent_pieces_mask[current_square.i][y] ) { moves.push_back(target_square); break; }
        moves.push_back(target_square);
    }
    // left sliding
    for(int y = current_square.j-1; y >= 0; y--){
        target_square.j = y;
        if(your_pieces_mask[current_square.i][y]) { break; }
        if(opponent_pieces_mask[current_square.i][y] ) { moves.push_back(target_square); break; }
        moves.push_back(target_square);
    }
    return moves;
}

// ---------------------------------------------
// ----------    MANAGE BiSHOPS   --------------
// ---------------------------------------------
Mask BishopMovesMask(Square current_square, Mask your_pieces_mask, Mask opponent_pieces_mask) {
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

Mask BishopCoveredSquaresMask(Square current_square, Mask all_pieces_mask) {
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

std::vector<Square> BishopTargetSquares(Square current_square, Mask your_pieces_mask, Mask opponent_pieces_mask){
    std::vector<Square> moves;
    Square target_square = current_square;
    // slide to bottom right
    for(int step=1; step<8; step++){
        target_square.i++;
        target_square.j++;
        if(target_square.i > 7 || target_square.j > 7 || your_pieces_mask[target_square.i][target_square.j]){ break; }
        if(opponent_pieces_mask[target_square.i][target_square.j]){ moves.push_back(target_square); break; }
        moves.push_back(target_square);
    }
    // slide to top left
    target_square = current_square;
    for(int step=1; step<8; step++){
        target_square.i--;
        target_square.j--;
        if(target_square.i < 0 || target_square.j < 0 || your_pieces_mask[target_square.i][target_square.j]){ break; }
        if(opponent_pieces_mask[target_square.i][target_square.j]){ moves.push_back(target_square); break; }
        moves.push_back(target_square);
    }
    // slide to bottom left
    target_square = current_square;
    for(int step=1; step<8; step++){
        target_square.i++;
        target_square.j--;
        if(target_square.i > 7 || target_square.j < 0 || your_pieces_mask[target_square.i][target_square.j]){ break; }
        if(opponent_pieces_mask[target_square.i][target_square.j]){ moves.push_back(target_square); break; }
        moves.push_back(target_square);
    }
    // slide to top right
    target_square = current_square;
    for(int step=1; step<8; step++){
        target_square.i--;
        target_square.j++;
        if(target_square.i < 0 || target_square.j > 7 || your_pieces_mask[target_square.i][target_square.j]){ break; }
        if(opponent_pieces_mask[target_square.i][target_square.j]){ moves.push_back(target_square); break; }
        moves.push_back(target_square);
    }
    return moves;
}

// ---------------------------------------------
// -----------    MANAGE QUEENS   --------------
// ---------------------------------------------
Mask QueenMovesMask(Square current_square, Mask your_pieces_mask, Mask opponent_pieces_mask) {
    return (RookMovesMask(current_square, your_pieces_mask, opponent_pieces_mask) + BishopMovesMask(current_square, your_pieces_mask, opponent_pieces_mask));
}

Mask QueenCoveredSquaresMask(Square current_square, Mask all_pieces_mask) {
    return (RookCoveredSquaresMask(current_square, all_pieces_mask) + BishopCoveredSquaresMask(current_square, all_pieces_mask));
}

std::vector<Square> QueenTargetSquares(Square current_square, Mask your_pieces_mask, Mask opponent_pieces_mask){
    std::vector<Square> moves = RookTargetSquares(current_square, your_pieces_mask, opponent_pieces_mask);
    std::vector<Square> bishop_moves = BishopTargetSquares(current_square, your_pieces_mask, opponent_pieces_mask);
    moves.insert(moves.end(), bishop_moves.begin(), bishop_moves.end());
    return moves;
}

// ---------------------------------------------
// -----------    MANAGE KINGS   ---------------
// ---------------------------------------------
Mask KingMovesMask(Square current_square, Mask your_pieces_mask) {
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

std::vector<Square> KingTargetSquares(Square current_square, Mask your_pieces_mask){
    std::vector<Square> target_squares;
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
Mask WhitePawnMovesMask(Square current_square, Mask your_pieces_mask, Mask opponent_pieces_mask){
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

std::vector<Square> WhitePawnTargetSquares(Square current_square, Mask your_pieces_mask, Mask opponent_pieces_mask){
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

Mask BlackPawnMovesMask(Square current_square, Mask your_pieces_mask, Mask opponent_pieces_mask){
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

std::vector<Square> BlackPawnTargetSquares(Square current_square, Mask your_pieces_mask, Mask opponent_pieces_mask){
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
    for(int i=0; i<8; i++){
        for(int j=0; j<8; j++){
            char c = board[i][j];
            Square current_square; 
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
            else if(c == 'p'){ black_covered_squares_mask += WhitePawnCoveredSquaresMask(current_square); }
        }
    }
    return black_covered_squares_mask;
}