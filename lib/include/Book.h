#pragma once
#include <Utilities.h>
#include <Move.h>
#include <Position.h>
#include <vector>
#include <string>

const std::string book_dir = "../assets/Book/";

const std::vector<std::string> openings = { 
    "CaroKann",
    "Italian",
    "QueensGambitDeclined",
    "Sicilian"
};

const int MAX_MEMORIZED_MOVES = 16; // how many good moves can the bot memorize at most for any given pos.

struct BookEntry {
    uint64_t hash; // footprint of a given pos as a 64bit integer
    int n_memorized_moves; // how many moves are actually stored in this item
    Move possible_moves[MAX_MEMORIZED_MOVES]; // memorized moves
    float weights[MAX_MEMORIZED_MOVES]; // probability that the corresponding move is played
};

const int BOOK_SIZE = 1 << 20;

extern BookEntry book[BOOK_SIZE];

// generate the new position obtained from the current position after applying a legal move expressed in SAN notation
// then return the move as a Move class
void ApplyMoveSAN(Position& pos, std::string move_SAN);
Move MoveFromSAN(Position pos, std::string move_SAN);

// initialize book
void BookInit();

// check if the given zobrist_key matches some entry in the transposition table
// and in case of success, return a pointer to that entry
BookEntry* BookProbe(uint64_t zobrist_key);

void BookStoreMove(uint64_t hash, Move move);

void FillBook();

