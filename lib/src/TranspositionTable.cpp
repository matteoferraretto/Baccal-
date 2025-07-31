#include <TranspositionTable.h>

TTEntry transposition_table[TT_SIZE];

void TTInit(){
    for(int i = 0; i < TT_SIZE; i++){
        transposition_table[i].depth = -1;
        transposition_table[i].flag = EXACT;
        transposition_table[i].score = 0;
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

void TTStore(int depth, uint64_t hash, int score, NodeFlag flag/*, Move best_move*/){
    TTEntry& entry = transposition_table[hash % TT_SIZE];
    if(entry.hash != hash || depth > entry.depth){
        entry = {depth, hash, score, flag/*, best_move*/};
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
    for(int i=0; i<4; i++){
        zobrist_table.castling_rights[i] = rand64();
    }
    for(int i=0; i<8; i++){
        zobrist_table.en_passant_file[i] = rand64();
    }
}

uint64_t ZobristHashing(Position& pos) {
    // initialize value of 0
    uint64_t hash = 0;
    uint64_t piece;
    unsigned long square;
    // encode squares and pieces
    for(int piece_index = 0; piece_index < 12; piece_index++){
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
    if(pos.can_white_castle_kingside){ hash ^= zobrist_table.castling_rights[0]; }
    else if(pos.can_white_castle_queenside){ hash ^= zobrist_table.castling_rights[1]; }
    else if(pos.can_black_castle_kingside){ hash ^= zobrist_table.castling_rights[2]; }
    else if(pos.can_black_castle_queenside){ hash ^= zobrist_table.castling_rights[3]; }
    // encode en-passant target
    if(pos.en_passant_target_square){ 
        _BitScanForward64(&square, pos.en_passant_target_square);
        hash ^= zobrist_table.en_passant_file[square % 8];
    }
    return hash;
}