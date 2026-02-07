#include <Book.h>

//std::vector<std::string> SAN_moves = ReadGameFromPGN("../assets/PGN_prova.txt");
BookEntry book[BOOK_SIZE];

// 1. generate all the pseudolegal moves from pos
// Loop over all moves:
// 2. check legality
// 3. transform move in SAN notation and check if it matches the given SAN
// 4. make move
void ApplyMoveSAN(Position& pos, std::string move_SAN){
    StateMemory state;
    Move move = 0;
    Move pseudolegal_moves[MAX_NUMBER_OF_MOVES];
    for(int i = 0; i < MAX_NUMBER_OF_MOVES; i++){
        pseudolegal_moves[i] = 0;
    }
    PseudoLegalMoves(pos, pseudolegal_moves);

    for(int i = 0; i < MAX_NUMBER_OF_MOVES; i++){
        move = pseudolegal_moves[i];
        if(move_SAN == AlgebraicNotation(pos, move) && IsLegal(pos, move)){
            MakeMove(pos, move, state);
            return;
        }
    }
}

Move MoveFromSAN(Position pos, std::string move_SAN){
    StateMemory state;
    Move move = 0;
    Move pseudolegal_moves[MAX_NUMBER_OF_MOVES];
    for(int i = 0; i < MAX_NUMBER_OF_MOVES; i++){
        pseudolegal_moves[i] = 0;
    }
    PseudoLegalMoves(pos, pseudolegal_moves);

    for(int i = 0; i < MAX_NUMBER_OF_MOVES; i++){
        move = pseudolegal_moves[i];
        if(move_SAN == AlgebraicNotation(pos, move) && IsLegal(pos, move)){
            break;
        }
    }
    return move;
}



void BookInit(){
    for(int i = 0; i < BOOK_SIZE; i++){
        book[i].hash = 0ULL;
        book[i].n_memorized_moves = 0;
        for(int j = 0; j < MAX_MEMORIZED_MOVES; j++){
            book[i].possible_moves[j] = 0;
            book[i].weights[j] = 0.0;
        }
    }
}


BookEntry* BookProbe(uint64_t zobrist_key){
    // go at the memory address of table corresponding to the given Zobrist key
    BookEntry& entry = book[zobrist_key % BOOK_SIZE];
    // if entry's position hash matches the Zobrist key, return a pointer to that entry
    if(entry.hash == zobrist_key) return &entry;
    return nullptr;
}

void BookStoreMove(uint64_t zobrist_key, Move move){
    BookEntry& entry = book[zobrist_key % BOOK_SIZE];
    entry.hash = zobrist_key;
    // if there is space...
    if(entry.n_memorized_moves < MAX_MEMORIZED_MOVES - 1){
        // if move is already stored, no need to store it
        for(int i = 0; i < entry.n_memorized_moves; i++){
            if(entry.possible_moves[i] == move){ 
                std::cout << "move: " << move << "\n"; 
                std::cout << entry.n_memorized_moves << "\n";
                return;
            }
        }
        // else, store the move
        entry.possible_moves[entry.n_memorized_moves] = move;
        entry.n_memorized_moves++;
    }
}

void FillBook() {
    std::string filename;
    std::vector<std::string> moves_SAN; // extract list of moves in SAN notation
    size_t n_moves = 0;
    Position pos;
    Move move;
    BookEntry* entry;

    // Loop through all the openings
    for(auto& opening_name : openings){
        filename = book_dir + opening_name + ".txt";
        std::cout << filename << "\n";
        moves_SAN = ReadGameFromPGN(filename);
        n_moves = moves_SAN.size();
        // assuming that the starting pos is the standard one, not the one saved in the PGN
        pos = PositionFromFen(starting_position_fen);
        
        // Loop through all the moves
        for(int i = 0; i < n_moves; i++){
            // if the position is not stored, store it
            entry = BookProbe(pos.zobrist_key);
            move = MoveFromSAN(pos, moves_SAN[i]);
            BookStoreMove(pos.zobrist_key, move); 
            // apply the move to evolve the position 
            ApplyMoveSAN(pos, moves_SAN[i]);
        }
    }
}