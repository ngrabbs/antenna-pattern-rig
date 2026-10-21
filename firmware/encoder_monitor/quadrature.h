#ifndef ANTENNA_QUADRATURE_H
#define ANTENNA_QUADRATURE_H
#include <stdint.h>

// State bits: A is bit 1, B is bit 0. Sign is a convention, not physical CW.
// Repeated states count zero; simultaneous two-bit jumps are invalid.
struct QuadratureChange { int8_t delta; bool invalid; };
inline QuadratureChange decodeQuadrature(uint8_t previous, uint8_t current) {
  static const int8_t delta[16] = {
     0,  1, -1,  0,
    -1,  0,  0,  1,
     1,  0,  0, -1,
     0, -1,  1,  0
  };
  return {delta[(previous << 2) | current], (previous ^ current) == 3};
}
#endif
