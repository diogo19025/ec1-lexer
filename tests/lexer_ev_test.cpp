#include "lexer.h"

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

struct TokenEsperado {
    TokenType tipo;
    std::string lexema;
    std::size_t posicao;
};

static bool verificar_tokens(const std::string& nome,
                             const std::string& entrada,
                             const std::vector<TokenEsperado>& esperados) {
    Lexer lexer(entrada);

    for (std::size_t i = 0; i < esperados.size(); ++i) {
        Token obtido = lexer.proximo_token();
        const TokenEsperado& esperado = esperados[i];

        if (obtido.get_tipo() != esperado.tipo ||
            obtido.get_lexema() != esperado.lexema ||
            obtido.get_posicao() != esperado.posicao) {
            std::cerr << "[FALHOU] " << nome << ", token " << i << "\n"
                      << "  esperado: <" << token_type_to_string(esperado.tipo)
                      << ", \"" << esperado.lexema << "\", "
                      << esperado.posicao << ">\n"
                      << "  obtido:   " << obtido << "\n";
            return false;
        }
    }

    std::cout << "[PASSOU] " << nome << "\n";
    return true;
}

int main() {
    bool passou = true;

    passou &= verificar_tokens("identificador e atribuicao simples", "x = 10;", {
        {TokenType::IDENTIFICADOR, "x", 0},
        {TokenType::IGUAL, "=", 2},
        {TokenType::LITERAL, "10", 4},
        {TokenType::PONTO_VIRGULA, ";", 6},
        {TokenType::FIM, "", 7},
    });

    passou &= verificar_tokens("identificador seguido por numero", "largura2 = 30;", {
        {TokenType::IDENTIFICADOR, "largura2", 0},
        {TokenType::IGUAL, "=", 9},
        {TokenType::LITERAL, "30", 11},
        {TokenType::PONTO_VIRGULA, ";", 13},
        {TokenType::FIM, "", 14},
    });

    passou &= verificar_tokens("sequencia iniciada por digito e invalida", "237axy = 5;", {
        {TokenType::INVALIDO, "237axy", 0},
        {TokenType::IGUAL, "=", 7},
        {TokenType::LITERAL, "5", 9},
        {TokenType::PONTO_VIRGULA, ";", 10},
        {TokenType::FIM, "", 11},
    });

    {
        Lexer lexer("237axy = 5;");
        std::ostringstream erros;
        std::streambuf* destino_original = std::cerr.rdbuf(erros.rdbuf());
        std::vector<Token> tokens = lexer.tokenizar();
        std::cerr.rdbuf(destino_original);

        if (erros.str().find("237axy") == std::string::npos ||
            tokens.empty() || tokens.front().get_tipo() != TokenType::IGUAL) {
            std::cerr << "[FALHOU] reporte do erro lexico 237axy\n";
            passou = false;
        } else {
            std::cout << "[PASSOU] reporte do erro lexico 237axy\n";
        }
    }

    passou &= verificar_tokens("letras maiusculas e minusculas", "Area2", {
        {TokenType::IDENTIFICADOR, "Area2", 0},
        {TokenType::FIM, "", 5},
    });

    if (token_type_to_string(TokenType::IDENTIFICADOR) != "Identificador" ||
        token_type_to_string(TokenType::IGUAL) != "Igual" ||
        token_type_to_string(TokenType::PONTO_VIRGULA) != "PontoVirgula") {
        std::cerr << "[FALHOU] conversao dos novos tipos de token\n";
        passou = false;
    } else {
        std::cout << "[PASSOU] conversao dos novos tipos de token\n";
    }

    return passou ? EXIT_SUCCESS : EXIT_FAILURE;
}
