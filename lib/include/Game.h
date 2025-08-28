#pragma once
#include <cstdint>
#include <string>
#include <Utilities.h>
#include <Move.h>
#include <Position.h>

class Game{
    public:
        Position pos;
        int n_moves;
        Move move_list[256];
        Position game_history[256];
        bool white_wins = false;
        bool black_wins = false;
        bool is_draw = false;

        // reset all the attributes, show the board
        void StartNewGame(void);

        void ReadGameFromPNG(std::string game_png);

        // ask the user to type a move, check if it's legal and add it to the stack
        void ReadMove(void);

    private: 
    
}