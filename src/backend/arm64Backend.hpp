#ifndef SRC_BACKEND_ARM64BACKEND_H
#define SRC_BACKEND_ARM64BACKEND_H

#include "emit.hpp"
#include "localManager.hpp"

class Arm64Backend {
public:
  Arm64Backend();

public:
  Emit emit{};
  LM lm{};
};

#endif