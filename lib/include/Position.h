#pragma once
#include <iostream>
#include <string>
#include <vector>
#include "Baccala.h"

// Class for chess position
class Position
{
private:
public:
    std::string fen;
    bool white_to_move; // 1 = white; 0 = black
    Board board = empty_board;
    char castling_rights; // <---- we can deprecate this
    bool can_white_castle_kingside = false;
    bool can_white_castle_queenside = false;
    bool can_black_castle_kingside = false;
    bool can_black_castle_queenside = false;
    Mask white_pieces_mask = empty_mask;
    Mask black_pieces_mask = empty_mask;
    Mask all_pieces_mask = empty_mask;
    Mask white_covered_squares_mask = empty_mask;
    Mask black_covered_squares_mask = empty_mask;
    Square white_king_square;
    Square black_king_square;
    Square en_passant_target_square = {0, 0}; // if no en passant is possible, conventionally the square is (0,0) = a8. This will never be a valid target square!
    bool is_check;
    int material_value = 0;
    int move_counter = 0;
    int half_move_counter = 0;
    int score = 0; // this is the score evaluated by the minmax algorithm 

    // constructors
    Position(std::string fen);

    // methods
    void PrintBoard(void);
    bool IsLegal(Move move);
    std::vector<Move> LegalMoves();

    // destructor
    ~Position();
};

// function that generates the new position from a given position
Position NewPosition(Position old_position, Move move);


class EfficientPosition
{
private:
public:
    std::vector<char> pieces;
    std::vector<int> ranks;
    std::vector<int> files;
    bool white_to_move;
    bool can_white_castle_kingside = false;
    bool can_white_castle_queenside = false;
    bool can_black_castle_kingside = false;
    bool can_black_castle_queenside = false;
    int half_move_counter = 0;
    int move_counter = 0;
    Square en_passant_target_square; // <--- can improve by storing just the file 
    int material_value = 0;
    int score = 0;
    Mask white_pieces_mask = empty_mask;
    Mask black_pieces_mask = empty_mask;
    Mask all_pieces_mask = empty_mask;
    Mask white_covered_squares_mask = empty_mask;
    Mask black_covered_squares_mask = empty_mask;

    // constructor
    EfficientPosition(std::string fen);

    // methods
    void PrintBoard(void);

    // destructor
    ~EfficientPosition();
};