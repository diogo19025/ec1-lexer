#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include "codegen.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

// substitui a extensão do arquivo por ".s"
static std::string nome_saida(const std::string& entrada) {
    std::size_t ponto = entrada.rfind('.');
    if (ponto == std::string::npos)
        return entrada + ".s";
    return entrada.substr(0, ponto) + ".s";
}

// lê o texto e constrói a AST; lança ErroSintatico em caso de erro
static std::unique_ptr<Exp> construir_arvore(const std::string& texto) {
    Lexer lex(texto);
    std::vector<Token> tokens;
    while (true) {
        Token t = lex.proximo_token();
        tokens.push_back(t);
        if (t.get_tipo() == TokenType::FIM)
            break;
    }
    Parser parser(std::move(tokens));
    return parser.analisar();
}

// modo --compilar: gera o arquivo assembly a partir do fonte EC1
static int modo_compilar(const std::string& caminho) {
    std::ifstream arq(caminho);
    if (!arq.is_open()) {
        std::cerr << "Erro: nao foi possivel abrir '" << caminho << "'\n";
        return 1;
    }
    std::ostringstream buf;
    buf << arq.rdbuf();

    try {
        std::unique_ptr<Exp> arvore = construir_arvore(buf.str());

        std::string saida = nome_saida(caminho);
        std::ofstream out(saida);
        if (!out.is_open()) {
            std::cerr << "Erro: nao foi possivel criar '" << saida << "'\n";
            return 1;
        }
        gerar_assembly_completo(*arvore, out);
        std::cerr << "Assembly gerado: " << saida << "\n";
    } catch (const ErroSintatico& e) {
        std::cerr << "Erro de sintaxe: " << e.what() << "\n";
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Erro: " << e.what() << "\n";
        return 1;
    }
    return 0;
}

// modo padrão: imprime tokens, árvore e valor interpretado (Atividades 04/05)
static int modo_analisar(const std::string& caminho) {
    std::ifstream arq(caminho);
    if (!arq.is_open()) {
        std::cerr << "Erro: nao foi possivel abrir '" << caminho << "'\n";
        return 1;
    }
    std::ostringstream buf;
    buf << arq.rdbuf();
    std::string entrada = buf.str();

    // análise léxica — imprime todos os tokens
    Lexer lexer(entrada);
    std::vector<Token> tokens = lexer.tokenizar();
    for (const Token& t : tokens)
        std::cout << t << "\n";

    try {
        std::unique_ptr<Exp> arvore = construir_arvore(entrada);

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

int main(int argc, char* argv[]) {
    if (argc == 3 && std::string(argv[1]) == "--compilar")
        return modo_compilar(argv[2]);

    if (argc == 2)
        return modo_analisar(argv[1]);

    std::cerr << "Uso:\n"
              << "  " << argv[0] << " <arquivo.ec1>             -- analisa e interpreta\n"
              << "  " << argv[0] << " --compilar <arquivo.ec1>  -- gera assembly\n";
    return 1;
}
