#ifndef ANTENNA_MOTION_LOGIC_H
#define ANTENNA_MOTION_LOGIC_H
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

namespace rig {
enum Command { INVALID, CONFIG, ARM, ZERO, PROBE, JOG, STOP, CLEAR, STATUS, PING, TICK, BELTTEST, SWEEP180, FEEDBACK180 };
struct Request { Command command; double value; };
inline Request parse(const char* input) {
  char text[64];
  if (strlen(input) >= sizeof(text)) return {INVALID, 0};
  strcpy(text, input);
  char* word = strtok(text, " \t");
  if (!word) return {INVALID, 0};
  Command c = INVALID;
  const char* names[] = {"CONFIG", "ARM", "ZERO", "PROBE", "JOG", "STOP", "CLEAR", "STATUS", "PING", "TICK", "BELTTEST", "SWEEP180", "FEEDBACK180"};
  const Command codes[] = {CONFIG, ARM, ZERO, PROBE, JOG, STOP, CLEAR, STATUS, PING, TICK, BELTTEST, SWEEP180, FEEDBACK180};
  for (uint8_t i = 0; i < 13; ++i) if (!strcmp(word, names[i])) c = codes[i];
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
enum Fault { OK, COMM_TIMEOUT, FOLLOWING, BAD_ENCODER, TRAVEL, SETTLE_TIMEOUT, MOVE_TIMEOUT, BAD_FRAME,
             NO_FEEDBACK, CORRECTION_LIMIT, WRONG_DIRECTION };

class Guard {
 public:
  Phase phase = DISABLED;
  Fault fault = OK;
  uint8_t microsteps = 0;
  bool armed = false, referenced = false;
  int8_t sign = 0;  // Raw encoder polarity relative to commanded positive steps.
  int32_t position = 0, target = 0, offset = 0, relative = 0;
  uint32_t invalid = 0, completed = 0;
  uint8_t sweepLeg = 0;  // 0 inactive, 1 outward, 2 return, 3 completed.
  int32_t sweepOutCounts = 0, sweepHomeCounts = 0;
  int32_t sweepOutSteps = 0;
  int8_t feedbackSign = 0;
  int32_t encoderGoal = 0;
  uint16_t feedbackSteps = 0;
  uint8_t feedbackCorrections = 0;

  float stepsPerDegree() const { return 200.0f * microsteps / 360.0f; }
  uint8_t diagnosticMode() const { return feedbackTest ? 4 : (sweepTest ? 3 : (beltTest ? 2 : (singleStepTest ? 1 : 0))); }
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
    singleStepTest = beltTest = sweepTest = feedbackTest = false;
    sweepLeg = 0; sweepOutCounts = sweepHomeCounts = sweepOutSteps = 0;
    feedbackSign = 0; encoderGoal = 0; feedbackSteps = feedbackCorrections = 0;
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
    probing = true; singleStepTest = beltTest = sweepTest = feedbackTest = false;
    return begin(5L * microsteps, now);  // Exactly 9 nominal degrees at any subdivision.
  }
  const char* tick(uint32_t now) {
    if (!armed || !referenced || phase != HOLDING || sign || position != 0) return "zero_before_tick";
    if (microsteps != 1) return "tick_requires_full_steps";
    probing = true; singleStepTest = true; beltTest = sweepTest = feedbackTest = false;
    return begin(1, now);  // One full step; observe for one second, then disable.
  }
  const char* belt(uint32_t now) {
    if (!armed || !referenced || phase != HOLDING || sign || position != 0) return "zero_before_belttest";
    if (microsteps != 1) return "belttest_requires_full_steps";
    probing = true; singleStepTest = sweepTest = feedbackTest = false; beltTest = true;
    return begin(20, now);  // Fixed 36-degree motor command; encoder ratio unqualified.
  }
  const char* sweep(uint32_t now) {
    if (!armed || !referenced || phase != HOLDING || sign || position != 0) return "zero_before_sweep";
    if (microsteps != 1) return "sweep_requires_full_steps";
    probing = true; singleStepTest = beltTest = feedbackTest = false; sweepTest = true;
    sweepLeg = 1; sweepOutCounts = sweepHomeCounts = sweepOutSteps = 0;
    return begin(-100, now);  // 180 nominal degrees outward, then return to step zero.
  }
  const char* feedback(uint32_t now) {
    if (!armed || !referenced || phase != HOLDING || sign || position != 0) return "zero_before_feedback";
    if (microsteps != 1) return "feedback_requires_full_steps";
    singleStepTest = beltTest = sweepTest = false; feedbackTest = true;
    sweepLeg = 1; sweepOutCounts = sweepHomeCounts = sweepOutSteps = 0;
    feedbackSign = 0; encoderGoal = -1200; feedbackSteps = feedbackCorrections = 0;
    fbLastPosition = fbBurstPosition = position;
    fbBurstOrigin = relative; fbBurstChecked = false; fbAtTarget = false;
    fbNoProgressSteps = 0; fbLegStarted = now;
    return begin(-5, now);  // Learn polarity from a bounded five-step outward burst.
  }
  const char* jog(double degrees, uint32_t now) {
    if (!armed || !referenced || !sign || phase != HOLDING) return "probe_first_or_busy";
    if (!isfinite(degrees) || fabs(degrees) > 9.0 || degrees == 0) return "jog_range";
    int32_t delta = (int32_t)lround(degrees * stepsPerDegree());
    if (!delta) return "below_step_resolution";
    int32_t next = position + delta;
    if (fabs(next / stepsPerDegree()) > 45.001f) return "target_limit";
    probing = false; singleStepTest = beltTest = sweepTest = feedbackTest = false;
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
    const float encoderLimit = (sweepTest || feedbackTest) ? 210.0f : 50.0f;
    if (fabs(relative * 0.15f) > encoderLimit) { stop(TRAVEL); return; }
    if (sweepTest && (position < -100 || position > 0)) { stop(TRAVEL); return; }
    if (feedbackTest) { updateFeedback(now, motorDone); return; }
    if (phase == MOVING || phase == SETTLING) {
      float actual = probing ? fabs((float)relative) : relative * sign;
      float expected = probing ? fabs(expectedCounts(position)) : expectedCounts(position);
      if (!beltTest && !sweepTest && fabs(actual - expected) > 24.0f) { stop(FOLLOWING); return; }
      const uint32_t movementTimeout = sweepTest ? 45000 : 15000;
      if ((uint32_t)(now - moveTime) > movementTimeout) { stop(MOVE_TIMEOUT); return; }
      if (phase == MOVING && motorDone) { phase = SETTLING; settleTime = now; }
      if (phase == SETTLING) {
        const uint32_t settleTimeout = sweepTest ? 4000 : 1500;
        if ((uint32_t)(now - settleTime) > settleTimeout) { stop(SETTLE_TIMEOUT); return; }
        const uint32_t observationTime = sweepTest ? 2000 : ((singleStepTest || beltTest) ? 1000 : 250);
        if ((uint32_t)(now - settleTime) >= observationTime && (uint32_t)(now - lastChange) >= 250) {
          if (sweepTest) {
            if (sweepLeg == 1) {
              sweepOutCounts = relative; sweepOutSteps = position;
              sweepLeg = 2; begin(0, now); return;
            }
            sweepHomeCounts = relative; sweepLeg = 3; ++completed; stop(); return;
          }
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
  bool sweepTest = false;
  bool feedbackTest = false;
  bool fbBurstChecked = false, fbAtTarget = false;
  uint8_t fbNoProgressSteps = 0;
  int32_t fbLastPosition = 0, fbBurstPosition = 0, fbBurstOrigin = 0;
  uint32_t fbLegStarted = 0, fbBurstEnded = 0, fbAtTargetSince = 0;
  uint32_t heartbeat = 0, armTime = 0, lastChange = 0, referenceInvalid = 0;
  uint32_t moveTime = 0, settleTime = 0;
  int32_t previousRaw = 0;
  void feedbackMove(uint32_t now) {
    const int32_t error = encoderGoal - relative * feedbackSign;
    int32_t count = labs(error) / 12;
    if (count < 1) count = 1;
    if (count > 5) count = 5;
    if (labs(error) <= 24) {
      count = 1;
      if (++feedbackCorrections > 8) { stop(CORRECTION_LIMIT); return; }
    }
    int32_t next = position + (error > 0 ? count : -count);
    if (next < -125 || next > 5 || feedbackSteps + count > 140) { stop(TRAVEL); return; }
    fbBurstPosition = position; fbBurstOrigin = relative;
    fbBurstChecked = false; fbAtTarget = false;
    begin(next, now);
  }
  void updateFeedback(uint32_t now, bool motorDone) {
    const int32_t moved = labs(position - fbLastPosition);
    if (moved > 140 || feedbackSteps + moved > 140 || position < -125 || position > 5) {
      stop(TRAVEL); return;
    }
    feedbackSteps += moved; fbLastPosition = position;
    if ((uint32_t)(now - fbLegStarted) > 90000) { stop(MOVE_TIMEOUT); return; }
    if (phase == MOVING) {
      if (!motorDone) return;
      phase = SETTLING; fbBurstEnded = now;
    }
    if (phase != SETTLING) return;
    if ((uint32_t)(now - fbBurstEnded) > 6000) { stop(SETTLE_TIMEOUT); return; }
    if ((uint32_t)(now - fbBurstEnded) < 350 || (uint32_t)(now - lastChange) < 250) return;
    if (!feedbackSign) {
      if (labs(relative) < 12) { stop(NO_FEEDBACK); return; }
      feedbackSign = relative < 0 ? 1 : -1;
    }
    if (!fbBurstChecked) {
      const int32_t observed = (relative - fbBurstOrigin) * feedbackSign;
      const int32_t command = position - fbBurstPosition;
      if ((command > 0 && observed < -12) || (command < 0 && observed > 12)) {
        stop(WRONG_DIRECTION); return;
      }
      if (labs(observed) < 2) fbNoProgressSteps += labs(command);
      else fbNoProgressSteps = 0;
      if (fbNoProgressSteps >= 10) { stop(NO_FEEDBACK); return; }
      fbBurstChecked = true;
    }
    const int32_t error = encoderGoal - relative * feedbackSign;
    if (labs(error) > 6) { fbAtTarget = false; feedbackMove(now); return; }
    if (!fbAtTarget) { fbAtTarget = true; fbAtTargetSince = now; }
    if ((uint32_t)(now - fbAtTargetSince) < 2000) return;
    if (sweepLeg == 1) {
      sweepOutCounts = relative; sweepOutSteps = position; sweepLeg = 2; encoderGoal = 0;
      feedbackSteps = feedbackCorrections = fbNoProgressSteps = 0;
      fbLegStarted = now; fbLastPosition = position;
      feedbackMove(now); return;
    }
    sweepHomeCounts = relative; sweepLeg = 3; ++completed; stop();
  }
  const char* begin(int32_t next, uint32_t now) {
    target = next; moveTime = now; phase = MOVING; return NULL;
  }
};
}
#endif
