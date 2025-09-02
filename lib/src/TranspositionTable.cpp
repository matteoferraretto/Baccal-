#include <TranspositionTable.h>
#include <Bitboards.h>
#include <algorithm>

TTEntry transposition_table[TT_SIZE];
TTEntryPerft transposition_table_perft[TT_SIZE];

void TTInit(){
    for(int i = 0; i < TT_SIZE; i++){
        transposition_table[i].depth = -1;
        transposition_table[i].hash = 0ULL;
        transposition_table[i].flag = EXACT;
        transposition_table[i].score = 0;
        transposition_table[i].best_move = 0;
    }
}

void TTPerftInit(){
    for(int i = 0; i < TT_SIZE; i++){
        transposition_table_perft[i].depth = -1;
        transposition_table_perft[i].hash = 0ULL;
        transposition_table_perft[i].perft = 0;
    }
}

TTEntry* TTProbe(uint64_t zobrist_key){
    // go at the memory address of table corresponding to the given Zobrist key
    TTEntry& entry = transposition_table[zobrist_key % TT_SIZE];
    // if entry's position hash matches the Zobrist key, return a pointer to that entry
    if(entry.hash == zobrist_key){
        return &entry;
    }
    return nullptr;
}

TTEntryPerft* TTPerftProbe(uint64_t zobrist_key){
    // go at the memory address of table corresponding to the given Zobrist key
    TTEntryPerft& entry = transposition_table_perft[zobrist_key % TT_SIZE];
    // if entry's position hash matches the Zobrist key, return a pointer to that entry
    if(entry.hash == zobrist_key){
        return &entry;
    }
    return nullptr;
}

void TTStore(int depth, uint64_t hash, int score, NodeFlag flag, Move best_move){
    TTEntry& entry = transposition_table[hash % TT_SIZE];
    if(depth >= entry.depth){
        TT_ENTRIES++;
        if(entry.hash == hash) TT_ENTRIES--;
        entry = {depth, hash, score, flag, best_move};
    }
}

void TTPerftStore(int depth, uint64_t hash, unsigned long long int score){
    TTEntryPerft& entry = transposition_table_perft[hash % TT_SIZE];
    if(depth >= entry.depth){
        entry = {depth, hash, score};
    }
}


ZobristTable zobrist_table;

void InitializeZobrist(){
    for(int piece = 0; piece < 12; piece++){
        for(int square = 0; square < 64; square++){
            zobrist_table.pieces_and_squares[piece][square] = rand64();
        }
    }
    zobrist_table.white_to_move = rand64();
    for(int i=0; i<16; i++){
        zobrist_table.castling_rights[i] = rand64();
    }
    for(int i=0; i<8; i++){
        zobrist_table.en_passant_file[i] = rand64();
    }
}

uint8_t CastlingHashing(const Position& pos) {
    uint8_t castling_hash = 
        static_cast<uint8_t>(pos.can_white_castle_kingside) | 
        (static_cast<uint8_t>(pos.can_white_castle_queenside) << 1) | 
        (static_cast<uint8_t>(pos.can_black_castle_kingside) << 2) |
        (static_cast<uint8_t>(pos.can_black_castle_queenside) << 3);
    return castling_hash;
}

uint64_t ZobristHashing(Position& pos) {
    // initialize value of 0
    uint64_t hash = 0;
    uint64_t piece;
    unsigned long square;
    // encode squares and pieces
    for(int piece_index = WHITE_KING; piece_index <= BLACK_PAWN; piece_index++){
        piece = pos.pieces[piece_index];
        while(piece){
            _BitScanForward64(&square, piece);
            hash ^= zobrist_table.pieces_and_squares[piece_index][square];
            clear_last_active_bit(piece);
        }
    }
    // encode side to move
    if(pos.white_to_move){ hash ^= zobrist_table.white_to_move; }
    // encode castling rights 
    uint8_t castling_hash = CastlingHashing(pos);
    hash ^= zobrist_table.castling_rights[castling_hash];
    // encode en-passant target
    if(pos.en_passant_target_square){ 
        // we hash the e.p. target ONLY if capture is actually possible
        _BitScanForward64(&square, pos.en_passant_target_square);
        uint64_t attacks;
        if(pos.white_to_move){
            // imagine a fictitious pawn in the e.p. target and compute its attacks
            attacks = black_pawn_covered_squares_bitboards[square];
            // if there's a white pawn there, hash!
            if(attacks & pos.pieces[WHITE_PAWN]) hash ^= zobrist_table.en_passant_file[square % 8];
        } else {
            attacks = white_pawn_covered_squares_bitboards[square];
            if(attacks & pos.pieces[BLACK_PAWN]) hash ^= zobrist_table.en_passant_file[square % 8];
        }
    }
    return hash;
}

uint64_t repetition_stack[SIZE_REPETITION_STACK] = { };

void PrintRepetitionStack(){
    for(int i = 0; i < SIZE_REPETITION_STACK; i++){
        if(repetition_stack[i] == 0) break;
        std::cout << repetition_stack[i] << "; ";
    }
    std::cout << "\n";
}

void ResetRepetitionStack(){
    for(int i = 0; i < SIZE_REPETITION_STACK; i++){
        repetition_stack[i] = 0;
    }
}

bool ThreeRepetitions(const Position& pos, int ply){
    // if not enough deep down the tree, draw by repetition is impossible
    // this line will change when we will include game history.
    if(ply < 6) return false;

    int count = 0;
    // steps of 2 because a repetition can be found only if the side to move is the current side to move 
    for(int i = 0; i <= pos.half_move_counter; i += 2){
        if(repetition_stack[ply - i] == pos.zobrist_key)
            count++;
        if(count == 2) return true;
    }
    return false;
}


bool ThreeRepetitions(const Position& pos, uint64_t *history, int root_ply, int ply){
    // if not enough deep down the tree, draw by repetition is impossible
    // this line will change when we will include game history.
    if(ply < 6) return false;

    int count = 0;
    // steps of 2 because a repetition can be found only if the side to move is the current side to move 
    for(int i = 0; i <= pos.half_move_counter; i += 2){
        if(history[root_ply + ply - i] == pos.zobrist_key)
            count++;
        if(count == 2) return true;
    }
    return false;
}