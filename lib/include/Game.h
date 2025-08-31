#pragma once
#include <cstdint>
#include <string>
#include <Utilities.h>
#include <Move.h>
#include <Position.h>

const int MAX_N_MOVES_IN_GAME = 256;

class Game{
    public:
        int n_moves = 0;
        Move moves_list[MAX_N_MOVES_IN_GAME] = { };
        Position positions_list[MAX_N_MOVES_IN_GAME];
        bool white_wins = false;
        bool black_wins = false;
        bool is_draw = false;
        bool engine_is_white = false;

        Game();

        // reset all the attributes, show the board
        void StartNewGame(void);

        void ResetHistory(void);

        void ResetPseudoLegalMoves(void);

        void ReadGameFromPNG(std::string game_png);

        // ask the user to type a move, check if it's legal and add it to the stack
        void ReadMove(void);

        // transform the input string (given in square-square notation) into a Move
        Move MoveFromString(std::string move_str);

        void Play(void);

        bool GameOver();

    private: 
        Position pos; // current position
        Move pseudolegal_moves[MAX_NUMBER_OF_MOVES];
        const int think_time = 2000; // milliseconds
};