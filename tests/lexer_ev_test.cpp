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

    passou &= verificar_tokens("chaves e operadores de comparacao", "{ a < b > c == d }", {
        {TokenType::CHAVE_ESQ, "{", 0},
        {TokenType::IDENTIFICADOR, "a", 2},
        {TokenType::MENOR, "<", 4},
        {TokenType::IDENTIFICADOR, "b", 6},
        {TokenType::MAIOR, ">", 8},
        {TokenType::IDENTIFICADOR, "c", 10},
        {TokenType::IGUALDADE, "==", 12},
        {TokenType::IDENTIFICADOR, "d", 15},
        {TokenType::CHAVE_DIR, "}", 17},
        {TokenType::FIM, "", 18},
    });

    passou &= verificar_tokens("atribuicao e igualdade sao tokens distintos", "x = y == 10;", {
        {TokenType::IDENTIFICADOR, "x", 0},
        {TokenType::IGUAL, "=", 2},
        {TokenType::IDENTIFICADOR, "y", 4},
        {TokenType::IGUALDADE, "==", 6},
        {TokenType::LITERAL, "10", 9},
        {TokenType::PONTO_VIRGULA, ";", 11},
        {TokenType::FIM, "", 12},
    });

    passou &= verificar_tokens("palavras-chave", "if else while return", {
        {TokenType::IF, "if", 0},
        {TokenType::ELSE, "else", 3},
        {TokenType::WHILE, "while", 8},
        {TokenType::RETURN, "return", 14},
        {TokenType::FIM, "", 20},
    });

    passou &= verificar_tokens("palavras-chave como prefixo de identificadores",
                               "ifx else2 whileLoop returnValue", {
        {TokenType::IDENTIFICADOR, "ifx", 0},
        {TokenType::IDENTIFICADOR, "else2", 4},
        {TokenType::IDENTIFICADOR, "whileLoop", 10},
        {TokenType::IDENTIFICADOR, "returnValue", 20},
        {TokenType::FIM, "", 31},
    });

    passou &= verificar_tokens("simbolos adjacentes", "{}<>>===", {
        {TokenType::CHAVE_ESQ, "{", 0},
        {TokenType::CHAVE_DIR, "}", 1},
        {TokenType::MENOR, "<", 2},
        {TokenType::MAIOR, ">", 3},
        {TokenType::MAIOR, ">", 4},
        {TokenType::IGUALDADE, "==", 5},
        {TokenType::IGUAL, "=", 7},
        {TokenType::FIM, "", 8},
    });

    if (token_type_to_string(TokenType::IDENTIFICADOR) != "Identificador" ||
        token_type_to_string(TokenType::IGUAL) != "Igual" ||
        token_type_to_string(TokenType::PONTO_VIRGULA) != "PontoVirgula" ||
        token_type_to_string(TokenType::IGUALDADE) != "Igualdade" ||
        token_type_to_string(TokenType::CHAVE_ESQ) != "ChaveEsq" ||
        token_type_to_string(TokenType::CHAVE_DIR) != "ChaveDir" ||
        token_type_to_string(TokenType::MENOR) != "Menor" ||
        token_type_to_string(TokenType::MAIOR) != "Maior" ||
        token_type_to_string(TokenType::IF) != "If" ||
        token_type_to_string(TokenType::ELSE) != "Else" ||
        token_type_to_string(TokenType::WHILE) != "While" ||
        token_type_to_string(TokenType::RETURN) != "Return") {
        std::cerr << "[FALHOU] conversao dos novos tipos de token\n";
        passou = false;
    } else {
        std::cout << "[PASSOU] conversao dos novos tipos de token\n";
    }

    return passou ? EXIT_SUCCESS : EXIT_FAILURE;
}
