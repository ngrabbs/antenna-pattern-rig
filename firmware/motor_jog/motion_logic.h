#ifndef ANTENNA_MOTION_LOGIC_H
#define ANTENNA_MOTION_LOGIC_H
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

namespace rig {
enum Command { INVALID, CONFIG, ARM, ZERO, PROBE, JOG, STOP, CLEAR, STATUS, PING, TICK, BELTTEST };
struct Request { Command command; double value; };
inline Request parse(const char* input) {
  char text[64];
  if (strlen(input) >= sizeof(text)) return {INVALID, 0};
  strcpy(text, input);
  char* word = strtok(text, " \t");
  if (!word) return {INVALID, 0};
  Command c = INVALID;
  const char* names[] = {"CONFIG", "ARM", "ZERO", "PROBE", "JOG", "STOP", "CLEAR", "STATUS", "PING", "TICK", "BELTTEST"};
  const Command codes[] = {CONFIG, ARM, ZERO, PROBE, JOG, STOP, CLEAR, STATUS, PING, TICK, BELTTEST};
  for (uint8_t i = 0; i < 11; ++i) if (!strcmp(word, names[i])) c = codes[i];
  char* arg = strtok(NULL, " \t");
  if (c == CONFIG || c == JOG) {
    if (!arg || strtok(NULL, " \t")) return {INVALID, 0};
    char* end;
    double number = strtod(arg, &end);
    if (*end || end == arg || !isfinite(number)) return {INVALID, 0};
    return {c, number};
  }
  return arg ? Request{INVALID, 0} : Request{c, 0};
}

class LineInput {
 public:
  enum Result { NONE, READY, TOO_LONG, EMERGENCY };
  char text[64] = {};
  Result push(char ch) {
    if (ch == '!') { length = 0; discard = true; return EMERGENCY; }
    if (ch == '\n' || ch == '\r') {
      if (discard) { discard = false; length = 0; return NONE; }
      if (!length) return NONE;
      text[length] = 0; length = 0; return READY;
    }
    if (discard) return NONE;
    if (length >= sizeof(text) - 1) { discard = true; length = 0; return TOO_LONG; }
    text[length++] = ch;
    return NONE;
  }
 private:
  uint8_t length = 0;
  bool discard = false;
};

enum Phase { DISABLED, HOLDING, MOVING, SETTLING, FAULT };
enum Fault { OK, COMM_TIMEOUT, FOLLOWING, BAD_ENCODER, TRAVEL, SETTLE_TIMEOUT, MOVE_TIMEOUT, BAD_FRAME };

class Guard {
 public:
  Phase phase = DISABLED;
  Fault fault = OK;
  uint8_t microsteps = 0;
  bool armed = false, referenced = false;
  int8_t sign = 0;  // Raw encoder polarity relative to commanded positive steps.
  int32_t position = 0, target = 0, offset = 0, relative = 0;
  uint32_t invalid = 0, completed = 0;

  float stepsPerDegree() const { return 200.0f * microsteps / 360.0f; }
  uint8_t diagnosticMode() const { return beltTest ? 2 : (singleStepTest ? 1 : 0); }
  float expectedCounts(int32_t steps) const { return steps * 12.0f / microsteps; }
  void ping(uint32_t now) { heartbeat = now; }
  const char* configure(double value) {
    if (armed) return "disarm_first";
    if (!(value == 1 || value == 2 || value == 4 || value == 8 || value == 16 || value == 32)) return "bad_microsteps";
    microsteps = (uint8_t)value; referenced = false; sign = 0; return NULL;
  }
  const char* arm(uint32_t now) {
    if (armed || phase == FAULT) return "not_disarmed";
    if (!microsteps) return "configure_first";
    armed = true; referenced = false; sign = 0; position = target = 0;
    singleStepTest = beltTest = false;
    armTime = heartbeat = now; phase = HOLDING; return NULL;
  }
  const char* zero(uint32_t now, int32_t raw, uint32_t bad) {
    if (!armed || phase != HOLDING) return "not_holding";
    if ((uint32_t)(now - armTime) < 500 || (uint32_t)(now - lastChange) < 250) return "not_settled";
    offset = raw; relative = 0; previousRaw = raw; invalid = bad;
    referenceInvalid = bad; position = target = 0; sign = 0;
    referenced = true; return NULL;
  }
  const char* probe(uint32_t now) {
    if (!armed || !referenced || phase != HOLDING || sign || position != 0) return "zero_before_probe";
    probing = true; singleStepTest = beltTest = false;
    return begin(5L * microsteps, now);  // Exactly 9 nominal degrees at any subdivision.
  }
  const char* tick(uint32_t now) {
    if (!armed || !referenced || phase != HOLDING || sign || position != 0) return "zero_before_tick";
    if (microsteps != 1) return "tick_requires_full_steps";
    probing = true; singleStepTest = true; beltTest = false;
    return begin(1, now);  // One full step; observe for one second, then disable.
  }
  const char* belt(uint32_t now) {
    if (!armed || !referenced || phase != HOLDING || sign || position != 0) return "zero_before_belttest";
    if (microsteps != 1) return "belttest_requires_full_steps";
    probing = true; singleStepTest = false; beltTest = true;
    return begin(20, now);  // Fixed 36-degree motor command; encoder ratio unqualified.
  }
  const char* jog(double degrees, uint32_t now) {
    if (!armed || !referenced || !sign || phase != HOLDING) return "probe_first_or_busy";
    if (!isfinite(degrees) || fabs(degrees) > 9.0 || degrees == 0) return "jog_range";
    int32_t delta = (int32_t)lround(degrees * stepsPerDegree());
    if (!delta) return "below_step_resolution";
    int32_t next = position + delta;
    if (fabs(next / stepsPerDegree()) > 45.001f) return "target_limit";
    probing = false; singleStepTest = beltTest = false;
    return begin(next, now);
  }
  void stop(Fault reason = OK) {
    armed = false; referenced = false; sign = 0; target = position;
    if (reason != OK) fault = reason;
    phase = fault == OK ? DISABLED : FAULT;
  }
  const char* clear() {
    if (armed) return "disarm_first";
    fault = OK; phase = DISABLED; referenced = false; sign = 0; return NULL;
  }
  void update(uint32_t now, int32_t raw, uint32_t bad, int32_t steps, bool motorDone) {
    if (raw != previousRaw) { lastChange = now; previousRaw = raw; }
    relative = raw - offset; invalid = bad; position = steps;
    if (!armed) return;
    if ((uint32_t)(now - heartbeat) > 2000) { stop(COMM_TIMEOUT); return; }
    if (!referenced) return;
    if (bad != referenceInvalid) { stop(BAD_ENCODER); return; }
    if (fabs(relative * 0.15f) > 50.0f) { stop(TRAVEL); return; }
    if (phase == MOVING || phase == SETTLING) {
      float actual = probing ? fabs((float)relative) : relative * sign;
      float expected = probing ? fabs(expectedCounts(position)) : expectedCounts(position);
      if (!beltTest && fabs(actual - expected) > 24.0f) { stop(FOLLOWING); return; }
      if ((uint32_t)(now - moveTime) > 15000) { stop(MOVE_TIMEOUT); return; }
      if (phase == MOVING && motorDone) { phase = SETTLING; settleTime = now; }
      if (phase == SETTLING) {
        if ((uint32_t)(now - settleTime) > 1500) { stop(SETTLE_TIMEOUT); return; }
        const uint32_t observationTime = (singleStepTest || beltTest) ? 1000 : 250;
        if ((uint32_t)(now - settleTime) >= observationTime && (uint32_t)(now - lastChange) >= 250) {
          if (beltTest) { ++completed; stop(); return; }  // Completion is not a tracking pass.
          if (fabs(actual - expectedCounts(target)) > 6.0f) { stop(FOLLOWING); return; }
          if (singleStepTest) { ++completed; stop(); return; }
          if (probing) sign = relative > 0 ? 1 : -1;
          phase = HOLDING; ++completed;
        }
      }
    } else if (sign && fabs(relative * sign - expectedCounts(position)) > 6.0f) {
      stop(FOLLOWING);
    }
  }
 private:
  bool probing = false;
  bool singleStepTest = false;
  bool beltTest = false;
  uint32_t heartbeat = 0, armTime = 0, lastChange = 0, referenceInvalid = 0;
  uint32_t moveTime = 0, settleTime = 0;
  int32_t previousRaw = 0;
  const char* begin(int32_t next, uint32_t now) {
    target = next; moveTime = now; phase = MOVING; return NULL;
  }
};
}
#endif
