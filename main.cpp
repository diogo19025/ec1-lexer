#include "lexer.h"
#include <fstream>
#include <iostream>
#include <sstream>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Uso: " << argv[0] << " <arquivo>\n";
        return 1;
    }

    // abre e lê o arquivo
    std::ifstream arquivo(argv[1]);
    if (!arquivo.is_open()) {
        std::cerr << "Erro: não foi possível abrir o arquivo '" << argv[1] << "'\n";
        return 1;
    }

    std::ostringstream buf;
    buf << arquivo.rdbuf();
    std::string entrada = buf.str();

    // roda o lexer
    Lexer lexer(entrada);
    std::vector<Token> tokens = lexer.tokenizar();

    // imprime cada token no formato <Tipo, "lexema", posicao>
    for (const Token& t : tokens)
        std::cout << t << "\n";

    return 0;
}
