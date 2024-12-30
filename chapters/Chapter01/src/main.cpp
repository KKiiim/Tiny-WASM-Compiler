#include <cassert>

#include "parse.hpp"

int main(int argc, char *argv[]) {
  assert(argc == 2);
  Parser parser{argv[1]};

  parser.startCompilation();
}