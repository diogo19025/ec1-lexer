#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include "codegen.h"
#include "semantica.h"
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

// constrói a AST (um Programa: declarações + expressão final, corpo de
// comandos ou bloco main) a partir dos tokens já produzidos pelo lexer;
// lança ErroSintatico ou ErroSemantico em caso de erro
static std::unique_ptr<Programa> construir_arvore(std::vector<Token> tokens) {
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

    // análise léxica: um caractere fora do alfabeto da linguagem é um erro
    // léxico, e não faz sentido analisar a sintaxe de uma entrada da qual
    // tokens foram descartados (o parser reportaria um erro de sintaxe
    // qualquer, apontando para o lugar errado)
    Lexer lex(buf.str());
    std::vector<Token> tokens = lex.tokenizar();
    if (lex.houve_erro_lexico()) {
        std::cerr << "Erro lexico: a entrada tem caracteres fora do alfabeto "
                     "da linguagem (veja as mensagens acima)\n";
        return 1;
    }

    try {
        std::unique_ptr<Programa> arvore = construir_arvore(std::move(tokens));

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
    } catch (const ErroSemantico& e) {
        std::cerr << "Erro semantico: " << e.what() << "\n";
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

    // os tokens válidos já foram mostrados acima; se algum era inválido, a
    // lista está incompleta e analisar a sintaxe dela só produziria um erro
    // enganoso, apontando para depois do caractere que causou o problema
    if (lexer.houve_erro_lexico()) {
        std::cerr << "Erro lexico: a entrada tem caracteres fora do alfabeto "
                     "da linguagem (veja as mensagens acima)\n";
        return 1;
    }

    try {
        std::unique_ptr<Programa> arvore = construir_arvore(std::move(tokens));

        std::cout << "\nArvore (linear): " << arvore->imprimir() << "\n";
        std::cout << "Arvore (visual):\n";
        arvore->imprimir_arvore(std::cout, 1);

        // a interpretacao direta (avaliar) so existe para expressoes sem
        // variaveis: Var::avaliar() lanca erro, pois nao ha um ambiente de
        // valores nesta etapa (a avaliacao de programas com variaveis,
        // comandos ou funcoes e feita gerando e executando o assembly,
        // com --compilar).
        if (arvore->tem_corpo()) {
            std::cout << "(programa com comandos ou funcoes; use --compilar "
                       << "para gerar e executar o assembly)\n";
        } else if (arvore->get_decls().empty()) {
            std::cout << "Valor: " << arvore->get_exp().avaliar() << "\n";
        } else {
            std::cout << "(programa usa variaveis; use --compilar para "
                       << "gerar e executar o assembly)\n";
        }
    } catch (const ErroSintatico& e) {
        std::cerr << "Erro de sintaxe: " << e.what() << "\n";
        return 1;
    } catch (const ErroSemantico& e) {
        std::cerr << "Erro semantico: " << e.what() << "\n";
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
