CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra
SRC      := $(wildcard src/*.cpp)
BIN      := ec1

$(BIN): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(BIN)

# ---- testes ----------------------------------------------------------------
# test-sin : análise sintática e interpretação (Atividade 05)
# test-cod : geração de código assembly      (Atividade 06, requer Linux/as/ld)
test: test-sin

test-sin: $(BIN)
	bash scripts/run_tests.sh

test-cod: $(BIN)
	bash scripts/run_tests_ativ06.sh

clean:
	rm -f $(BIN) $(BIN).exe

.PHONY: test test-sin test-cod clean
