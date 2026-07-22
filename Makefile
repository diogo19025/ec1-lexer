CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra
SRC      := $(wildcard src/*.cpp)
BIN      := ec1
LEXER_TEST_BIN  := lexer_ev_tests
PARSER_TEST_BIN := parser_ev_tests
PARSER_CMD_TEST_BIN := parser_cmd_tests

$(BIN): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(BIN)

# ---- testes ----------------------------------------------------------------
# test-sin      : análise sintática e interpretação (Atividade 05)
# test-ec2      : precedência e associatividade      (Atividade 07)
# test-cod      : geração de código assembly          (Atividade 06, requer Linux/as/ld)
# test-lex-ev   : análise léxica da linguagem EV       (Atividade 08 - Pessoa 1)
# test-parser-ev: parser + análise semântica da EV     (Atividade 08 - Pessoa 3)
# test-cod-ev   : geração de código da linguagem EV    (Atividade 08 - Pessoa 4, requer Linux/as/ld)
# test-parser-cmd: parser dos comandos da linguagem Cmd (Atividade 09 - parte 2)
test: test-sin test-ec2 test-lex-ev test-parser-ev test-parser-cmd test-cod-ev

test-sin: $(BIN)
	bash scripts/run_tests.sh

test-ec2: $(BIN)
	bash scripts/run_tests_ec2.sh

test-lex-ev:
	$(CXX) $(CXXFLAGS) -Isrc tests/lexer_ev_test.cpp src/lexer.cpp src/token.cpp -o $(LEXER_TEST_BIN)
	./$(LEXER_TEST_BIN)

test-parser-ev:
	$(CXX) $(CXXFLAGS) -Isrc tests/parser_ev_test.cpp src/lexer.cpp src/token.cpp src/parser.cpp src/ast.cpp src/semantica.cpp -o $(PARSER_TEST_BIN)
	./$(PARSER_TEST_BIN)

test-parser-cmd:
	$(CXX) $(CXXFLAGS) -Isrc tests/parser_cmd_test.cpp src/lexer.cpp src/token.cpp src/parser.cpp src/ast.cpp src/semantica.cpp -o $(PARSER_CMD_TEST_BIN)
	./$(PARSER_CMD_TEST_BIN)

test-cod: $(BIN)
	bash scripts/run_tests_ativ06.sh

test-cod-ev: $(BIN)
	bash scripts/run_tests_ev.sh

clean:
	rm -f $(BIN) $(BIN).exe $(LEXER_TEST_BIN) $(LEXER_TEST_BIN).exe $(PARSER_TEST_BIN) $(PARSER_TEST_BIN).exe $(PARSER_CMD_TEST_BIN) $(PARSER_CMD_TEST_BIN).exe

.PHONY: test test-sin test-ec2 test-lex-ev test-parser-ev test-parser-cmd test-cod test-cod-ev clean
