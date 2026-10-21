#include <cassert>
#include <iostream>
#include "../firmware/encoder_monitor/quadrature.h"

int main() {
  // An ideal 600-cycle forward revolution, then an exact reverse revolution.
  const uint8_t forward[] = {0, 1, 3, 2, 0};
  const uint8_t reverse[] = {0, 2, 3, 1, 0};
  int counts = 0;
  for (int cycle = 0; cycle < 600; ++cycle)
    for (int edge = 1; edge < 5; ++edge) {
      auto step = decodeQuadrature(forward[edge - 1], forward[edge]);
      assert(!step.invalid);
      counts += step.delta;
    }
  assert(counts == 2400);
  for (int cycle = 0; cycle < 600; ++cycle)
    for (int edge = 1; edge < 5; ++edge) {
      auto step = decodeQuadrature(reverse[edge - 1], reverse[edge]);
      assert(!step.invalid);
      counts += step.delta;
    }
  assert(counts == 0);
  for (uint8_t state = 0; state < 4; ++state) {
    auto repeated = decodeQuadrature(state, state);
    assert(repeated.delta == 0 && !repeated.invalid);
    auto skipped = decodeQuadrature(state, state ^ 3);
    assert(skipped.delta == 0 && skipped.invalid);
  }
  // One-edge bounce should not introduce a net position change.
  assert(decodeQuadrature(0, 1).delta + decodeQuadrature(1, 0).delta == 0);
  std::cout << "Quadrature revolution, reversal, bounce and invalid-state checks passed\n";
}
