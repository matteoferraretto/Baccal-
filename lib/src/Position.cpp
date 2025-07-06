#include <Baccala.h>
#include <Position.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <typeinfo>
#include <cctype>

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
        // if pawn promotion
        if(move.new_piece != 'z'){ 
            new_board[move.target_square.i][move.target_square.j] = move.new_piece;
        }
        else{
            new_board[move.target_square.i][move.target_square.j] = move.piece;
        }
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
                            is_capture = this->black_pieces_mask[target_square.i][target_square.j];
                            Move move = Move(current_square, target_square, c, is_capture);
                            if(IsLegal(move)){
                                // rank the move
                                move.rank = -PieceValue(this->board[target_square.i][target_square.j]);
                                moves.push_back(move);
                            }
                        }
                    }
                    else if(c == 'B'){
                        for(Square& target_square: BishopTargetSquares(current_square, this->white_pieces_mask, this->black_pieces_mask)){
                            is_capture = this->black_pieces_mask[target_square.i][target_square.j];
                            Move move = Move(current_square, target_square, c, is_capture);
                            if(IsLegal(move)){
                                move.rank = -PieceValue(this->board[target_square.i][target_square.j]);
                                moves.push_back(move); 
                            }
                        }
                    }
                    else if(c == 'Q'){
                        for(Square& target_square: QueenTargetSquares(current_square, this->white_pieces_mask, this->black_pieces_mask)){
                            is_capture = this->black_pieces_mask[target_square.i][target_square.j];
                            Move move = Move(current_square, target_square, c, is_capture);
                            if(IsLegal(move)){
                                move.rank = -PieceValue(this->board[target_square.i][target_square.j]);
                                moves.push_back(move);
                            }
                        }
                    }
                    else if(c == 'K'){
                        for(Square& target_square: KingTargetSquares(current_square, this->white_pieces_mask)){
                            is_capture = this->black_pieces_mask[target_square.i][target_square.j];
                            Move move = Move(current_square, target_square, c, is_capture);
                            if(IsLegal(move)){
                                move.rank = -PieceValue(this->board[target_square.i][target_square.j]);
                                moves.push_back(move);
                            }
                        }
                    }
                    else if(c == 'N'){
                        for(Square& target_square: KnightTargetSquares(current_square, this->white_pieces_mask)){
                            is_capture = this->black_pieces_mask[target_square.i][target_square.j];
                            Move move = Move(current_square, target_square, c, is_capture);
                            if(IsLegal(move)){
                                // rank the move and insert it at the beginning of the array if it is ranked best
                                move.rank = -PieceValue(this->board[target_square.i][target_square.j]);
                                moves.push_back(move);
                            }
                        }
                    }
                    else if(c == 'P'){
                        for(Square& target_square: WhitePawnTargetSquares(current_square, this->white_pieces_mask, this->black_pieces_mask, this->en_passant_target_square)){
                            // pawn promotion
                            if(target_square.i == 0){
                                for(int a=0; a<4; a++){
                                    Move move = Move(target_square, pieces_white_pawn_becomes[a]);
                                    if(IsLegal(move)){
                                        move.rank = PieceValue(pieces_white_pawn_becomes[a]) - 1;
                                        moves.push_back(move);
                                    }
                                }
                            }
                            // normal pawn moves
                            else{
                                is_capture = (abs(target_square.j - current_square.j) == 1); // if pawn moves diagonally this is a capture
                                Move move = Move(current_square, target_square, c, is_capture);
                                if(IsLegal(move)){
                                    move.rank = -PieceValue(this->board[target_square.i][target_square.j]);
                                    moves.push_back(move);
                                }
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
                            is_capture = this->white_pieces_mask[target_square.i][target_square.j];
                            Move move = Move(current_square, target_square, c, is_capture);
                            if(IsLegal(move)){
                                move.rank = PieceValue(this->board[target_square.i][target_square.j]);
                                moves.push_back(move);
                            }
                        }
                    }
                    else if(c == 'b'){
                        for(Square& target_square: BishopTargetSquares(current_square, this->black_pieces_mask, this->white_pieces_mask)){
                            is_capture = this->white_pieces_mask[target_square.i][target_square.j];
                            Move move = Move(current_square, target_square, c, is_capture);
                            if(IsLegal(move)){
                                move.rank = PieceValue(this->board[target_square.i][target_square.j]);
                                moves.push_back(move);
                            }
                        }
                    }
                    else if(c == 'q'){
                        for(Square& target_square: QueenTargetSquares(current_square, this->black_pieces_mask, this->white_pieces_mask)){
                            is_capture = this->white_pieces_mask[target_square.i][target_square.j];
                            Move move = Move(current_square, target_square, c, is_capture);
                            if(IsLegal(move)){
                                move.rank = PieceValue(this->board[target_square.i][target_square.j]);
                                moves.push_back(move);
                            }
                        }
                    }
                    else if(c == 'k'){
                        for(Square& target_square: KingTargetSquares(current_square, this->black_pieces_mask)){
                            is_capture = this->white_pieces_mask[target_square.i][target_square.j];
                            Move move = Move(current_square, target_square, c, is_capture);
                            if(IsLegal(move)){
                                move.rank = PieceValue(this->board[target_square.i][target_square.j]);
                                moves.push_back(move);
                            }
                        }
                    }
                    else if(c == 'n'){
                        for(Square& target_square: KnightTargetSquares(current_square, this->black_pieces_mask)){
                            is_capture = this->white_pieces_mask[target_square.i][target_square.j];
                            Move move = Move(current_square, target_square, c, is_capture);
                            if(IsLegal(move)){
                                move.rank = PieceValue(this->board[target_square.i][target_square.j]);
                                moves.push_back(move);
                            }
                        }
                    }
                    else if(c == 'p'){
                        for(Square& target_square: BlackPawnTargetSquares(current_square, this->black_pieces_mask, this->white_pieces_mask)){
                            // pawn promotion
                            if(target_square.i == 7){
                                for(int a=0; a<4; a++){
                                    Move move = Move(target_square, pieces_black_pawn_becomes[a]);
                                    if(IsLegal(move)){
                                        move.rank = -PieceValue(pieces_white_pawn_becomes[a]) - 1;
                                        moves.push_back(move);
                                    }
                                }
                            }
                            // normal pawn moves
                            else{
                                is_capture = (abs(target_square.j - current_square.j) == 1); // if pawn moves diagonally this is a capture
                                Move move = Move(current_square, target_square, c, is_capture);
                                if(IsLegal(move)){
                                    move.rank = PieceValue(this->board[target_square.i][target_square.j]);
                                    moves.push_back(move);
                                }
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
    SortMoves(moves);
    return moves;
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
    }
    // castling
    else{
        if(move.castling_side == 'K'){
            new_position.board[7][7] = ' '; new_position.board[7][6] = 'K'; new_position.board[7][5] = 'R'; new_position.board[7][4] = ' ';
            new_position.can_white_castle_kingside = false;
            new_position.can_white_castle_queenside = false;
            new_position.white_king_square = {7, 6};
            new_position.white_pieces_mask[7][7] = 0;
            new_position.white_pieces_mask[7][6] = 1;
            new_position.white_pieces_mask[7][5] = 1;
            new_position.white_pieces_mask[7][4] = 0;
            new_position.all_pieces_mask[7][7] = 0;
            new_position.all_pieces_mask[7][6] = 1;
            new_position.all_pieces_mask[7][5] = 1;
            new_position.all_pieces_mask[7][4] = 0;
        }
        else if(move.castling_side == 'Q'){
            new_position.board[7][0] = ' '; new_position.board[7][1] = ' '; new_position.board[7][2] = 'K'; new_position.board[7][3] = 'R'; new_position.board[7][4] = ' ';
            new_position.can_white_castle_kingside = false;
            new_position.can_white_castle_queenside = false;
            new_position.white_king_square = {7, 2};
            new_position.white_pieces_mask[7][0] = 0;
            new_position.white_pieces_mask[7][1] = 0;
            new_position.white_pieces_mask[7][2] = 1;
            new_position.white_pieces_mask[7][3] = 1;
            new_position.white_pieces_mask[7][4] = 0;
            new_position.all_pieces_mask[7][0] = 0;
            new_position.all_pieces_mask[7][1] = 0;
            new_position.all_pieces_mask[7][2] = 1;
            new_position.all_pieces_mask[7][3] = 1;
            new_position.all_pieces_mask[7][4] = 0;
            
        }
        else if(move.castling_side == 'k'){
            new_position.board[0][7] = ' '; new_position.board[0][6] = 'k'; new_position.board[0][5] = 'r'; new_position.board[0][4] = ' ';
            new_position.can_black_castle_kingside = false;
            new_position.can_black_castle_queenside = false;
            new_position.black_king_square = {0, 6};
            new_position.black_pieces_mask[0][7] = 0;
            new_position.black_pieces_mask[0][6] = 1;
            new_position.black_pieces_mask[0][5] = 1;
            new_position.black_pieces_mask[0][4] = 0;
            new_position.all_pieces_mask[0][7] = 0;
            new_position.all_pieces_mask[0][6] = 1;
            new_position.all_pieces_mask[0][5] = 1;
            new_position.all_pieces_mask[0][4] = 0;
        }
        else if(move.castling_side == 'q'){
            new_position.board[0][0] = ' '; new_position.board[0][1] = ' '; new_position.board[0][2] = 'k'; new_position.board[0][3] = 'r'; new_position.board[0][4] = ' ';
            new_position.can_black_castle_kingside = false;
            new_position.can_black_castle_queenside = false;
            new_position.black_king_square = {0, 2};
            new_position.black_pieces_mask[0][0] = 0;
            new_position.black_pieces_mask[0][1] = 0;
            new_position.black_pieces_mask[0][2] = 1;
            new_position.black_pieces_mask[0][3] = 1;
            new_position.black_pieces_mask[0][4] = 0;
            new_position.all_pieces_mask[0][0] = 0;
            new_position.all_pieces_mask[0][1] = 0;
            new_position.all_pieces_mask[0][2] = 1;
            new_position.all_pieces_mask[0][3] = 1;
            new_position.all_pieces_mask[0][4] = 0;
        }
    }
    // update side to move and move counter
    if(old_position.white_to_move){ new_position.move_counter += 1; }
    if(move.is_capture || move.piece == 'P' || move.piece == 'p'){ new_position.half_move_counter = 0; }
    else{ new_position.half_move_counter += 1; }
    new_position.white_to_move = !old_position.white_to_move;    
    // update covered squares mask
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


// ----------------------------------------------------
EfficientPosition::EfficientPosition(std::string fen)
{
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
    int c_casted; 
    for(char& c: words[0]){
        c_casted = c - '0';
        if(c_casted == 8) { continue; }
        if(c_casted == -1) { i++; j = 0; continue; } // if c is '/', increment row index and reset column index to 0
        if(c_casted > 0 && c_casted < 8) { j += c_casted; continue; }
        this->pieces.push_back(c);
        this->ranks.push_back(i);
        this->files.push_back(j);
        j++; 
    }

    // set side to move
    if(words[1] == "w") { this->white_to_move = 1; }
    else this->white_to_move = 0;

    // set castling rights
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

    // evaluate masks of white, black and all pieces and material value
    char piece;
    for(unsigned int index = 0; index < pieces.size(); index++){
        piece = this->pieces[index];
        i = this->ranks[index];
        j = this->files[index];
        this->all_pieces_mask[i][j] = 1;
        this->material_value += PieceValue(piece);
    }
    // loop again
    Square current_square;
    for(unsigned int index = 0; index < pieces.size(); index++){
        piece = this->pieces[index];
        i = this->ranks[index];
        j = this->files[index];
        current_square = {i, j};
        // Rooks
        if(piece == 'R'){ 
            this->white_covered_squares_mask += RookCoveredSquaresMask(current_square, this->all_pieces_mask); 
            this->white_pieces_mask[i][j] = 1;
        }
        else if(piece == 'r'){ 
            this->black_covered_squares_mask += RookCoveredSquaresMask(current_square, this->all_pieces_mask); 
            this->black_pieces_mask[i][j] = 1;
        }
        // Bishops
        else if(piece == 'B'){ 
            this->white_covered_squares_mask += BishopCoveredSquaresMask(current_square, this->all_pieces_mask); 
            this->white_pieces_mask[i][j] = 1;
        }
        else if(piece == 'b'){ 
            this->black_covered_squares_mask += BishopCoveredSquaresMask(current_square, this->all_pieces_mask); 
            this->black_pieces_mask[i][j] = 1;
        }
        // Queens
        else if(piece == 'Q'){ 
            this->white_covered_squares_mask += QueenCoveredSquaresMask(current_square, this->all_pieces_mask); 
            this->white_pieces_mask[i][j] = 1;
        }
        else if(piece == 'q'){ 
            this->black_covered_squares_mask += QueenCoveredSquaresMask(current_square, this->all_pieces_mask); 
            this->black_pieces_mask[i][j] = 1;
        }
        // Kings
        else if(piece == 'K'){ 
            this->white_covered_squares_mask += KingCoveredSquaresMask(current_square); 
            this->white_pieces_mask[i][j] = 1;
        }
        else if(piece == 'k'){ 
            this->black_covered_squares_mask += KingCoveredSquaresMask(current_square); 
            this->black_pieces_mask[i][j] = 1;
        }
        // Knights
        else if(piece == 'N'){ 
            this->white_covered_squares_mask += KnightCoveredSquaresMask(current_square); 
            this->white_pieces_mask[i][j] = 1;
        }
        else if(piece == 'n'){ 
            this->black_covered_squares_mask += KnightCoveredSquaresMask(current_square); 
            this->black_pieces_mask[i][j] = 1;
        }
        // Pawns
        else if(piece == 'P'){ 
            this->white_covered_squares_mask += WhitePawnCoveredSquaresMask(current_square); 
            this->white_pieces_mask[i][j] = 1;
        }
        else if(piece == 'p'){ 
            this->black_covered_squares_mask += BlackPawnCoveredSquaresMask(current_square); 
            this->black_pieces_mask[i][j] = 1;
        }
    }
}

EfficientPosition::~EfficientPosition()
{
}

void EfficientPosition::PrintBoard(void){
    char board[64];
    for(int i=0; i<64; i++){ board[i] = '0'; }
    int i; 
    int j;
    char piece;
    for(unsigned int index = 0; index < this->pieces.size(); index++){
        piece = this->pieces[index]; i = this->ranks[index]; j = this->files[index];
        board[8*i + j] = piece;
    }
    // print
    for(int n = 0; n < 64; n++){
        if(n % 8 == 0){ std::cout << "\n"; }
        std::cout << board[n] << " ";
    }
    std::cout << "\n";
};

/*
// generate the list of all the child positions obtained from the current one applying all the legal moves
// sort the new positions based on the likelihood of being good for the current side to move
// i.e. checks, captures and pawn promotions first.
std::vector<EfficientPosition> New_Positions(EfficientPosition& pos){
    // initialize stuff
    std::vector<EfficientPosition> new_positions;
    new_positions.reserve(80);
    EfficientPosition new_pos = pos;
    char piece;
    int i;
    int j;
    // loop over all the pieces
    for(unsigned int index = 0; index < pos.pieces.size(); index++){
        piece = pos.pieces[index];
        // if piece has wrong color, skip
        if(pos.white_to_move && islower(piece)){ continue; }
        if(!pos.white_to_move && isupper(piece)){ continue; }
        // if you pick your own piece, elaborate moves
        i = pos.ranks[index];
        j = pos.files[index];
        // MANAGE WHITE ROOKS
        if(piece == 'R'){
            // side to side sliding

        }
        // MANAGE WHITE BISHOPS
        else if(piece == 'B'){

        }
    }
}
*/