#include <Game.h>
#include <Utilities.h>
#include <Baccala.h>
#include <TranspositionTable.h>
#include <iostream>
#include <string>
#include <chrono>


int YELLOW_RGB[3] = { 255, 255, 0 };
int RED_RGB[3] = { 255, 0, 0 };
int GREEN_RGB[3] = { 0, 255, 0 };


Game::Game(){
    InitGraphics();
}

Game::~Game() {
    Clean();
}


// game options
void Game::PresentGame(void) {
    std::string side;
    std::cout << "Choose your color!\n";
    std::getline( std::cin, side );
    // if human player types "b", they play with black and engine is white
    if(side == "b") engine_is_white = true;
    // if they type anything else -> engine is black by default.
    else engine_is_white = false;

    std::string normal_game;
    std::string position_fen;
    std::string theme;
    std::cout << "Play normal game [n]; Play a game from arbitrary starting position [a]\n";
    std::getline( std::cin, normal_game );
    std::cout << "Choose a theme\n";
    std::getline( std::cin, theme );
    ChooseTheme(theme);

    // reset stuff
    ResetHistory();
    ResetPseudoLegalMoves();
    PLY = 0;
    ResetRepetitionStack();

    while(true){
        if(normal_game == "n")
            position_fen = starting_position_fen;
        else{
            std::cout << "Insert FEN string of starting position\n";
            std::getline( std::cin, position_fen );
        }
        pos = PositionFromFen(position_fen);
        if(PositionIsPlayable(pos)){
            positions_list[0] = pos;
            break;
        }
        std::cout << "Invalid position.\n";
    }

    PrintBoard(pos, !engine_is_white);
    PseudoLegalMoves(pos, pseudolegal_moves);
    repetition_stack[0] = pos.zobrist_key;

    std::cout << "Let the battle begin!\n\n";
}


void Game::Menu(void){
    
    // 3. Create text surface
    SDL_Color WHITE = {255, 255, 255, 255};
    const char* text = "Choose your color";
    SDL_Surface* text_surface = TTF_RenderText_Blended(font, text, WHITE);
    if (!text_surface) {
        SDL_Log("Failed to create text surface: %s", TTF_GetError());
        TTF_CloseFont(font);
        return;
    }
    SDL_Texture* text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
    SDL_FreeSurface(text_surface);
    if (!text_texture) {
        SDL_Log("Failed to create text texture: %s", SDL_GetError());
        TTF_CloseFont(font);
        return;
    }

    // 5. Center the text
    SDL_Rect text_rect;
    text_rect.w = 200;
    text_rect.h = 200;
    text_rect.x = (WIDTH  - text_rect.w) / 2;
    text_rect.y = (HEIGHT - text_rect.h) / 2;

    // 6. Present
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_QueryTexture(text_texture, nullptr, nullptr, &text_rect.w, &text_rect.h);
    SDL_RenderCopy(renderer, text_texture, nullptr, &text_rect);

    SDL_Rect white_king_rect = { 400, 400, 100, 100 };
    SDL_Rect black_king_rect = { 500, 400, 100, 100 };
    SDL_Rect settings_rect = { WIDTH - 110, 10, 80, 80 };
    SDL_RenderCopy(renderer, pieces_texture, &piece_clips[WHITE_KING], &white_king_rect);
    SDL_RenderCopy(renderer, pieces_texture, &piece_clips[BLACK_KING], &black_king_rect);
    SDL_RenderCopy(renderer, settings_texture, nullptr, &settings_rect);
    SDL_RenderPresent(renderer);

    SDL_Event event;
    bool show_menu = true;
    while (show_menu) {
        while (SDL_PollEvent(&event)) {
            // Quit the game
            if (event.type == SDL_QUIT) {
                Clean();
                return;
            }
            // Choose a color
            else if(event.type == SDL_MOUSEBUTTONDOWN){
                if (event.button.button == SDL_BUTTON_LEFT) {
                    SDL_Point mouse = { event.button.x , event.button.y };
                    if (SDL_PointInRect(&mouse, &white_king_rect)){
                        engine_is_white = false;
                        show_menu = false;
                        break;
                    }
                    else if (SDL_PointInRect(&mouse, &black_king_rect)){
                        engine_is_white = true;
                        show_menu = false;
                        break;
                    }
                    else if (SDL_PointInRect(&mouse, &settings_rect)) {
                        SettingsMenu();
                        return;
                    }
                }
            }
        }
        SDL_Delay(50);
    }

    PlayVsEngine();

    Clean();
}


void Game::SettingsMenu(void) {
    const char* text_theme = "Theme: ";
    SDL_Surface* text_theme_surface = TTF_RenderText_Blended(font, text_theme, /* white */{255,255,255,255});
    SDL_Texture* text_theme_texture = SDL_CreateTextureFromSurface(renderer, text_theme_surface);
    SDL_FreeSurface(text_theme_surface);
    int chosen_theme = 0;

    const char* text_askFEN = "Starting pos. FEN: ";
    SDL_Surface* text_askFEN_surface = TTF_RenderText_Blended(font, text_askFEN, /* white */{255,255,255,255});
    SDL_Texture* text_askFEN_texture = SDL_CreateTextureFromSurface(renderer, text_askFEN_surface);
    SDL_FreeSurface(text_askFEN_surface);

    const char* text_askTime = "Think for [ms]: ";
    SDL_Surface* text_askTime_surface = TTF_RenderText_Blended(font, text_askTime, /* white */{255,255,255,255});
    SDL_Texture* text_askTime_texture = SDL_CreateTextureFromSurface(renderer, text_askTime_surface);
    SDL_FreeSurface(text_askTime_surface);

    const char* text_askShow = "Path to PGN: ";
    SDL_Surface* text_askShow_surface = TTF_RenderText_Blended(font, text_askShow, /* white */{255,255,255,255});
    SDL_Texture* text_askShow_texture = SDL_CreateTextureFromSurface(renderer, text_askShow_surface);
    SDL_FreeSurface(text_askShow_surface);
    
    // Initialize rectangles
    SDL_Rect square_rect;
    SDL_Rect text_theme_rect = { 50, 200, 200, 200 };
    SDL_Rect back_arrow_rect = { 50, 50, 60, 60 };
    SDL_Rect classic_theme_rect = { 300, 200, 40, 40 };
    SDL_Rect sea_theme_rect = { 400, 200, 40, 40 };
    SDL_Rect pink_theme_rect = { 500, 200, 40, 40 };
    SDL_Rect green_theme_rect = { 600, 200, 40, 40 };
    SDL_Rect askFEN_rect = { 50, 300, 200, 200 };
    SDL_Rect input_fen_rect = { 250, 300, 400, 40 };
    SDL_Rect askTime_rect = { 50, 400, 200, 200 };
    SDL_Rect input_time_rect = { 250, 400, 400, 40 };
    SDL_Rect askShow_rect = { 50, 500, 200, 200 };
    SDL_Rect input_show_rect = { 250, 500, 400, 40 };

    // input starting position FEN 
    bool enable_input_fen = false;
    std::string input_fen;
    SDL_Surface* input_fen_surface;
    SDL_Texture* input_fen_texture;

    // input thinking time
    bool enable_input_time = false;
    std::string input_time;
    SDL_Surface* input_time_surface;
    SDL_Texture* input_time_texture;

    // input path to PGN file to be shown
    bool enable_input_show = false;
    std::string input_show;
    SDL_Surface* input_show_surface;
    SDL_Texture* input_show_texture;

    SDL_Event event;
    bool show_menu = true;

    while (show_menu) {
        SDL_RenderClear(renderer);

        // Draw "Theme", "Ask FEN" and the "go back" arrow
        SDL_QueryTexture(text_theme_texture, nullptr, nullptr, &text_theme_rect.w, &text_theme_rect.h);
        SDL_RenderCopy(renderer, text_theme_texture, nullptr, &text_theme_rect);
        SDL_QueryTexture(text_askFEN_texture, nullptr, nullptr, &askFEN_rect.w, &askFEN_rect.h);
        SDL_RenderCopy(renderer, text_askFEN_texture, nullptr, &askFEN_rect);
        SDL_QueryTexture(text_askTime_texture, nullptr, nullptr, &askTime_rect.w, &askTime_rect.h);
        SDL_RenderCopy(renderer, text_askTime_texture, nullptr, &askTime_rect);
        SDL_QueryTexture(text_askShow_texture, nullptr, nullptr, &askShow_rect.w, &askShow_rect.h);
        SDL_RenderCopy(renderer, text_askShow_texture, nullptr, &askShow_rect);
        SDL_RenderCopy(renderer, back_arrow_texture, nullptr, &back_arrow_rect);
        SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
        SDL_RenderFillRect(renderer, &input_fen_rect);
        SDL_RenderFillRect(renderer, &input_time_rect);
        SDL_RenderFillRect(renderer, &input_show_rect);       

        // Draw the colored squares representing themes
        for(int theme = 0; theme < 4; theme++){
            ChooseTheme(theme);
            for(int i = 0; i < 2; i++){
                for(int j = 0; j < 2; j++){
                    square_rect = { 300 + theme*100 + j*20, 200 + i*20, 20, 20 };
                    if((i+j)%2 == 0) SDL_SetRenderDrawColor(renderer, LIGHT_SQUARES[0], LIGHT_SQUARES[1], LIGHT_SQUARES[2], 255);
                    else SDL_SetRenderDrawColor(renderer, DARK_SQUARES[0], DARK_SQUARES[1], DARK_SQUARES[2], 255);
                    SDL_RenderFillRect(renderer, &square_rect);
                }
            }
        }
        ChooseTheme(chosen_theme);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

        // Draw user's input text
        input_fen_surface = TTF_RenderText_Blended(font, input_fen.c_str(), /* white */{255,255,255,255});
        input_fen_texture = SDL_CreateTextureFromSurface(renderer, input_fen_surface);
        SDL_FreeSurface(input_fen_surface);
        SDL_QueryTexture(input_fen_texture, nullptr, nullptr, &input_fen_rect.w, &input_fen_rect.h);
        SDL_RenderCopy(renderer, input_fen_texture, nullptr, &input_fen_rect);

        input_time_surface = TTF_RenderText_Blended(font, input_time.c_str(), /* white */{255,255,255,255});
        input_time_texture = SDL_CreateTextureFromSurface(renderer, input_time_surface);
        SDL_FreeSurface(input_time_surface);
        SDL_QueryTexture(input_time_texture, nullptr, nullptr, &input_time_rect.w, &input_time_rect.h);
        SDL_RenderCopy(renderer, input_time_texture, nullptr, &input_time_rect);

        input_show_surface = TTF_RenderText_Blended(font, input_show.c_str(), /* white */{255,255,255,255});
        input_show_texture = SDL_CreateTextureFromSurface(renderer, input_show_surface);
        SDL_FreeSurface(input_show_surface);
        SDL_QueryTexture(input_show_texture, nullptr, nullptr, &input_show_rect.w, &input_show_rect.h);
        SDL_RenderCopy(renderer, input_show_texture, nullptr, &input_show_rect);

        SDL_RenderPresent(renderer);

        SDL_StartTextInput();

        // Handle events
        while (SDL_PollEvent(&event)) {
            // Quit the game
            if (event.type == SDL_QUIT) {
                SDL_DestroyTexture(text_theme_texture);
                SDL_DestroyTexture(text_askFEN_texture);
                SDL_DestroyTexture(input_fen_texture);
                Clean(); return;
            }
            // Mouse click
            else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                SDL_Point mouse = { event.button.x , event.button.y };
                // back to main menu
                if (SDL_PointInRect(&mouse, &back_arrow_rect)){
                    Menu(); return;
                }
                // choose theme
                else if (SDL_PointInRect(&mouse, &classic_theme_rect)) chosen_theme = 0;
                else if (SDL_PointInRect(&mouse, &sea_theme_rect)) chosen_theme = 1;
                else if (SDL_PointInRect(&mouse, &pink_theme_rect)) chosen_theme = 2;
                else if (SDL_PointInRect(&mouse, &green_theme_rect)) chosen_theme = 3;
                // input text
                else if (SDL_PointInRect(&mouse, &input_fen_rect)){
                    enable_input_fen = true; enable_input_time = false; enable_input_show = false;
                }
                else if (SDL_PointInRect(&mouse, &input_time_rect)){
                    enable_input_fen = false; enable_input_time = true; enable_input_show = false;
                }
                else if (SDL_PointInRect(&mouse, &input_show_rect)){
                    enable_input_fen = false; enable_input_time = false; enable_input_show = true;
                }
            }
            // Input FEN string, time etc.
            else if (enable_input_fen && event.type == SDL_TEXTINPUT) input_fen += event.text.text;
            else if (enable_input_time && event.type == SDL_TEXTINPUT) input_time += event.text.text;
            else if (enable_input_show && event.type == SDL_TEXTINPUT) input_show += event.text.text;
            // 
            if(enable_input_fen && event.type == SDL_KEYDOWN){
                if (event.key.keysym.sym == SDLK_BACKSPACE && !input_fen.empty()) input_fen.pop_back();
                else if (event.key.keysym.sym == SDLK_RETURN) {
                    initial_pos_fen = input_fen;
                    input_fen = "";
                    SDL_StopTextInput();
                }
                // Use ctrl + v to paste a FEN string
                else if ( (event.key.keysym.mod & KMOD_CTRL) && event.key.keysym.sym == SDLK_v){
                    char* clipboard = SDL_GetClipboardText();
                    if (clipboard){
                        input_fen += clipboard;
                        SDL_free(clipboard);
                    }
                }
            }
            if(enable_input_time && event.type == SDL_KEYDOWN){
                if (event.key.keysym.sym == SDLK_BACKSPACE && !input_time.empty()) input_time.pop_back();
                else if (event.key.keysym.sym == SDLK_RETURN) {
                    think_time = std::stoi( input_time );
                    input_time = "";
                    SDL_StopTextInput();
                }
            }
            if(enable_input_show && event.type == SDL_KEYDOWN){
                if (event.key.keysym.sym == SDLK_BACKSPACE && !input_show.empty()) input_show.pop_back();
                else if (event.key.keysym.sym == SDLK_RETURN) {
                    SDL_StopTextInput();
                    ShowGame(input_show);
                }
                // Use ctrl + v to paste a FEN string
                else if ( (event.key.keysym.mod & KMOD_CTRL) && event.key.keysym.sym == SDLK_v){
                    char* clipboard = SDL_GetClipboardText();
                    if (clipboard){
                        input_show += clipboard;
                        SDL_free(clipboard);
                    }
                }
            }
        }
    }
}


void Game::InitGraphics(){
    // initialize SDL, images and fonts
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init error: " << SDL_GetError() << std::endl;
        return;
    }
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        std::cerr << "SDL_image Init Error: " << IMG_GetError() << std::endl;
        return;
    }
    if (TTF_Init() == -1) {
        SDL_Log("TTF_Init failed: %s", TTF_GetError());
        return;
    }
    // load font
    font = TTF_OpenFont("../assets/Roboto-Regular.ttf", 24);
    if (!font) {
        SDL_Log("Failed to load font: %s", TTF_GetError());
        return;
    }

    // create window and renderer
    window = SDL_CreateWindow(
        "Chess GUI - demo",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIDTH, HEIGHT,
        SDL_WINDOW_SHOWN
    );
    if (!window) {
        std::cerr << "CreateWindow error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return;
    }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    // Carica la sprite-sheet dei pezzi
    SDL_Surface* temp_surface = IMG_Load("../assets/Chess_Pieces_Sprite.png");
    if (!temp_surface) {
        std::cerr << "IMG_Load Error: " << IMG_GetError() << std::endl;
        Clean();
        return;
    }
    pieces_texture = SDL_CreateTextureFromSurface(renderer, temp_surface);
    SDL_FreeSurface(temp_surface);
    if (!pieces_texture) {
        std::cerr << "CreateTextureFromSurface Error: " << SDL_GetError() << std::endl;
        Clean();
        return;
    }
    // Supponiamo immagine 2 righe x 6 colonne
    int w = 133; // <- cambia con la larghezza reale di ogni pezzo
    int h = 133; // <- cambia con l’altezza reale
    piece_clips[WHITE_KING] = { 0, 0, w, h };
    piece_clips[WHITE_QUEEN] = { w, 0, w, h };
    piece_clips[WHITE_BISHOP] = { 2*w, 0, w, h };
    piece_clips[WHITE_KNIGHT] = { 3*w, 0, w, h };
    piece_clips[WHITE_ROOK] = { 4*w, 0, w, h };
    piece_clips[WHITE_PAWN] = { 5*w, 0, w, h };
    piece_clips[BLACK_KING] = { 0, h, w, h };
    piece_clips[BLACK_QUEEN] = { w, h, w, h };
    piece_clips[BLACK_BISHOP] = { 2*w, h, w, h };
    piece_clips[BLACK_KNIGHT] = { 3*w, h, w, h };
    piece_clips[BLACK_ROOK] = { 4*w, h, w, h };
    piece_clips[BLACK_PAWN] = { 5*w, h, w, h };

    // Load settings symbol
    // Carica la sprite-sheet dei pezzi
    SDL_Surface* settings_surface = IMG_Load("../assets/settings.png");
    if (!settings_surface) {
        std::cerr << "IMG_Load Error: " << IMG_GetError() << std::endl;
        Clean();
        return;
    }
    settings_texture = SDL_CreateTextureFromSurface(renderer, settings_surface);
    SDL_FreeSurface(settings_surface);
    if (!settings_texture) {
        std::cerr << "CreateTextureFromSurface Error: " << SDL_GetError() << std::endl;
        Clean();
        return;
    }

    // Load back arrow symbol
    SDL_Surface* back_arrow_surface = IMG_Load("../assets/back_arrow.png");
    if (!back_arrow_surface) {
        std::cerr << "IMG_Load Error: " << IMG_GetError() << std::endl;
        Clean();
        return;
    }
    back_arrow_texture = SDL_CreateTextureFromSurface(renderer, back_arrow_surface);
    SDL_FreeSurface(back_arrow_surface);
    if (!back_arrow_texture) {
        std::cerr << "CreateTextureFromSurface Error: " << SDL_GetError() << std::endl;
        Clean();
        return;
    }
}


void Game::DrawBoard(){
    SDL_RenderClear(renderer);

    board_texture = SDL_CreateTexture(
        renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, WIDTH, HEIGHT
    );

    SDL_SetRenderTarget(renderer, board_texture);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

    // quadrato colorato (per test)
    int i, j;
    SDL_Rect rect[64];
    for(int square = 0; square < 64; square++){
        i = square / 8; j = square % 8;
        rect[square].x = LEFT_PADDING + SQUARE_SIZE * j;
        rect[square].y = TOP_PADDING + SQUARE_SIZE * i; 
        rect[square].w = SQUARE_SIZE;
        rect[square].h = SQUARE_SIZE;

        if((i+j) % 2 == 0) 
            SDL_SetRenderDrawColor(renderer, LIGHT_SQUARES[0], LIGHT_SQUARES[1], LIGHT_SQUARES[2], 255);
        else 
            SDL_SetRenderDrawColor(renderer, DARK_SQUARES[0], DARK_SQUARES[1], DARK_SQUARES[2], 255);

        SDL_RenderFillRect(renderer, &rect[square]);
    }

    SDL_SetRenderTarget(renderer, NULL);

}


void Game::DrawPieces() {
    SDL_Rect dest;
    Bitboard piece;
    unsigned long square;
    int x, y;

    for(int idx = 0; idx < 12; idx++){
        piece = pos.pieces[idx];
        while (piece){
            _BitScanForward64(&square, piece);
            // during dragging, do not print the piece
            if(dragging && idx == moved_piece_index && static_cast<uint16_t>(square) == from){
                clear_last_active_bit(piece);
                continue;
            }
            y = engine_is_white ? 7 - (square / 8) : square / 8; 
            x = engine_is_white ? 7 - (square % 8) : square % 8;
            dest = { LEFT_PADDING + x * SQUARE_SIZE, TOP_PADDING + y * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE };
            SDL_RenderCopy(renderer, pieces_texture, &piece_clips[idx], &dest);
            clear_last_active_bit(piece);
        }
    }

}


void Game::HighlightSquare(uint16_t square, int color[3]) {
    uint16_t i = square / 8;
    uint16_t j = square % 8;

    if(i < 0 || i > 7 || j < 0 || j > 7) return;

    if(engine_is_white){
        i = 7 - i; j = 7 - j;
    }

    SDL_Rect rect = {LEFT_PADDING + j * SQUARE_SIZE, TOP_PADDING + i * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE};
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, color[0], color[1], color[2], 128); // translucent yellow
    SDL_RenderFillRect(renderer, &rect);
}


void Game::EngineMove() {
    StateMemory state;
    // engine's turn
    if(pos.white_to_move == engine_is_white){
        // probe book or think
        BookEntry* entry = BookProbe(pos.zobrist_key);
        if(entry != nullptr){
            int r = random_int((*entry).n_memorized_moves);
            engine_move = (*entry).possible_moves[r];
        } 
        else engine_move = IterativeDeepening();
        // make the move
        MakeMove(pos, engine_move, state);
        // safety check: is move legal?
        if(engine_move == 0 || !IsLegal(pos, engine_move)){
            UnmakeMove(pos, engine_move, state);
            return;
        }
        // if move is legal, recompute pseudolegals and update the stack
        ResetPseudoLegalMoves();
        PseudoLegalMoves(pos, pseudolegal_moves);
        n_moves++;
        idx_move_in_game++;
        idx_pos_in_game++;
        moves_list[idx_move_in_game] = engine_move;
        positions_list[n_moves] = pos;
        repetition_stack[n_moves] = pos.zobrist_key;
        from = engine_move & 0b0000000000111111;
        to = (engine_move >> 6) & 0b0000000000111111;
        // print stuff
        if(!pos.white_to_move) std::cout << 1 + n_moves/2 << ". ";
        std::cout << AlgebraicNotation(positions_list[n_moves-1], engine_move) << " ";
    }
}


void Game::PlayerMove() {
    SDL_Event event;
    uint16_t mask = 0b0000111111111111;

    // player move
    while (SDL_PollEvent(&event)) {
        // Quit the game
        if (event.type == SDL_QUIT) {
            is_running = false;
            Clean();
        }
        // Pick up a piece
        else if(event.type == SDL_MOUSEBUTTONDOWN){
            if (event.button.button == SDL_BUTTON_LEFT) {
                from = SquareFromMouseClick(event.button.x, event.button.y);
                // check if a piece is on that square
                FindPiece(from);
            }
        }
        // release piece
        else if(event.type == SDL_MOUSEBUTTONUP){
            if (event.button.button == SDL_BUTTON_LEFT && dragging) {
                dragging = false;
                // generate moves
                to = SquareFromMouseClick(event.button.x, event.button.y);
                player_move = from | (to << 6);
                // if promotion, freeze the screen and ask for promoted piece
                is_promo = AskPromotion(); // break the event loop
                if(is_promo) break;

                // compare with pseudolegal moves, and if there's matching, make the move and check legality
                for(int idx = 0; idx < MAX_NUMBER_OF_MOVES; idx++){
                    // if we have finished the pseudolegal moves, break
                    if(pseudolegal_moves[idx] == 0) break;
                    // if the input move matches one of the precomputed pseudolegal moves
                    if((pseudolegal_moves[idx] & mask) == player_move){
                        player_move = pseudolegal_moves[idx];
                        StateMemory state;
                        MakeMove(pos, player_move, state);
                        if(!IsLegal(pos, player_move)) 
                            UnmakeMove(pos, player_move, state);
                        else{ // move is legal
                            n_moves++;
                            idx_move_in_game++; idx_pos_in_game++;
                            moves_list[idx_move_in_game] = player_move;
                            positions_list[n_moves] = pos;
                            repetition_stack[n_moves] = pos.zobrist_key;
                            ResetPseudoLegalMoves();
                            PseudoLegalMoves(pos, pseudolegal_moves);
                            // print stuff
                            if(!pos.white_to_move) std::cout << 1 + n_moves/2 << ". ";
                            std::cout << AlgebraicNotation(positions_list[n_moves-1], player_move) << " ";
                        }
                    }
                }
            }
        }
        // drag piece
        else if (event.type == SDL_MOUSEMOTION) {
            mouse_x = event.motion.x;
            mouse_y = event.motion.y;
        }
        // visualize game history
        else if(event.type == SDL_KEYDOWN){
            // go forward (right arrow)
            if (event.key.keysym.sym == SDLK_RIGHT) {
                if(idx_pos_in_game < n_moves && idx_pos_in_game >= 0){
                    idx_move_in_game++;
                    from = moves_list[idx_move_in_game] & 0b0000000000111111;
                    to = (moves_list[idx_move_in_game] >> 6) & 0b0000000000111111;
                    idx_pos_in_game++;
                    pos = positions_list[idx_pos_in_game];
                    pos.white_to_move = !engine_is_white; // trick to ensure that the engine does not play a move while visualizing game history
                }
            }
            // go backwards (left arrow)
            else if(event.key.keysym.sym == SDLK_LEFT) { 
                if(idx_pos_in_game > 0 && idx_pos_in_game <= n_moves){        
                    from = moves_list[idx_move_in_game] & 0b0000000000111111;
                    to = (moves_list[idx_move_in_game] >> 6) & 0b0000000000111111;
                    idx_move_in_game--;
                    idx_pos_in_game--;
                    pos = positions_list[idx_pos_in_game];
                    pos.white_to_move = !engine_is_white; // trick to ensure that the engine does not play a move while visualizing game history
                }
            }
            // go to starting pos (down arrow)
            else if(event.key.keysym.sym == SDLK_DOWN) {
                idx_move_in_game = -1;
                idx_pos_in_game = 0;
                from = 64; to = 64;
                pos = positions_list[0];
                pos.white_to_move = !engine_is_white; // trick to ensure that the engine does not play a move while visualizing game history
            }
            // go to the current pos (up arrow)
            else if(event.key.keysym.sym == SDLK_UP) {
                idx_move_in_game = n_moves - 1;
                idx_pos_in_game = n_moves;
                from = moves_list[n_moves - 1] & 0b0000000000111111;
                to = (moves_list[n_moves - 1] >> 6) & 0b0000000000111111;
                pos = positions_list[n_moves];
            }
            // Retire the last move
            else if(event.key.keysym.sym == SDLK_r) {
                if(idx_pos_in_game > 1 && idx_pos_in_game <= n_moves && idx_move_in_game >= 1){        
                    from = moves_list[idx_move_in_game-1] & 0b0000000000111111;
                    to = (moves_list[idx_move_in_game-1] >> 6) & 0b0000000000111111;
                    idx_move_in_game -= 2;
                    idx_pos_in_game -= 2;
                    n_moves -= 2;
                    pos = positions_list[idx_pos_in_game];
                    ResetPseudoLegalMoves();
                    PseudoLegalMoves(pos, pseudolegal_moves);
                    continue;
                }
            }
        }
    }

}


void Game::ResetHistory(void){
    n_moves = 0; PLY = 0;
    Position empty_pos;
    for(int idx = 0; idx < MAX_N_MOVES_IN_GAME; idx++){
        moves_list[idx] = 0;
        positions_list[idx] = empty_pos;
    }
    game_over = false;
}


void Game::ResetPseudoLegalMoves(void){
    for(int i = 0; i < MAX_NUMBER_OF_MOVES; i++){
        pseudolegal_moves[i] = 0;
    }
}


bool Game::DrawByRepetitions(void){
    if(n_moves < 6) return false;

    int count = 0;
    // steps of 2 because a repetition can be found only if the side to move is the current side to move 
    for(int i = 2; i <= n_moves; i += 2){
        if(repetition_stack[n_moves - i] == pos.zobrist_key)
            count++;
        if(count == 2) return true;
    }
    return false;
}


Move Game::MoveFromString(std::string move_str){
    if(move_str.size() < 4 || move_str.size() > 5){
        std::cout << "Invalid move. Returning 0\n";
        return 0;
    }

    std::string from_str = move_str.substr(0, 2);
    std::string to_str = move_str.substr(2, 2);
    Bitboard from_bb = AlphabetToBitboard(from_str);
    Bitboard to_bb = AlphabetToBitboard(to_str);

    unsigned long from_square, to_square;
    _BitScanForward64(&from_square, from_bb);
    Move move = static_cast<uint16_t>(from_square);
    _BitScanForward64(&to_square, to_bb);
    move |= static_cast<uint16_t>(to_square << 6);

    StateMemory state;

    // in case of pawn promotion, we build the flag by hand
    if(move_str.size() == 5){
        // avoid non-sense
        if(pos.white_to_move && (RANK_OF_SQUARE[from_square] != 1 || RANK_OF_SQUARE[to_square] != 0))
            return 0;
        if(!pos.white_to_move && (RANK_OF_SQUARE[from_square] != 6 || RANK_OF_SQUARE[to_square] != 7))
            return 0;
        int delta_x = FILE_OF_SQUARE[to_square] - FILE_OF_SQUARE[from_square];
        if(delta_x > 1 || delta_x < -1)
            return 0;

        std::string promo = move_str.substr(4, 1);
        if(promo == "Q"){
            if(delta_x == 0) move |= static_cast<uint16_t>(PROMOTION_QUEEN << 12);
            else move |= static_cast<uint16_t>(PROMOTION_QUEEN_CAPTURE << 12);
        }
        else if(promo == "R"){
            if(delta_x == 0) move |= static_cast<uint16_t>(PROMOTION_ROOK << 12);
            else move |= static_cast<uint16_t>(PROMOTION_ROOK_CAPTURE << 12);
        }
        else if(promo == "B"){
            if(delta_x == 0) move |= static_cast<uint16_t>(PROMOTION_BISHOP << 12);
            else move |= static_cast<uint16_t>(PROMOTION_BISHOP_CAPTURE << 12);
        }
        else if(promo == "N"){
            if(delta_x == 0) move |= static_cast<uint16_t>(PROMOTION_KNIGHT << 12);
            else move |= static_cast<uint16_t>(PROMOTION_KNIGHT_CAPTURE << 12);
        }

        MakeMove(pos, move, state);
        if(IsLegal(pos, move)) return move;
        else UnmakeMove(pos, move, state);
    }

    // for any other move, we just check if "from" and "to" match some pseudolegal move
    uint16_t mask = 0b0000111111111111;

    for(int idx = 0; idx < MAX_NUMBER_OF_MOVES; idx++){
        // if we have finished the pseudolegal moves, break
        if(pseudolegal_moves[idx] == 0) break;
        // if the input move matches one of the precomputed pseudolegal moves
        if((pseudolegal_moves[idx] & mask) == move){
            move = pseudolegal_moves[idx];
            MakeMove(pos, move, state);
            if(IsLegal(pos, move)) return move;
            else UnmakeMove(pos, move, state);
        }
    }

    return 0;
}


void Game::ReadMove(void){
    Move move = 0;
    while(move == 0){
        std::cout << "Type your move \n";
        std::string move_str;
        std::getline( std::cin, move_str );
        move = MoveFromString(move_str);
    }
}


void Game::PlayVsEngine(void){
    /*Move engine_move;
    StateMemory state;
    bool game_over = false;
    while(true){
        // engine move
        if(pos.white_to_move == engine_is_white){
            engine_move = IterativeDeepening();
            MakeMove(pos, engine_move, state);
            PrintMove(engine_move); std::cout << "\n";
            PrintBoard(pos, !engine_is_white);
            ResetPseudoLegalMoves();
            PseudoLegalMoves(pos, pseudolegal_moves);
        }
        // player's move
        else{
            ReadMove();
            PrintBoard(pos, !engine_is_white);
            ResetPseudoLegalMoves();
            PseudoLegalMoves(pos, pseudolegal_moves);
        }
        // check game over
        n_moves++;
        repetition_stack[n_moves] = pos.zobrist_key;
        game_over = GameOver();
        if(game_over) break;
    }*/
    // PresentGame();

    // InitGraphics();

    // reset stuff
    ResetHistory();
    ResetPseudoLegalMoves();
    PLY = 0;
    ResetRepetitionStack();
    // prepare position
    pos = PositionFromFen(initial_pos_fen);
    PseudoLegalMoves(pos, pseudolegal_moves);
    positions_list[0] = pos;
    repetition_stack[0] = pos.zobrist_key;

    DrawBoard();
    
    SDL_RenderCopy(renderer, board_texture, NULL, NULL);
    SDL_RenderPresent(renderer);

    while (is_running) {

        EngineMove();

        PlayerMove();
        
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, board_texture, NULL, NULL);

        if(from < 64)
            HighlightSquare(from, YELLOW_RGB); 
        if(to < 64)
            HighlightSquare(to, YELLOW_RGB);
        DrawPieces();

        if(dragging && moved_piece_index != NO_PIECE){
            SDL_Rect dest = { mouse_x - SQUARE_SIZE/2, mouse_y - SQUARE_SIZE/2, SQUARE_SIZE, SQUARE_SIZE };
            SDL_RenderCopy(renderer, pieces_texture, &piece_clips[moved_piece_index], &dest);
        }

        SDL_RenderPresent(renderer);
        
        // check game over
        game_over = GameOver();
        if(game_over){
            SDL_Event e;
            bool wait = true;
            while (wait) {
                while (SDL_PollEvent(&e)) {
                    if (e.type == SDL_QUIT) {
                        wait = false;
                        is_running = false;
                        Clean();
                    }
                }

                SDL_RenderClear(renderer);
                SDL_RenderCopy(renderer, board_texture, NULL, NULL);

                unsigned long white_king_sq, black_king_sq;
                _BitScanForward64(&white_king_sq, pos.pieces[WHITE_KING]);
                _BitScanForward64(&black_king_sq, pos.pieces[BLACK_KING]);
                if(white_wins){
                    HighlightSquare(static_cast<uint16_t> (white_king_sq), GREEN_RGB);
                    HighlightSquare(static_cast<uint16_t> (black_king_sq), RED_RGB);
                }
                else if(black_wins){
                    HighlightSquare(static_cast<uint16_t> (white_king_sq), RED_RGB);
                    HighlightSquare(static_cast<uint16_t> (black_king_sq), GREEN_RGB);
                }
                else {
                    HighlightSquare(static_cast<uint16_t> (white_king_sq), YELLOW_RGB);
                    HighlightSquare(static_cast<uint16_t> (black_king_sq), YELLOW_RGB);
                }

                DrawPieces();
                SDL_RenderPresent(renderer);

                SDL_Delay(50); // avoid burning CPU
            }
        }

        SDL_Delay(16); // ~60 fps
    }
}


bool Game::AskPromotion(){
    
    uint16_t i = to / 8, j = to % 8;

    if(moved_piece_index != WHITE_PAWN && moved_piece_index != BLACK_PAWN)
        return false;
    else if(moved_piece_index == WHITE_PAWN && i != 0)
        return false;
    else if(moved_piece_index == BLACK_PAWN && i != 7)
        return false;

    SDL_Rect options[4];
    int promoted_piece_indexes[4] = { NO_PIECE, NO_PIECE, NO_PIECE, NO_PIECE };

    // if white is promoting
    if(moved_piece_index == WHITE_PAWN && i == 0){
        for(int idx = 0; idx < 4; idx++){
            options[idx] = {
                LEFT_PADDING + j * SQUARE_SIZE, 
                TOP_PADDING + idx * SQUARE_SIZE,
                SQUARE_SIZE,
                SQUARE_SIZE
            };
            promoted_piece_indexes[idx] = 1 + idx; 
        }
    }

    // if black is promoting
    else if(moved_piece_index == BLACK_PAWN && i == 7){
        for(int idx = 0; idx < 4; idx++){
            options[idx] = {
                LEFT_PADDING + (7 - j) * SQUARE_SIZE, 
                TOP_PADDING + idx * SQUARE_SIZE,
                SQUARE_SIZE,
                SQUARE_SIZE
            };
            promoted_piece_indexes[idx] = 7 + idx; 
        }
    }
    // don't deal with other types of move in this function
    else return false;

    // before asking for promotion, check legality
    // if illegal -> return to main loop
    StateMemory state;
    uint16_t flags;
    Move move;

    // compare with pseudolegal moves, and if there's matching, make the move and check legality
    uint16_t mask = 0b0000111111111111;
    for(int idx = 0; idx < MAX_NUMBER_OF_MOVES; idx++){
        // if we have finished the pseudolegal moves, break
        move = pseudolegal_moves[idx];
        if(move == 0) return false;
        // if the input move matches one of the precomputed pseudolegal moves
        if((move & mask) == player_move){
            MakeMove(pos, move, state);
            if(!IsLegal(pos, move)) {
                UnmakeMove(pos, move, state);
                return false;
            }
            UnmakeMove(pos, move, state);
            break;
        } 
    }
                        
    // If we are really promoting,
    // Freeze the board during promotion 
    bool wait = true; 
    while (wait) {
        // render board + highlight options
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, board_texture, NULL, NULL);
        DrawPieces();
        for(int idx = 0; idx < 4; idx++){
            SDL_RenderCopy(renderer, pieces_texture, &piece_clips[promoted_piece_indexes[idx]], &options[idx]);
        }
        SDL_RenderPresent(renderer);

        // manage events
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            // quit
            if (e.type == SDL_QUIT) {
                is_running = false; 
                wait = false; 
                return false;
            }
            // left click -> choose piece
            else if (e.type == SDL_MOUSEBUTTONDOWN) {
                int mx = e.button.x, my = e.button.y; // mouse click coordinates
                // check where the player clicked
                for (uint16_t idx = 0; idx < 4; idx++) {
                    if (mx >= options[idx].x && mx < options[idx].x + options[idx].w &&
                        my >= options[idx].y && my < options[idx].y + options[idx].h) {
                        // flag is different if it is simple promo or promo with capture
                        //   - normal promotion
                        if(from == to + 8 || from == to - 8) flags = 11 - idx;
                        //   - promotion with capture 
                        else flags = 15 - idx;
                        // update move and make it (we ensured that this is legal)
                        player_move |= static_cast<uint16_t>(flags << 12);
                        MakeMove(pos, player_move, state);
                        n_moves++;
                        idx_move_in_game++;
                        idx_pos_in_game++;
                        moves_list[idx_move_in_game] = player_move;
                        positions_list[n_moves] = pos;
                        repetition_stack[n_moves] = pos.zobrist_key;
                        ResetPseudoLegalMoves();
                        PseudoLegalMoves(pos, pseudolegal_moves);
                        // print stuff
                        if(!pos.white_to_move) std::cout << 1 + n_moves/2 << ". ";
                        std::cout << AlgebraicNotation(positions_list[n_moves-1], player_move) << " ";
                        // un-freeze the board
                        wait = false;
                    }
                }
            }
        }
    }
    return true;
}


bool Game::GameOver(){
    int n_legal_moves = 0;
    StateMemory state;
    Move move = 0;
    // count the legal moves
    for(int i = 0; i < MAX_NUMBER_OF_MOVES; i++){
        move = pseudolegal_moves[i];
        if(move == 0) break;
        MakeMove(pos, move, state);
        if(IsLegal(pos, move)) n_legal_moves++;
        UnmakeMove(pos, move, state);
    }
    if(n_legal_moves == 0){
        // checkmate
        if(InCheck(pos)){
            if(pos.white_to_move){
                std::cout << "Checkmate. Black wins.\n";
                black_wins = true;
            }
            else{
                std::cout << "Checkmate. White wins.\n";
                white_wins = true;
            }
        }
        // otherwise it's stalemate
        else{
            std::cout << "Draw by stalemate.\n";
            draw = true;
        }
        return true;
    }
    // ..
    if(InsufficientMaterial(pos)){
        std::cout << "Draw by insufficient material.\n";
        draw = true;
        return true;
    }
    // check draw by repetition or insufficient material
    if(DrawByRepetitions()){
        std::cout << "Draw by repetitions.\n";
        draw = true;
        return true;
    }

    return false;
}


Move Game::IterativeDeepening(){
// start the clock
    std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();

    int eval, best_eval_this_depth;
    int best_eval = pos.white_to_move ? negative_infinity : positive_infinity;
    Move move = 0;
    Move best_move = 0, best_move_this_depth = 0;
    Move moves[MAX_NUMBER_OF_MOVES] = { };
    int scores[MAX_NUMBER_OF_MOVES] = { };
    //bool win_detected = false;

    // filter out illegal moves
    int n_legal_moves = 0;
    for(int idx = 0; idx < MAX_NUMBER_OF_MOVES; idx++){
        move = pseudolegal_moves[idx];
        if(move == 0) break;
        StateMemory state;
        MakeMove(pos, move, state);
        if(IsLegal(pos, move)){
            moves[n_legal_moves] = move;
            best_move = move;
            n_legal_moves++;
        }
        UnmakeMove(pos, move, state);
    }

    // if only one move, avoid search and play immediately
    if(n_legal_moves == 1) return best_move;

    // score legal moves
    for(int idx = 0; idx < n_legal_moves; idx++){
        scores[idx] = ScoreMove(pos, moves[idx]);
    }

    // ITERATIVE DEEPENING LOOP
    for(int depth = 1; depth <= 50; depth++){

        // Reset stuff
        best_eval_this_depth = pos.white_to_move ? negative_infinity : positive_infinity;
        best_move_this_depth = 0;
        PLY = n_moves;
        if(depth >= MIN_DEPTH_LMR) LMR_ACTIVE = true;
        if(pop_count(pos.all_pieces) < 4) LMR_ACTIVE = false;

        // Loop over legal moves
        for(int idx = 0; idx < n_legal_moves; idx++){

            // pick the best move down the list (from the current index)
            for(int j = idx + 1; j < n_legal_moves; j++){
                if(scores[j] > scores[idx]){
                    std::swap(scores[idx], scores[j]);
                    std::swap(moves[idx], moves[j]);
                }
            }
            move = moves[idx];

            // if move leads to forced loss, do not consider it!
            /*if(scores[idx] < - MATE_SCORE) continue; 
            
            if(scores[idx] > MATE_SCORE){ 
                best_move_this_depth = move;
                win_detected = true;
                std::cout << "mate eval " << scores[idx] << "\n";
                break; 
            }*/

            // initialize the best move, if not initialized yet
            if(best_move_this_depth == 0) best_move_this_depth = move;

            // make the move and evaluate it
            StateMemory state_1;
            MakeMove(pos, move, state_1);
            zobrist_keys_list[n_moves + 1] = pos.zobrist_key;
            eval = BestEvaluation(pos, depth - 1, negative_infinity, positive_infinity, false);
            UnmakeMove(pos, move, state_1);

            //if((pos.white_to_move && eval >= MATE_SCORE) || (!pos.white_to_move && eval <= - MATE_SCORE))
            //    win_detected = true;

            // update best move if possible and the moves' score
            if(pos.white_to_move){
                scores[idx] = eval; // update move score
                if(eval > best_eval_this_depth){
                    best_move_this_depth = move; 
                    best_eval_this_depth = eval;
                }
                if(best_eval_this_depth > best_eval){
                    best_move = best_move_this_depth;
                    best_eval = best_eval_this_depth;
                }
            }
            else{
                scores[idx] = - eval;
                if(eval < best_eval_this_depth){
                    best_move_this_depth = move;
                    best_eval_this_depth = eval;
                }
                if(best_eval_this_depth < best_eval){
                    best_move = best_move_this_depth;
                    best_eval = best_eval_this_depth;
                }
            }

            // time out 
            if(time_up(start_time, think_time)) return best_move;

        }

        best_eval = best_eval_this_depth;
        best_move = best_move_this_depth;

        // win detected
        //if(win_detected) break;
    }

    return best_move;
}


uint16_t Game::SquareFromMouseClick(int x, int y){
    int j = (x - LEFT_PADDING) / SQUARE_SIZE;
    int i = (y - TOP_PADDING) / SQUARE_SIZE;

    if(engine_is_white){
        i = 7 - i;
        j = 7 - j;
    }

    // misclick prediction 
    if(i < 0 || i >= 8 || j < 0 || j >= 8) 
        return 64; // index out of bounds
    else
        return static_cast<uint16_t>(8*i + j);
}


void Game::FindPiece(int square){
    // square is not valid (the click was not on any square)
    if(square > 63){
        moved_piece_index = NO_PIECE;
        dragging = false;
        return; 
    }
    else{
        // loop over the pieces of player's color to find what piece is moving
        int start_idx = engine_is_white ? BLACK_KING : WHITE_KING;
        int end_idx = engine_is_white ? BLACK_PAWN : WHITE_PAWN;
        for(int idx = start_idx; idx <= end_idx; idx++){
            if(pos.pieces[idx] & (1ULL << square)){
                moved_piece_index = idx;
                dragging = true;
                return;
            }
        }
        moved_piece_index = NO_PIECE;
        dragging = false;
    }
}


void Game::ChooseTheme(std::string theme){
    if(theme == "green"){
        LIGHT_SQUARES[0] = 238; LIGHT_SQUARES[1] = 238; LIGHT_SQUARES[2] = 210;
        DARK_SQUARES[0] = 118; DARK_SQUARES[1] = 150; DARK_SQUARES[2] = 86;
    }
    if(theme == "pink"){
        LIGHT_SQUARES[0] = 250; LIGHT_SQUARES[1] = 220; LIGHT_SQUARES[2] = 245;
        DARK_SQUARES[0] = 255; DARK_SQUARES[1] = 140; DARK_SQUARES[2] = 230;
    }
    else if(theme == "sea"){
        LIGHT_SQUARES[0] = 185; LIGHT_SQUARES[1] = 225; LIGHT_SQUARES[2] = 255;
        DARK_SQUARES[0] = 15; DARK_SQUARES[1] = 150; DARK_SQUARES[2] = 250;
    }
    else {
        /*LIGHT_SQUARES[0] = 255; LIGHT_SQUARES[1] = 200; LIGHT_SQUARES[2] = 150;
        DARK_SQUARES[0] = 200; DARK_SQUARES[1] = 140; DARK_SQUARES[2] = 68;*/
        LIGHT_SQUARES[0] = 255; LIGHT_SQUARES[1] = 244; LIGHT_SQUARES[2] = 230;
        DARK_SQUARES[0] = 190; DARK_SQUARES[1] = 155; DARK_SQUARES[2] = 123;
    }
}
void Game::ChooseTheme(int theme){
    if(theme == 3){
        LIGHT_SQUARES[0] = 238; LIGHT_SQUARES[1] = 238; LIGHT_SQUARES[2] = 210;
        DARK_SQUARES[0] = 118; DARK_SQUARES[1] = 150; DARK_SQUARES[2] = 86;
    }
    else if(theme == 2){
        LIGHT_SQUARES[0] = 250; LIGHT_SQUARES[1] = 220; LIGHT_SQUARES[2] = 245;
        DARK_SQUARES[0] = 255; DARK_SQUARES[1] = 140; DARK_SQUARES[2] = 230;
    }
    else if(theme == 1){
        LIGHT_SQUARES[0] = 185; LIGHT_SQUARES[1] = 225; LIGHT_SQUARES[2] = 255;
        DARK_SQUARES[0] = 15; DARK_SQUARES[1] = 150; DARK_SQUARES[2] = 250;
    }
    else {
        /*LIGHT_SQUARES[0] = 255; LIGHT_SQUARES[1] = 200; LIGHT_SQUARES[2] = 150;
        DARK_SQUARES[0] = 200; DARK_SQUARES[1] = 140; DARK_SQUARES[2] = 68;*/
        LIGHT_SQUARES[0] = 255; LIGHT_SQUARES[1] = 244; LIGHT_SQUARES[2] = 230;
        DARK_SQUARES[0] = 190; DARK_SQUARES[1] = 155; DARK_SQUARES[2] = 123;
    }
}


void Game::ShowGame(std::string pgn_file){
    /*std::cout << "Insert path to PGN file \n";
    std::string pgn_file;
    std::cin >> pgn_file;*/

    // extract list of moves from the PGN file and save as vector of strings in Standard Algebraic Notation [SAN]
    std::vector<std::string> moves_SAN_history = ReadGameFromPGN(pgn_file);

    for(auto s: moves_SAN_history){
        std::cout << s << "\n";
    }

    size_t n_moves = moves_SAN_history.size();
    Position starting_pos = PositionFromFen(starting_position_fen);
    pos = starting_pos;
    ResetHistory();
    ResetRepetitionStack();
    ResetPseudoLegalMoves();
    PLY = 0;
    PseudoLegalMoves(pos, pseudolegal_moves);

    Move move = 0;
    std::string move_str;
    int idx_move_in_game = 0;
    SDL_Event event;
    StateMemory state;
    
    // LOOP over game history in SAN: 
    //    1. generate pseudolegal moves from pos
    //    2. LOOP over these moves and skip illegal ones
    //    3. translate the others in SAN
    //    4. if one of them matches the current move in SAN, update pos applying this move
    for(idx_move_in_game = 0; idx_move_in_game < n_moves; idx_move_in_game++){
        for(int idx = 0; idx < 256; idx++){
            move = pseudolegal_moves[idx];
            if(move == 0) break;
            move_str = AlgebraicNotation(pos, move);

            MakeMove(pos, move, state);
            if(!IsLegal(pos, move)){
                UnmakeMove(pos, move, state);
                continue;
            }
            if(move_str == moves_SAN_history[idx_move_in_game]){
                moves_list[idx_move_in_game] = move;
                break;
            }
            else{
                UnmakeMove(pos, move, state);
            }
        }

        if(move == 0) break;

        ResetPseudoLegalMoves();
        PseudoLegalMoves(pos, pseudolegal_moves);
    }


    InitGraphics();
    DrawBoard();
    SDL_RenderCopy(renderer, board_texture, NULL, NULL);
    SDL_RenderPresent(renderer);

    idx_move_in_game = 0;
    pos = starting_pos;
    StateMemory states[MAX_N_MOVES_IN_GAME];

    while (is_running) {

        while (SDL_PollEvent(&event)) {
            // Quit the game
            if (event.type == SDL_QUIT) {
                is_running = false;
                Clean();
            }
            // Explore the game
            else if(event.type == SDL_KEYDOWN){
                // right arrow
                if (event.key.keysym.sym == SDLK_RIGHT) {
                    if(idx_move_in_game < n_moves - 1){
                        move = moves_list[idx_move_in_game];
                        from = move & 0b0000000000111111;
                        to = (move >> 6) & 0b0000000000111111;
                        MakeMove(pos, move, states[idx_move_in_game]);
                        idx_move_in_game++;
                    }
                }
                // left arrow
                else if(event.key.keysym.sym == SDLK_LEFT) { 
                    if(idx_move_in_game > 0) {
                        idx_move_in_game--;
                        move = moves_list[idx_move_in_game];
                        from = move & 0b0000000000111111;
                        to = (move >> 6) & 0b0000000000111111;
                        UnmakeMove(pos, move, states[idx_move_in_game]);
                    }
                }
                // go to starting pos (down arrow)
                else if(event.key.keysym.sym == SDLK_DOWN) {
                    idx_move_in_game = 0;
                    pos = starting_pos;
                    from = 64; to = 64; // trick to avoid coloring the squares
                }
            }
        }

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, board_texture, NULL, NULL);

        if(from < 64)
            HighlightSquare(from, YELLOW_RGB); 
        if(to < 64)
            HighlightSquare(to, YELLOW_RGB);
        DrawPieces();

        SDL_RenderPresent(renderer);
        
        SDL_Delay(16); // ~60 fps
    }
    /*
    for(int idx_move_in_game = 0; idx_move_in_game < moves_SAN_history.size(); idx_move_in_game++){
        move = moves_history[idx_move_in_game];
        PrintMove(move); std::cout << " ";
    }*/
}


void Game::Clean(){
    // 8. Cleanup
    if (text_texture) SDL_DestroyTexture(text_texture);
    if (font) TTF_CloseFont(font);
    if (pieces_texture) SDL_DestroyTexture(pieces_texture);
    if (board_texture) SDL_DestroyTexture(board_texture);
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}