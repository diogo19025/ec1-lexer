#include "lexer.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

struct TokenEsperado {
    TokenType tipo;
    std::string lexema;
};

static int falhas = 0;

static void verificar(const std::string& nome,
                      const std::string& fonte,
                      const std::vector<TokenEsperado>& esperados) {
    Lexer lexer(fonte);
    for (std::size_t i = 0; i < esperados.size(); ++i) {
        Token obtido = lexer.proximo_token();
        if (obtido.get_tipo() != esperados[i].tipo ||
            obtido.get_lexema() != esperados[i].lexema) {
            std::cerr << "[FALHA] " << nome << ", token " << i << "\n"
                      << "  esperado: <"
                      << token_type_to_string(esperados[i].tipo)
                      << ", \"" << esperados[i].lexema << "\">\n"
                      << "  obtido:   " << obtido << "\n";
            ++falhas;
            return;
        }
    }
    std::cout << "[OK]    " << nome << "\n";
}

int main() {
    verificar("palavras-chave da linguagem Fun", "fun var main", {
        {TokenType::FUN, "fun"},
        {TokenType::VAR, "var"},
        {TokenType::MAIN, "main"},
        {TokenType::FIM, ""},
    });

    verificar("virgulas em listas de parametros", "soma(x, y, 10)", {
        {TokenType::IDENTIFICADOR, "soma"},
        {TokenType::PAREN_ESQ, "("},
        {TokenType::IDENTIFICADOR, "x"},
        {TokenType::VIRGULA, ","},
        {TokenType::IDENTIFICADOR, "y"},
        {TokenType::VIRGULA, ","},
        {TokenType::LITERAL, "10"},
        {TokenType::PAREN_DIR, ")"},
        {TokenType::FIM, ""},
    });

    verificar("prefixos continuam identificadores",
              "funny fun2 variable mainLoop", {
        {TokenType::IDENTIFICADOR, "funny"},
        {TokenType::IDENTIFICADOR, "fun2"},
        {TokenType::IDENTIFICADOR, "variable"},
        {TokenType::IDENTIFICADOR, "mainLoop"},
        {TokenType::FIM, ""},
    });

    if (token_type_to_string(TokenType::VIRGULA) != "Virgula" ||
        token_type_to_string(TokenType::FUN) != "Fun" ||
        token_type_to_string(TokenType::VAR) != "Var" ||
        token_type_to_string(TokenType::MAIN) != "Main") {
        std::cerr << "[FALHA] nomes dos novos tipos de token\n";
        ++falhas;
    } else {
        std::cout << "[OK]    nomes dos novos tipos de token\n";
    }

    if (falhas == 0) {
        std::cout << "Todos os testes lexicos da linguagem Fun passaram.\n";
        return EXIT_SUCCESS;
    }
    std::cerr << falhas << " teste(s) falharam.\n";
    return EXIT_FAILURE;
}
