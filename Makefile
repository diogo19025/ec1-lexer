CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra
SRC      := $(wildcard src/*.cpp)
BIN      := ec1

LEXER_TEST_BIN        := lexer_ev_tests
PARSER_TEST_BIN       := parser_ev_tests
PARSER_CMD_TEST_BIN   := parser_cmd_tests
SEMANTICA_CMD_TEST_BIN := semantica_cmd_tests
LEXER_FUN_TEST_BIN    := lexer_fun_tests
PARSER_FUN_TEST_BIN   := parser_fun_tests
SEMANTICA_FUN_TEST_BIN := semantica_fun_tests
CODEGEN_FUN_TEST_BIN  := codegen_fun_tests
ARRAY_TEST_BIN        := array_tests

$(BIN): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(BIN)

test: test-sin test-ec2 test-cod test-lex-ev test-parser-ev test-parser-cmd test-semantica-cmd test-cmd test-cod-ev test-lexer-fun test-parser-fun test-semantica-fun test-codegen-fun test-fun test-array test-array-e2e

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

test-semantica-cmd:
	$(CXX) $(CXXFLAGS) -Isrc tests/semantica_cmd_test.cpp src/lexer.cpp src/token.cpp src/parser.cpp src/ast.cpp src/semantica.cpp -o $(SEMANTICA_CMD_TEST_BIN)
	./$(SEMANTICA_CMD_TEST_BIN)

test-cmd: $(BIN)
	bash scripts/run_tests_semantica_cmd.sh

test-cod: $(BIN)
	bash scripts/run_tests_ativ06.sh

test-cod-ev: $(BIN)
	bash scripts/run_tests_ev.sh

test-lexer-fun:
	$(CXX) $(CXXFLAGS) -Isrc tests/lexer_fun_test.cpp src/lexer.cpp src/token.cpp -o $(LEXER_FUN_TEST_BIN)
	./$(LEXER_FUN_TEST_BIN)

test-parser-fun:
	$(CXX) $(CXXFLAGS) -Isrc tests/parser_fun_test.cpp src/lexer.cpp src/token.cpp src/parser.cpp src/ast.cpp src/semantica.cpp -o $(PARSER_FUN_TEST_BIN)
	./$(PARSER_FUN_TEST_BIN)

test-semantica-fun:
	$(CXX) $(CXXFLAGS) -Isrc tests/semantica_fun_test.cpp src/lexer.cpp src/token.cpp src/parser.cpp src/ast.cpp src/semantica.cpp -o $(SEMANTICA_FUN_TEST_BIN)
	./$(SEMANTICA_FUN_TEST_BIN)

test-codegen-fun:
	$(CXX) $(CXXFLAGS) -Isrc tests/codegen_fun_test.cpp src/lexer.cpp src/token.cpp src/parser.cpp src/ast.cpp src/semantica.cpp src/codegen.cpp -o $(CODEGEN_FUN_TEST_BIN)
	./$(CODEGEN_FUN_TEST_BIN)

test-fun: $(BIN)
	bash scripts/run_tests_fun.sh

test-array:
	$(CXX) $(CXXFLAGS) -Isrc tests/array_test.cpp src/lexer.cpp src/token.cpp src/parser.cpp src/ast.cpp src/semantica.cpp src/codegen.cpp -o $(ARRAY_TEST_BIN)
	./$(ARRAY_TEST_BIN)

test-array-e2e: $(BIN)
	bash scripts/run_tests_array.sh

clean:
	rm -f $(BIN) $(BIN).exe $(LEXER_TEST_BIN) $(LEXER_TEST_BIN).exe $(PARSER_TEST_BIN) $(PARSER_TEST_BIN).exe $(PARSER_CMD_TEST_BIN) $(PARSER_CMD_TEST_BIN).exe $(SEMANTICA_CMD_TEST_BIN) $(SEMANTICA_CMD_TEST_BIN).exe $(LEXER_FUN_TEST_BIN) $(LEXER_FUN_TEST_BIN).exe $(PARSER_FUN_TEST_BIN) $(PARSER_FUN_TEST_BIN).exe $(SEMANTICA_FUN_TEST_BIN) $(SEMANTICA_FUN_TEST_BIN).exe $(CODEGEN_FUN_TEST_BIN) $(CODEGEN_FUN_TEST_BIN).exe $(ARRAY_TEST_BIN) $(ARRAY_TEST_BIN).exe

.PHONY: test test-sin test-ec2 test-lex-ev test-parser-ev test-parser-cmd test-semantica-cmd test-cmd test-cod test-cod-ev test-lexer-fun test-parser-fun test-semantica-fun test-codegen-fun test-fun test-array test-array-e2e clean
