CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra
SRC      := $(wildcard src/*.cpp)
BIN      := ec1
LEXER_TEST_BIN := lexer_ev_tests

$(BIN): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(BIN)

# ---- testes ----------------------------------------------------------------
# test-sin : análise sintática e interpretação (Atividade 05)
# test-ec2 : precedência e associatividade  (Atividade 07)
# test-cod : geração de código assembly      (Atividade 06, requer Linux/as/ld)
test: test-sin test-ec2 test-lex-ev

test-sin: $(BIN)
	bash scripts/run_tests.sh

test-ec2: $(BIN)
	bash scripts/run_tests_ec2.sh

test-lex-ev:
	$(CXX) $(CXXFLAGS) -Isrc tests/lexer_ev_test.cpp src/lexer.cpp src/token.cpp -o $(LEXER_TEST_BIN)
	./$(LEXER_TEST_BIN)

test-cod: $(BIN)
	bash scripts/run_tests_ativ06.sh

clean:
	rm -f $(BIN) $(BIN).exe $(LEXER_TEST_BIN) $(LEXER_TEST_BIN).exe

.PHONY: test test-sin test-ec2 test-lex-ev test-cod clean
