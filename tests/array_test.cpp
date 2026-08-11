#include "ast.h"
#include "codegen.h"
#include "lexer.h"
#include "parser.h"
#include "semantica.h"

#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

static int falhas = 0;

static void checar(bool condicao, const std::string& descricao) {
    if (condicao) {
        std::cout << "[OK]    " << descricao << "\n";
    } else {
        std::cout << "[FALHA] " << descricao << "\n";
        ++falhas;
    }
}

static std::unique_ptr<Programa> parse(const std::string& fonte) {
    Lexer lexer(fonte);
    std::vector<Token> tokens;
    while (true) {
        Token token = lexer.proximo_token();
        tokens.push_back(token);
        if (token.get_tipo() == TokenType::FIM)
            break;
    }
    Parser parser(std::move(tokens));
    return parser.analisar();
}

static bool aceita(const std::string& fonte) {
    try {
        parse(fonte);
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

static bool rejeita_semantico(const std::string& fonte) {
    try {
        parse(fonte);
    } catch (const ErroSemantico&) {
        return true;
    } catch (const std::exception&) {
        return false;
    }
    return false;
}

static std::string gerar(const std::string& fonte) {
    std::unique_ptr<Programa> programa = parse(fonte);
    std::ostringstream saida;
    gerar_assembly_completo(*programa, saida);
    return saida.str();
}

static bool contem(const std::string& texto, const std::string& trecho) {
    return texto.find(trecho) != std::string::npos;
}

static void teste_lexer_e_ast() {
    Lexer lexer("var dados[4]; main { dados[2] = 7; return dados[2]; }");
    std::vector<Token> tokens = lexer.tokenizar();
    bool abriu = false;
    bool fechou = false;
    for (const Token& token : tokens) {
        abriu = abriu || token.get_tipo() == TokenType::COLCHETE_ESQ;
        fechou = fechou || token.get_tipo() == TokenType::COLCHETE_DIR;
    }
    checar(abriu && fechou, "lexer reconhece os dois colchetes");

    std::unique_ptr<Programa> programa = parse(
        "var dados[4]; main { dados[1 + 1] = 7; return dados[2]; }");
    const Decl& decl = programa->get_decls().front();
    checar(decl.eh_array() && decl.get_tamanho_array() == 4,
           "AST guarda a declaracao e o tamanho fixo do array");

    const auto& comandos = programa->get_corpo().get_comandos();
    checar(dynamic_cast<const AtribuicaoArray*>(comandos[0].get()) != nullptr,
           "AST distingue atribuicao indexada");
    const auto* retorno = dynamic_cast<const Retorno*>(comandos[1].get());
    checar(retorno != nullptr &&
           dynamic_cast<const AcessoArray*>(&retorno->get_valor()) != nullptr,
           "AST distingue leitura indexada");
    checar(contem(programa->imprimir(), "var dados[4];") &&
           contem(programa->imprimir(), "dados[(1 + 1)] = 7;"),
           "impressao da AST preserva a sintaxe dos arrays");
}

static void teste_semantica() {
    checar(aceita(
        "var dados[3]; main { var i = 1; dados[i] = 9; return dados[i]; }"),
        "array global aceita indice calculado");
    checar(aceita(
        "fun f(i) { var dados[3]; dados[i] = i + 1; return dados[i]; }"
        "main { return f(2); }"),
        "array local funciona dentro de funcao");
    checar(aceita(
        "var dados[2]; fun f() { var dados[3]; dados[2] = 7; return dados[2]; }"
        "main { return f(); }"),
        "array local pode esconder array global");

    checar(rejeita_semantico("var dados[0]; main { return 0; }"),
           "array de tamanho zero e rejeitado");
    checar(rejeita_semantico("var dados[2]; main { return dados; }"),
           "array usado sem indice e rejeitado");
    checar(rejeita_semantico("var x = 1; main { return x[0]; }"),
           "variavel escalar usada como array e rejeitada");
    checar(rejeita_semantico("var dados[2]; main { return dados[2]; }"),
           "indice literal fora dos limites e rejeitado");
    checar(rejeita_semantico(
        "var dados[2]; main { dados = 1; return 0; }"),
        "atribuicao ao array sem indice e rejeitada");
    checar(rejeita_semantico(
        "var dados[2]; main { dados[i] = 1; return 0; }"),
        "variavel nao declarada no indice e rejeitada");
}

static void teste_codegen() {
    std::string global = gerar(
        "var dados[4]; main { dados[2] = 9; return dados[2]; }");
    checar(contem(global, ".lcomm dados, 32"),
           "array global reserva oito bytes por elemento na secao .bss");
    checar(contem(global, "lea dados, %rdx") &&
           contem(global, "mov %rax, (%rdx,%rbx,8)") &&
           contem(global, "mov (%rdx,%rax,8), %rax"),
           "leitura e escrita usam enderecamento base mais indice vezes oito");

    std::string local = gerar(
        "fun f() { var dados[3]; var x = 5; dados[1] = x; return dados[1]; }"
        "main { return f(); }");
    checar(contem(local, "sub $32, %rsp"),
           "frame local soma os elementos do array e as variaveis escalares");
    checar(contem(local, "lea -24(%rbp), %rdx"),
           "primeiro elemento do array local recebe o deslocamento correto");
    checar(contem(local, "mov %rax, -32(%rbp)"),
           "variavel declarada depois do array nao sobrepoe seus elementos");
    checar(contem(local, "movq $0, -24(%rbp)") &&
           contem(local, "movq $0, -16(%rbp)") &&
           contem(local, "movq $0, -8(%rbp)"),
           "array local e inicializado com zeros");
}

int main() {
    teste_lexer_e_ast();
    teste_semantica();
    teste_codegen();

    if (falhas == 0) {
        std::cout << "Todos os testes de arrays passaram.\n";
        return 0;
    }
    std::cout << falhas << " teste(s) falharam.\n";
    return 1;
}
