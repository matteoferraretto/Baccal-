#include <Book.h>
#include <vector>
#include <string>

/*
std::vector<std::string> ReadPGN(std::string& file_name) {
    // open file 
    std::ifstream fin(file_name);
    std::vector<std::string> SAN_moves;
    std::string token; // any word in the file

    // loop through file words
    while(fin >> token) {

        // skip tags
        if(token[0] == '[') {
            while (token.back() != ']') { fin >> token; } 
            continue;
        }

        // skip comments
        if(token[0] == '{') {
            while (token.back() != '}') { fin >> token; } 
            continue;
        }

        // skip alternative lines ... for now.
        if(token[0] == '(') {
            while (token.back() != ')') { fin >> token; }
            continue;
        }

        std::cout << token << "\n";
        SAN_moves.push_back(token);
    }

    return SAN_moves;
}
*/

void helloworld(void) {
    std::vector<std::string> SAN_moves = ReadGameFromPGN("../assets/PGN_prova.txt");

    for(std::string& token : SAN_moves) {
        std::cout << token << "\n";
    }
}