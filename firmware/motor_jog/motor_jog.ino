/* Single-axis supervised jog bring-up. No autonomous sweep or RF capture.
 * Z STEP D4, Z DIR D7, shared active-low ENABLE D8; encoder D2/D3.
 * Boots DISABLED. Hardware jumper setting must match CONFIG. USB opening resets.
 * A/B encoder must be coupled 1:1 to motor, with X/Y/A drivers absent.
 */
#include <Arduino.h>
#include <AccelStepper.h>
#include <util/atomic.h>
#include "motion_logic.h"
#include "quadrature.h"
#if !defined(__AVR_ATmega328P__)
#error ATmega328P Uno required.
#endif

const uint8_t STEP_PIN = 4, DIR_PIN = 7, ENABLE_PIN = 8;
AccelStepper motor(AccelStepper::DRIVER, STEP_PIN, DIR_PIN, 0, 0, false);
rig::Guard guard;
rig::LineInput input;
volatile int32_t rawCounts = 0;
volatile uint32_t badTransitions = 0;
volatile uint8_t previousAB;

uint8_t readAB() {
  uint8_t bits = PIND;
  return ((bits & _BV(PD2)) ? 2 : 0) | ((bits & _BV(PD3)) ? 1 : 0);
}
void encoderEdge() {
  uint8_t next = readAB();
  QuadratureChange change = decodeQuadrature(previousAB, next);
  rawCounts += change.delta;
  if (change.invalid) ++badTransitions;
  previousAB = next;
}
void snapshot(int32_t& counts, uint32_t& bad) {
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { counts = rawCounts; bad = badTransitions; }
}
void disableImmediately() {
  digitalWrite(ENABLE_PIN, HIGH);
  motor.setCurrentPosition(motor.currentPosition());  // Cancel target and velocity.
}
void status() {
  Serial.print(F("{\"ms\":")); Serial.print(millis());
  Serial.print(F(",\"state\":")); Serial.print((int)guard.phase);
  Serial.print(F(",\"armed\":")); Serial.print(guard.armed ? 1 : 0);
  Serial.print(F(",\"ref\":")); Serial.print(guard.referenced ? 1 : 0);
  Serial.print(F(",\"sign\":")); Serial.print((int)guard.sign);
  Serial.print(F(",\"microsteps\":")); Serial.print(guard.microsteps);
  Serial.print(F(",\"step\":")); Serial.print(guard.position);
  Serial.print(F(",\"target\":")); Serial.print(guard.target);
  Serial.print(F(",\"enc\":")); Serial.print(guard.relative);
  Serial.print(F(",\"bad\":")); Serial.print(guard.invalid);
  Serial.print(F(",\"fault\":")); Serial.print((int)guard.fault);
  Serial.print(F(",\"done\":")); Serial.print(guard.completed);
  Serial.println('}');
}
void dispatch(const char* line) {
  rig::Request request = rig::parse(line);
  int32_t counts; uint32_t bad; snapshot(counts, bad);
  const uint32_t now = millis();
  const char* error = NULL;
  switch (request.command) {
    case rig::PING: guard.ping(now); return;
    case rig::STATUS: status(); return;
    case rig::CONFIG:
      error = guard.configure(request.value);
      if (!error) { motor.setMaxSpeed(5.0f * guard.stepsPerDegree()); motor.setAcceleration(10.0f * guard.stepsPerDegree()); }
      break;
    case rig::ARM:
      error = guard.arm(now);
      if (!error) { motor.setCurrentPosition(0); digitalWrite(ENABLE_PIN, LOW); }
      break;
    case rig::ZERO:
      error = guard.zero(now, counts, bad);
      if (!error) motor.setCurrentPosition(0);
      break;
    case rig::PROBE: error = guard.probe(now); break;
    case rig::JOG: error = guard.jog(request.value, now); break;
    case rig::STOP: guard.stop(); disableImmediately(); break;
    case rig::CLEAR: error = guard.clear(); break;
    default:
      error = "invalid_command";
      if (guard.armed) { guard.stop(rig::BAD_FRAME); disableImmediately(); }
      break;
  }
  Serial.print(error ? F("{\"error\":\"") : F("{\"ok\":\""));
  if (error) { Serial.print(error); Serial.println(F("\"}")); }
  else { Serial.print((int)request.command); Serial.println(F("\"}")); }
}
void setup() {
  digitalWrite(ENABLE_PIN, HIGH); pinMode(ENABLE_PIN, OUTPUT);
  digitalWrite(STEP_PIN, LOW); pinMode(STEP_PIN, OUTPUT);
  digitalWrite(DIR_PIN, LOW); pinMode(DIR_PIN, OUTPUT);
  pinMode(2, INPUT_PULLUP); pinMode(3, INPUT_PULLUP);
  previousAB = readAB();
  attachInterrupt(digitalPinToInterrupt(2), encoderEdge, CHANGE);
  attachInterrupt(digitalPinToInterrupt(3), encoderEdge, CHANGE);
  motor.setMinPulseWidth(3);
  Serial.begin(115200);
  Serial.println(F("{\"boot\":\"motor_jog_v1_disabled\"}"));
}
void loop() {
  uint8_t budget = 32;
  while (budget-- && Serial.available()) {
    rig::LineInput::Result result = input.push((char)Serial.read());
    if (result == rig::LineInput::EMERGENCY) { guard.stop(); disableImmediately(); }
    else if (result == rig::LineInput::TOO_LONG) { guard.stop(rig::BAD_FRAME); disableImmediately(); }
    else if (result == rig::LineInput::READY) dispatch(input.text);
  }
  int32_t counts; uint32_t bad; snapshot(counts, bad);
  if (guard.armed && guard.phase == rig::MOVING) motor.moveTo(guard.target);
  guard.update(millis(), counts, bad, motor.currentPosition(), motor.distanceToGo() == 0);
  if (!guard.armed) disableImmediately();
  else if (guard.phase == rig::MOVING) { motor.moveTo(guard.target); motor.run(); }
  static uint32_t reported = 0;
  if ((uint32_t)(millis() - reported) >= 250) { reported = millis(); status(); }
}
