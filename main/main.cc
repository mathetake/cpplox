#include <fstream>
#include <iostream>
#include <string>

#include "chunk.hpp"
#include "common.hpp"
#include "debug.hpp"
#include "vm.hpp"

void repl() {
  std::string line;
  printf("> ");
  while (std::getline(std::cin, line)) {
    vm.interpret(line.c_str());
    printf("> ");
  }
}

void runFile(const char* path) {
  std::ifstream ifs(path);
  std::string content((std::istreambuf_iterator<char>(ifs)),
                      (std::istreambuf_iterator<char>()));
  vm.interpret(content.c_str());
}

int main(int argc, char* argv[]) {
  vm.initVM();

  if (argc == 1) {
    repl();
  } else if (argc == 2) {
    runFile(argv[1]);
  } else {
    std::cout << "Usage: clox [path]\n";
    exit(64);
  }
};
