#include <Baccala.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <typeinfo>


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
    else this->white_to_move = 0;

    // set castling rights
    std::cout << words[2] << "\n";
    for(char& c: words[2]){
        if(c == '-') { break; }
        else if(c == 'K') { this->can_white_castle_kingside = true; continue; }
        else if(c == 'Q') { this->can_white_castle_queenside = true; continue; }
        else if(c == 'k') { this->can_black_castle_kingside = true; continue; }
        else if(c == 'q') { this->can_black_castle_queenside = true; continue; }
    }

    // set en-passant target square if different from '-'
    if(words[3] != "-"){
        this->en_passant_target_square = AlphabetToSquare(words[3]);
    }

    // set half move counter and move counter from fen
    this->half_move_counter = std::stoi(words[4]);
    this->move_counter = std::stoi(words[5]);

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
    Square king_square;
    Board new_board = this->board;
    Mask opponent_covered_squares_mask;
    bool is_legal;
    // for a normal move which is not castling
    if(!move.is_castling){
        // create a new board after the move
        new_board[move.current_square.i][move.current_square.j] = ' ';
        new_board[move.target_square.i][move.target_square.j] = move.piece;
        // generate the mask of covered squares by the opponent after the move
        // then check if your king is in a square attacked by the opponent after the move
        if(this->white_to_move){
            if(move.piece == 'K'){ king_square = move.target_square; }
            else{ king_square = this->white_king_square; }
            opponent_covered_squares_mask = BlackCoveredSquaresMask(new_board);
            is_legal = !(opponent_covered_squares_mask[king_square.i][king_square.j]);
        }
        else{
            if(move.piece == 'k'){ king_square = move.target_square; }
            else{ king_square = this->black_king_square; }
            opponent_covered_squares_mask = WhiteCoveredSquaresMask(new_board);      
            is_legal = !(opponent_covered_squares_mask[king_square.i][king_square.j]);
        }
    }
    // manage castling: control if the king is in check and if it passes through a covered square while castling
    else{
        if(move.castling_side == 'K'){
            opponent_covered_squares_mask = BlackCoveredSquaresMask(this->board);
            is_legal = !opponent_covered_squares_mask[7][4] && !opponent_covered_squares_mask[7][5] && !opponent_covered_squares_mask[7][6];
        }
        else if(move.castling_side == 'Q'){
            opponent_covered_squares_mask = BlackCoveredSquaresMask(this->board);
            is_legal = !opponent_covered_squares_mask[7][4] && !opponent_covered_squares_mask[7][3] && !opponent_covered_squares_mask[7][2];
        }
        else if(move.castling_side == 'k'){
            opponent_covered_squares_mask = WhiteCoveredSquaresMask(this->board);
            is_legal = !opponent_covered_squares_mask[0][4] && !opponent_covered_squares_mask[0][5] && !opponent_covered_squares_mask[0][6];
        }
        else if(move.castling_side == 'q'){
            opponent_covered_squares_mask = WhiteCoveredSquaresMask(this->board);
            is_legal = !opponent_covered_squares_mask[0][4] && !opponent_covered_squares_mask[0][3] && !opponent_covered_squares_mask[0][2];
        }
    }
    return is_legal;
}

std::vector<Move> Position::LegalMoves(){
    // initialize stuff
    std::vector<Move> moves;
    moves.reserve(80);
    Square current_square; 
    std::vector<Square> target_squares;
    bool is_capture;
    // scan the board to find the pieces
    for(int i=0; i<8; i++){
        for(int j=0; j<8; j++){
            char c = this->board[i][j];
            if(this->white_to_move){
                // if it's white to move, ignore empty squares or squares with black pieces
                if(c==' ' || c=='p' || c=='k' || c=='q' || c=='r' || c=='b' || c=='n'){ continue; }
                // instead, depending on the piece, create the vector of target squares where the piece can go
                else{
                    current_square.i = i; current_square.j = j;
                    // Rook moves: for every target square, build the corresponding move
                    if(c == 'R'){
                        for(Square& target_square: RookTargetSquares(current_square, this->white_pieces_mask, this->black_pieces_mask)){
                            Move move = Move(current_square, target_square, c, this->black_pieces_mask[target_square.i][target_square.j]);
                            if(IsLegal(move)){ moves.push_back(move); }
                        }
                    }
                    else if(c == 'B'){
                        for(Square& target_square: BishopTargetSquares(current_square, this->white_pieces_mask, this->black_pieces_mask)){
                            Move move = Move(current_square, target_square, c, this->black_pieces_mask[target_square.i][target_square.j]);
                            if(IsLegal(move)){ moves.push_back(move); }
                        }
                    }
                    else if(c == 'Q'){
                        for(Square& target_square: QueenTargetSquares(current_square, this->white_pieces_mask, this->black_pieces_mask)){
                            Move move = Move(current_square, target_square, c, this->black_pieces_mask[target_square.i][target_square.j]);
                            if(IsLegal(move)){ moves.push_back(move); }
                        }
                    }
                    else if(c == 'K'){
                        for(Square& target_square: KingTargetSquares(current_square, this->white_pieces_mask)){
                            Move move = Move(current_square, target_square, c, this->black_pieces_mask[target_square.i][target_square.j]);
                            if(IsLegal(move)){ moves.push_back(move); }
                        }
                    }
                    else if(c == 'N'){
                        for(Square& target_square: KnightTargetSquares(current_square, this->white_pieces_mask)){
                            Move move = Move(current_square, target_square, c, this->black_pieces_mask[target_square.i][target_square.j]);
                            if(IsLegal(move)){ moves.push_back(move); }
                        }
                    }
                    else if(c == 'P'){
                        for(Square& target_square: WhitePawnTargetSquares(current_square, this->white_pieces_mask, this->black_pieces_mask, this->en_passant_target_square)){
                            // pawn promotion
                            if(target_square.i == 0){
                                for(int a=0; a<4; a++){
                                    Move move = Move(target_square, pieces_white_pawn_becomes[a]);
                                    if(IsLegal(move)){ moves.push_back(move); }
                                }
                            }
                            // normal pawn moves
                            else{
                                is_capture = (abs(target_square.j - current_square.j) == 1); // if pawn moves diagonally this is a capture
                                Move move = Move(current_square, target_square, c, is_capture);
                                if(IsLegal(move)){ moves.push_back(move); }
                            }
                        }
                    }
                }
            }
            // if it is black to move do the specular operations
            else{
                if(c==' ' || c=='P' || c=='K' || c=='Q' || c=='R' || c=='B' || c=='N'){ continue; }
                else{
                    current_square.i = i; current_square.j = j;
                    if(c == 'r'){
                        for(Square& target_square: RookTargetSquares(current_square, this->black_pieces_mask, this->white_pieces_mask)){
                            Move move = Move(current_square, target_square, c, this->white_pieces_mask[target_square.i][target_square.j]);
                            if(IsLegal(move)){ moves.push_back(move); }
                        }
                    }
                    else if(c == 'b'){
                        for(Square& target_square: BishopTargetSquares(current_square, this->black_pieces_mask, this->white_pieces_mask)){
                            Move move = Move(current_square, target_square, c, this->white_pieces_mask[target_square.i][target_square.j]);
                            if(IsLegal(move)){ moves.push_back(move); }
                        }
                    }
                    else if(c == 'q'){
                        for(Square& target_square: QueenTargetSquares(current_square, this->black_pieces_mask, this->white_pieces_mask)){
                            Move move = Move(current_square, target_square, c, this->white_pieces_mask[target_square.i][target_square.j]);
                            if(IsLegal(move)){ moves.push_back(move); }
                        }
                    }
                    else if(c == 'k'){
                        for(Square& target_square: KingTargetSquares(current_square, this->black_pieces_mask)){
                            Move move = Move(current_square, target_square, c, this->white_pieces_mask[target_square.i][target_square.j]);
                            if(IsLegal(move)){ moves.push_back(move); }
                        }
                    }
                    else if(c == 'n'){
                        for(Square& target_square: KnightTargetSquares(current_square, this->black_pieces_mask)){
                            Move move = Move(current_square, target_square, c, this->white_pieces_mask[target_square.i][target_square.j]);
                            if(IsLegal(move)){ moves.push_back(move); }
                        }
                    }
                    else if(c == 'p'){
                        for(Square& target_square: BlackPawnTargetSquares(current_square, this->black_pieces_mask, this->white_pieces_mask)){
                            // pawn promotion
                            if(target_square.i == 7){
                                for(int a=0; a<4; a++){
                                    Move move = Move(target_square, pieces_black_pawn_becomes[a]);
                                    if(IsLegal(move)){ moves.push_back(move); }
                                }
                            }
                            // normal pawn moves
                            else{
                                is_capture = (abs(target_square.j - current_square.j) == 1); // if pawn moves diagonally this is a capture
                                Move move = Move(current_square, target_square, c, is_capture);
                                if(IsLegal(move)){ moves.push_back(move); }
                            }
                        }
                    }
                }
            }
        }
    }
    // castling
    if(this->white_to_move){
        if(this->can_white_castle_kingside && this->board[7][4]=='K' && this->board[7][5]==' ' && this->board[7][6] == ' ' && this->board[7][7] == 'R'){
            Move move = Move('K');
            if(IsLegal(move)){ moves.push_back(move); }
        }
        else if(this->can_white_castle_queenside && this->board[7][0]=='R' && this->board[7][1]==' ' && this->board[7][2]==' ' && this->board[7][3]==' ' && this->board[7][4]=='K'){
            Move move = Move('Q');
            if(IsLegal(move)){ moves.push_back(move); }
        }
    }
    else{
        if(this->can_black_castle_kingside && this->board[0][4]=='k' && this->board[0][5]==' ' && this->board[0][6] == ' ' && this->board[0][7] == 'r'){
            Move move = Move('k');
            if(IsLegal(move)){ moves.push_back(move); }
        }
        else if(this->can_black_castle_queenside && this->board[0][0]=='r' && this->board[0][1]==' ' && this->board[0][2]==' ' && this->board[0][3]==' ' && this->board[0][4]=='k'){
            Move move = Move('q');
            if(IsLegal(move)){ moves.push_back(move); }
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
Mask WhitePawnMovesMask(Square current_square, Mask your_pieces_mask, Mask opponent_pieces_mask, Square en_passant_target_square){
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

std::vector<Square> WhitePawnTargetSquares(Square current_square, Mask your_pieces_mask, Mask opponent_pieces_mask, Square en_passant_target_square){
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

// GET NEW POSITION APPLYING A GIVEN MOVE TO AN OLD POSITION
// the input move is assumed to be a legal move given the old position
// if an illegal move is provided, unexpected behavior occurs
Position NewPosition(Position old_position, Move move){
    int iold = move.current_square.i;
    int jold = move.current_square.j;
    int inew = move.target_square.i;
    int jnew = move.target_square.j;
    Position new_position = old_position;
    // UPDATE THE BOARD
    // normal move (not castling) 
    if(!move.is_castling){
        new_position.board[iold][jold] = ' ';
        new_position.board[inew][jnew] = move.piece;
        new_position.material_value -= PieceValue(old_position.board[inew][jnew]);
        // change castling rights if the king or the rook move from the starting square
        if(move.piece == 'K'){
            new_position.can_white_castle_kingside = false; 
            new_position.can_white_castle_queenside = false;
            new_position.white_king_square = move.target_square;
        }
        else if(move.piece == 'k'){
            new_position.can_black_castle_kingside = false; 
            new_position.can_black_castle_queenside = false;
            new_position.black_king_square = move.target_square;
        }
        else if(move.piece == 'R' && iold == 7 && jold == 7){new_position.can_white_castle_kingside = false;}
        else if(move.piece == 'R' && iold == 7 && jold == 0){new_position.can_white_castle_queenside = false;}
        else if(move.piece == 'r' && iold == 0 && jold == 7){new_position.can_black_castle_kingside = false;}
        else if(move.piece == 'r' && iold == 0 && jold == 0){new_position.can_black_castle_queenside = false;}
        // pawn promotion
        else if(move.piece == 'P' && inew == 0){
            new_position.board[0][jnew] = move.new_piece; 
            new_position.material_value += PieceValue(move.new_piece) - 1; 
        }
        else if(move.piece == 'p' && inew == 7){
            new_position.board[7][jnew] = move.new_piece;
            new_position.material_value += PieceValue(move.new_piece) - 1; 
        }
        // set en passant target square when a pawn moves two squares ahead
        else if(move.piece == 'P' && iold == 6 && inew == 4){ new_position.en_passant_target_square = {5, jnew}; }
        else if(move.piece == 'p' && iold == 1 && inew == 3){ new_position.en_passant_target_square = {2, jnew}; }
        // if a pawn captures en-passant (i.e. pawn moves diagonally but target square is empty), make sure to delete the eaten pawn from the board
        else if(move.piece == 'P' && abs(jnew - jold) == 1 && !old_position.black_pieces_mask[inew][jnew]){
            new_position.board[inew+1][jnew] = ' ';
        }
        else if(move.piece == 'p' && abs(jnew - jold) == 1 && !old_position.white_pieces_mask[inew][jnew]){
            new_position.board[inew-1][jnew] = ' ';
        }
    }
    // castling
    else{
        if(move.castling_side == 'K'){
            new_position.board[7][7] = ' '; new_position.board[7][6] = 'K'; new_position.board[7][5] = 'R'; new_position.board[7][4] = ' ';
            new_position.can_white_castle_kingside = false;
            new_position.can_white_castle_queenside = false;
            new_position.white_king_square = {7, 6};
        }
        else if(move.castling_side == 'Q'){
            new_position.board[7][0] = ' '; new_position.board[7][1] = ' '; new_position.board[7][2] = 'K'; new_position.board[7][3] = 'R'; new_position.board[7][4] = ' ';
            new_position.can_white_castle_kingside = false;
            new_position.can_white_castle_queenside = false;
            new_position.white_king_square = {7, 2};
        }
        else if(move.castling_side == 'k'){
            new_position.board[0][7] = ' '; new_position.board[0][6] = 'k'; new_position.board[0][5] = 'r'; new_position.board[0][4] = ' ';
            new_position.can_black_castle_kingside = false;
            new_position.can_black_castle_queenside = false;
            new_position.black_king_square = {0, 6};
        }
        else if(move.castling_side == 'q'){
            new_position.board[0][0] = ' '; new_position.board[0][1] = ' '; new_position.board[0][2] = 'k'; new_position.board[0][3] = 'r'; new_position.board[0][4] = ' ';
            new_position.can_black_castle_kingside = false;
            new_position.can_black_castle_queenside = false;
            new_position.black_king_square = {0, 2};
        }
    }
    // update side to move and move counter
    if(old_position.white_to_move){ new_position.move_counter += 1; }
    if(move.is_capture || move.piece == 'P' || move.piece == 'p'){ new_position.half_move_counter = 0; }
    else{ new_position.half_move_counter += 1; }
    new_position.white_to_move = !old_position.white_to_move;    
    // UPDATE THE MASKS - not the most elegant way, we are copy-pasting blocks of code
    // you can improve here: there's a smarter way than looping through the board twice...
    if(old_position.white_to_move){
        new_position.white_pieces_mask[iold][jold] = 0;
        new_position.white_pieces_mask[inew][jnew] = 1;
        new_position.black_pieces_mask[inew][jnew] = 0;
    }
    else{
        new_position.black_pieces_mask[iold][jold] = 0;
        new_position.black_pieces_mask[inew][jnew] = 1;
        new_position.white_pieces_mask[inew][jnew] = 0;
    }
    new_position.all_pieces_mask[inew][jnew] = 1;
    new_position.all_pieces_mask[iold][jold] = 0;
    new_position.white_covered_squares_mask = empty_mask;
    new_position.black_covered_squares_mask = empty_mask;
    for(int i=0; i<8; i++){
        for(int j=0; j<8; j++){
            char c = new_position.board[i][j];
            if(c == ' ') { continue; }
            // Rooks
            else if(c == 'R'){ new_position.white_covered_squares_mask += RookCoveredSquaresMask({i, j}, new_position.all_pieces_mask); }
            else if(c == 'r'){ new_position.black_covered_squares_mask += RookCoveredSquaresMask({i, j}, new_position.all_pieces_mask); }
            // Bishops
            else if(c == 'B'){ new_position.white_covered_squares_mask += BishopCoveredSquaresMask({i, j}, new_position.all_pieces_mask); }
            else if(c == 'b'){ new_position.black_covered_squares_mask += BishopCoveredSquaresMask({i, j}, new_position.all_pieces_mask); }
            // Queens
            else if(c == 'Q'){ new_position.white_covered_squares_mask += QueenCoveredSquaresMask({i, j}, new_position.all_pieces_mask); }
            else if(c == 'q'){ new_position.black_covered_squares_mask += QueenCoveredSquaresMask({i, j}, new_position.all_pieces_mask); }
            // Kings
            else if(c == 'K'){ new_position.white_covered_squares_mask += KingCoveredSquaresMask({i, j}); }
            else if(c == 'k'){ new_position.black_covered_squares_mask += KingCoveredSquaresMask({i, j}); }
            // Knights
            else if(c == 'N'){ new_position.white_covered_squares_mask += KnightCoveredSquaresMask({i, j}); }
            else if(c == 'n'){ new_position.black_covered_squares_mask += KnightCoveredSquaresMask({i, j}); }
            // Pawns
            else if(c == 'P'){ new_position.white_covered_squares_mask += WhitePawnCoveredSquaresMask({i, j}); }
            else if(c == 'p'){ new_position.black_covered_squares_mask += BlackPawnCoveredSquaresMask({i, j}); }
        }
    }
    // return the new position, NOTICE THAT THE CORRESPONDING FEN STRING IS NOT UPDATED!!!
    return new_position;
}