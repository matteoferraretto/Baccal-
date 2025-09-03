#include <Game.h>
#include <Utilities.h>
#include <Baccala.h>
#include <TranspositionTable.h>
#include <iostream>
#include <string>
#include <chrono>

Game::Game(){

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
    std::cout << "Let the battle begin!\n\n";

    if(normal_game == "n")
        StartNewGame();
    else{
        std::cout << "Insert FEN string of starting position\n";
        std::getline( std::cin, position_fen );
        StartNewGame(position_fen);
    }

    InitGraphics();
    DrawBoard();      // genera la texture della scacchiera
    //DrawPieces();
    // Componi tutto in un unico frame
    //SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, board_texture, NULL, NULL);
    SDL_RenderPresent(renderer);

    SDL_Event event;
    StateMemory state;

    while (is_running) {

        // engine move
        if(pos.white_to_move == engine_is_white){
            engine_move = IterativeDeepening();
            MakeMove(pos, engine_move, state);
            ResetPseudoLegalMoves();
            PseudoLegalMoves(pos, pseudolegal_moves);
            n_moves++;
            repetition_stack[n_moves] = pos.zobrist_key;
        }

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
                    uint16_t mask = 0b0000111111111111;
                    for(int idx = 0; idx < MAX_NUMBER_OF_MOVES; idx++){
                        // if we have finished the pseudolegal moves, break
                        if(pseudolegal_moves[idx] == 0) break;
                        // if the input move matches one of the precomputed pseudolegal moves
                        if((pseudolegal_moves[idx] & mask) == player_move){
                            player_move = pseudolegal_moves[idx];
                            MakeMove(pos, player_move, state);
                            if(!IsLegal(pos, player_move)) 
                                UnmakeMove(pos, player_move, state);
                            else{ // move is legal
                                n_moves++;
                                repetition_stack[n_moves] = pos.zobrist_key;
                                ResetPseudoLegalMoves();
                                PseudoLegalMoves(pos, pseudolegal_moves);
                            }
                        }
                    }
                }
            }
            else if (event.type == SDL_MOUSEMOTION) {
                mouse_x = event.motion.x;
                mouse_y = event.motion.y;
            }
        }

        
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, board_texture, NULL, NULL);
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
                SDL_Delay(50); // avoid burning CPU
            }
        }

        SDL_Delay(16); // ~60 fps
    }
}

Game::~Game() {
    Clean();
}


void Game::InitGraphics(){
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init error: " << SDL_GetError() << std::endl;
        return;
    }
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        std::cerr << "SDL_image Init Error: " << IMG_GetError() << std::endl;
        return;
    }

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

}


void Game::DrawBoard(){
    SDL_RenderClear(renderer);

    board_texture = SDL_CreateTexture(
        renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, WIDTH, HEIGHT
    );

    SDL_SetRenderTarget(renderer, board_texture);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

    // quadrato colorato (per test)
    const int size = 90, left_padding = 40, top_padding = 40;
    int i, j;
    SDL_Rect rect[64];
    for(int square = 0; square < 64; square++){
        i = square / 8; j = square % 8;
        rect[square].x = LEFT_PADDING + SQUARE_SIZE * j;
        rect[square].y = TOP_PADDING + SQUARE_SIZE * i; 
        rect[square].w = SQUARE_SIZE;
        rect[square].h = SQUARE_SIZE;

        if((i+j) % 2 == 0) SDL_SetRenderDrawColor(renderer, /*200, 150, 80,*/LIGHT_SQUARES[0], LIGHT_SQUARES[1], LIGHT_SQUARES[2], 255);
        else SDL_SetRenderDrawColor(renderer, /*250, 200, 160,*/DARK_SQUARES[0], DARK_SQUARES[1], DARK_SQUARES[2], 255);

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


void Game::StartNewGame(void){
    ResetHistory();
    ResetPseudoLegalMoves();
    PLY = 0;
    ResetRepetitionStack();
    pos = PositionFromFen(starting_position_fen);
    PrintBoard(pos, !engine_is_white);
    PseudoLegalMoves(pos, pseudolegal_moves);
    repetition_stack[0] = pos.zobrist_key;
}


void Game::StartNewGame(std::string position_fen){
    ResetHistory();
    ResetPseudoLegalMoves();
    PLY = 0;
    ResetRepetitionStack();
    pos = PositionFromFen(position_fen);
    PrintBoard(pos, !engine_is_white);
    PseudoLegalMoves(pos, pseudolegal_moves);
    repetition_stack[0] = pos.zobrist_key;
}


void Game::PlayVsEngine(void){
    Move engine_move;
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
    }
}


bool Game::AskPromotion(){
    
    uint16_t i = to / 8, j = to % 8;

    SDL_Rect options[4];
    int promoted_piece_indexes[4];

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
                LEFT_PADDING + j * SQUARE_SIZE, 
                TOP_PADDING + (7 - idx) * SQUARE_SIZE,
                SQUARE_SIZE,
                SQUARE_SIZE
            };
            promoted_piece_indexes[idx] = 6 + idx; 
        }
    }
    // don't deal with other types of move in this function
    else return false;

    // before asking for promotion, check legality
    // if illegal -> return to main loop
    StateMemory state;
    uint16_t flags;

    // compare with pseudolegal moves, and if there's matching, make the move and check legality
    uint16_t mask = 0b0000111111111111;
    for(int idx = 0; idx < MAX_NUMBER_OF_MOVES; idx++){
        // if we have finished the pseudolegal moves, break
        if(pseudolegal_moves[idx] == 0) break;
        // if the input move matches one of the precomputed pseudolegal moves
        if((pseudolegal_moves[idx] & mask) == player_move){
            MakeMove(pos, pseudolegal_moves[idx], state);
            if(!IsLegal(pos, pseudolegal_moves[idx])) {
                UnmakeMove(pos, pseudolegal_moves[idx], state);
                return false;
            }
            UnmakeMove(pos, pseudolegal_moves[idx], state);
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
            if (e.type == SDL_QUIT) { is_running = false; wait = false; }

            // left click
            if (e.type == SDL_MOUSEBUTTONDOWN) {
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
                        PrintMove(player_move); std::cout << "\n";
                        MakeMove(pos, player_move, state);
                        n_moves++;
                        repetition_stack[n_moves] = pos.zobrist_key;
                        ResetPseudoLegalMoves();
                        PseudoLegalMoves(pos, pseudolegal_moves);
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
            if(pos.white_to_move)
                std::cout << "Checkmate. Black wins.\n";
            else
                std::cout << "Checkmate. White wins.\n";
        }
        // otherwise it's stalemate
        else{
            std::cout << "Draw by stalemate.\n";
        }
        return true;
    }
    // ..
    if(InsufficientMaterial(pos)){
        std::cout << "Draw by insufficient material.\n";
        return true;
    }
    // check draw by repetition or insufficient material
    if(DrawByRepetitions()){
        std::cout << "Draw by repetitions.\n";
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
    bool win_detected = false;

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

    // score legal moves
    for(int idx = 0; idx < n_legal_moves; idx++){
        scores[idx] = ScoreMove(pos, moves[idx]);
    }


    // ITERATIVE DEEPENING LOOP
    for(int depth = 1; depth <= 50; depth++){

        // Reset stuff
        best_eval_this_depth = pos.white_to_move ? negative_infinity : positive_infinity;
        PLY = n_moves;
        if(depth >= MIN_DEPTH_LMR) LMR_ACTIVE = true;

        // Loop over legal moves
        for(int idx = 0; idx < n_legal_moves; idx++){

            // time out 
            if(time_up(start_time, think_time)) return best_move;

            // pick the best move down the list (from the current index)
            for(int j = idx + 1; j < n_legal_moves; j++){
                if(scores[j] > scores[idx]){
                    std::swap(scores[idx], scores[j]);
                    std::swap(moves[idx], moves[j]);
                }
            }
            move = moves[idx];

            // if move leads to forced loss, do not consider it!
            if(scores[idx] < - MATE_SCORE) continue; 
            
            if(scores[idx] > MATE_SCORE){ 
                best_move_this_depth = move;
                win_detected = true;
                break; 
            }

            // initialize the best move, if not initialized yet
            if(best_move_this_depth == 0) best_move_this_depth = move;

            // make the move and evaluate it
            StateMemory state_1;
            MakeMove(pos, move, state_1);
            zobrist_keys_list[n_moves + 1] = pos.zobrist_key;
            eval = BestEvaluation(pos, depth - 1, negative_infinity, positive_infinity, false);
            UnmakeMove(pos, move, state_1);

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

        }

        best_eval = best_eval_this_depth;
        best_move = best_move_this_depth;

        // win detected
        if(win_detected) break;

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
    if(theme == "pink"){
        LIGHT_SQUARES[0] = 255; LIGHT_SQUARES[1] = 143; LIGHT_SQUARES[2] = 233;
        DARK_SQUARES[0] = 250; DARK_SQUARES[1] = 220; DARK_SQUARES[2] = 244;
    }
    else if(theme == "sea"){
        LIGHT_SQUARES[0] = 185; LIGHT_SQUARES[1] = 225; LIGHT_SQUARES[2] = 255;
        DARK_SQUARES[0] = 15; DARK_SQUARES[1] = 150; DARK_SQUARES[2] = 250;
    }
    else {
        LIGHT_SQUARES[0] = 255; LIGHT_SQUARES[1] = 200; LIGHT_SQUARES[2] = 150;
        DARK_SQUARES[0] = 200; DARK_SQUARES[1] = 140; DARK_SQUARES[2] = 68;
    }
}


void Game::Clean(){
    if (pieces_texture) SDL_DestroyTexture(pieces_texture);
    if (board_texture) SDL_DestroyTexture(board_texture);
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
}