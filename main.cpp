#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include <fstream>
#include <iostream>
#include <sstream>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Uso: " << argv[0] << " <arquivo>\n";
        return 1;
    }

    std::ifstream arquivo(argv[1]);
    if (!arquivo.is_open()) {
        std::cerr << "Erro: não foi possível abrir o arquivo '" << argv[1] << "'\n";
        return 1;
    }

    std::ostringstream buf;
    buf << arquivo.rdbuf();
    std::string entrada = buf.str();

    Lexer lexer(entrada);
    std::vector<Token> tokens = lexer.tokenizar();

    for (const Token& t : tokens)
        std::cout << t << "\n";

    Lexer lexer_sintatico(entrada);
    std::vector<Token> tokens_sintaticos;
    while (true) {
        Token t = lexer_sintatico.proximo_token();
        tokens_sintaticos.push_back(t);
        if (t.get_tipo() == TokenType::FIM)
            break;
    }

    try {
        Parser parser(std::move(tokens_sintaticos));
        std::unique_ptr<Exp> arvore = parser.analisar();

        std::cout << "\nArvore (linear): " << arvore->imprimir() << "\n";
        std::cout << "Arvore (visual):\n";
        arvore->imprimir_arvore(std::cout, 1);
        std::cout << "Valor: " << arvore->avaliar() << "\n";
    } catch (const ErroSintatico& e) {
        std::cerr << "Erro de sintaxe: " << e.what() << "\n";
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Erro: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
