#ifndef SRC_BACKEND_ARM64BACKEND_H
#define SRC_BACKEND_ARM64BACKEND_H

#include "emit.hpp"

class Arm64Backend {
public:
  Arm64Backend();

public:
  Emit emit;
};

#endif