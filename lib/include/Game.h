#pragma once
#include <cstdint>
#include <string>
#include <sstream>
#include <fstream>
#include <Utilities.h>
#include <Move.h>
#include <Position.h>
#include <SDL.h>
#include <SDL_image.h>


const int MAX_N_MOVES_IN_GAME = 200; // max n. of half-moves to avoid risk of memory leaks

class Game{
    public:
        const int think_time = 1000; // milliseconds
        int n_moves = 0; // number of half-moves played in the game
        Move moves_list[MAX_N_MOVES_IN_GAME] = { }; // 0, ..., n_moves - 1, [garbage]  (n_moves non-garbage entries)
        Position positions_list[MAX_N_MOVES_IN_GAME]; // 0, ..., n_moves, [garbage]    (n_moves + 1 non-garbage entries)
        uint64_t zobrist_keys_list[MAX_N_MOVES_IN_GAME] = { };
        bool game_over = false, white_wins = false, black_wins = false, draw = false;
        bool engine_is_white = false;

        Game();
        ~Game();

        // game options
        void PresentGame(void);

        // graphics 
        void InitGraphics(void);
        void DrawBoard(void);
        void DrawPieces(void);
        void HighlightSquare(uint16_t square, int color[3]);
        void ChooseTheme(std::string theme);

        // reset
        void ResetHistory(void);
        void ResetPseudoLegalMoves(void);
        void Clean(void);

        // read user input
        void ReadMove(void);

        // gameplay 
        Move MoveFromString(std::string move_str);
        void PlayVsEngine(void);
        bool AskPromotion();
        Move IterativeDeepening();
        void FindPiece(int square);
        void HandleEvent(SDL_Event event);
        uint16_t SquareFromMouseClick(int x, int y);
        void EngineMove(void);
        void PlayerMove(void);
        void ShowGame(std::string pgn_file);

        // game over 
        bool DrawByRepetitions(void);
        bool GameOver();
        

    private: 
        Position pos; // current position
        Move pseudolegal_moves[MAX_NUMBER_OF_MOVES];
        Move player_move = 0, engine_move = 0;
        int idx_move_in_game = -1; 
        int idx_pos_in_game = 0; // 0: starting pos

        // graphics variables
        const int WIDTH  = 960; // width of screen
        const int HEIGHT = 960; // height of screen
        int SQUARE_SIZE = 110;
        int LEFT_PADDING = 40, TOP_PADDING = 40;
        int LIGHT_SQUARES[3] = {255, 200, 150};
        int DARK_SQUARES[3] = {200, 140, 68};
        bool is_running = true;
        SDL_Window* window;
        SDL_Renderer* renderer;
        SDL_Texture* board_texture;
        SDL_Texture* pieces_texture;
        SDL_Rect piece_clips[12]; // ritagli dalla sprite-sheet

        // input management 
        int moved_piece_index = NO_PIECE;
        uint16_t from = 64, to = 64;
        bool dragging = false;
        bool is_promo = false;
        int mouse_x, mouse_y;
};