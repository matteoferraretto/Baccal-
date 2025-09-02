#pragma once
#include <cstdint>
#include <string>
#include <Utilities.h>
#include <Move.h>
#include <Position.h>
#include <SDL.h>
#include <SDL_image.h>


const int MAX_N_MOVES_IN_GAME = 256;

struct Piece {
    int type;
    SDL_Rect rect;
};


class Game{
    public:
        const int think_time = 2000; // milliseconds
        int n_moves = 0;
        Move moves_list[MAX_N_MOVES_IN_GAME] = { };
        Position positions_list[MAX_N_MOVES_IN_GAME];
        uint64_t zobrist_keys_list[MAX_N_MOVES_IN_GAME] = { };
        bool game_over;
        bool engine_is_white = false;
        Piece board[64];

        Game();
        ~Game();

        void InitGraphics(void);

        // reset all the attributes, show the board
        void StartNewGame(void);
        void StartNewGame(std::string position_fen);

        void DrawBoard(void);
        void DrawPieces(void);

        void ResetHistory(void);
        void ResetPseudoLegalMoves(void);

        bool DrawByRepetitions(void);

        void ReadGameFromPNG(std::string game_png);

        // ask the user to type a move, check if it's legal and add it to the stack
        void ReadMove(void);

        // transform the input string (given in square-square notation) into a Move
        Move MoveFromString(std::string move_str);

        void PlayVsEngine(void);

        bool GameOver();

        Move IterativeDeepening();

        void Clean(void);

    private: 
        Position pos; // current position
        Move pseudolegal_moves[MAX_NUMBER_OF_MOVES];
        Move player_move = 0, engine_move = 0;

        // graphics variables
        const int WIDTH  = 960; // width of screen
        const int HEIGHT = 960; // height of screen
        int SQUARE_SIZE = 110;
        int LEFT_PADDING = 40, TOP_PADDING = 40;
        bool is_running = true;
        SDL_Window* window;
        SDL_Renderer* renderer;
        SDL_Texture* board_texture;
        SDL_Texture* pieces_texture;
        SDL_Rect piece_clips[12]; // ritagli dalla sprite-sheet

        // input management 
        uint16_t SquareFromMouseClick(int x, int y);
        int moved_piece_index = NO_PIECE;
        uint16_t from = 64, to = 64;
        bool dragging = false;
        int mouse_x, mouse_y;
        void FindPiece(int square);
        void HandleEvent(SDL_Event event);
};