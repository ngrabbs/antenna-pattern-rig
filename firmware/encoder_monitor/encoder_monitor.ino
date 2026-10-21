/*
 * Uno ATmega328P only. Diagnostic firmware; NO motor motion or RF capture.
 * Uploading replaces installed rotor firmware: back it up first.
 * D2/D3 also feed X/Y STEP on a common CNC Shield V3: remove X/Y/A drivers,
 * remove clone jumpers and disconnect motor power before wiring/testing.
 * Verify 5 V NPN open-collector encoder output, with external pull-ups to 5 V.
 */
#include <Arduino.h>
#include <util/atomic.h>
#include "quadrature.h"

#if !defined(__AVR_ATmega328P__)
#error This sketch uses ATmega328P Uno registers and is not portable as written.
#endif

const uint8_t ENCODER_A = 2;
const uint8_t ENCODER_B = 3;
const uint8_t DRIVER_ENABLE = 8;
const float COUNTS_PER_ENCODER_REV = 2400.0f;  // Verify on the actual encoder.
const float ENCODER_REVS_PER_TABLE_REV = 1.0f;  // Proposed direct drive.

volatile int32_t encoderCounts = 0;
volatile uint32_t invalidTransitions = 0;
volatile uint8_t previousState;
uint32_t referenceId = 0;

uint8_t readEncoderState() {
  const uint8_t pins = PIND;  // One snapshot of both inputs.
  return ((pins & _BV(PD2)) ? 2 : 0) | ((pins & _BV(PD3)) ? 1 : 0);
}

void onEncoderEdge() {
  const uint8_t current = readEncoderState();
  const QuadratureChange change = decodeQuadrature(previousState, current);
  encoderCounts += change.delta;
  if (change.invalid) ++invalidTransitions;
  previousState = current;  // Resynchronize after an invalid transition.
}

void setup() {
  digitalWrite(DRIVER_ENABLE, HIGH);  // Preload disabled level before output mode.
  pinMode(DRIVER_ENABLE, OUTPUT);
  pinMode(ENCODER_A, INPUT_PULLUP);
  pinMode(ENCODER_B, INPUT_PULLUP);
  previousState = readEncoderState();
  attachInterrupt(digitalPinToInterrupt(ENCODER_A), onEncoderEdge, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_B), onEncoderEdge, CHANGE);
  Serial.begin(115200);
  Serial.println(F("# encoder diagnostic only; no absolute position reference"));
  Serial.println(F("# z=new manual relative zero; reference_id changes; invalid count stays cumulative"));
  Serial.println(F("millis,reference_id,counts,relative_table_deg,invalid_transitions"));
}

void loop() {
  while (Serial.available()) {
    if (Serial.read() == 'z') {
      ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        encoderCounts = 0;
        previousState = readEncoderState();
      }
      ++referenceId;
      Serial.println(F("# new manual relative zero; not homed"));
    }
  }
  static uint32_t lastReport = 0;
  const uint32_t now = millis();
  if ((uint32_t)(now - lastReport) < 100) return;
  lastReport = now;
  int32_t counts;
  uint32_t invalid;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    counts = encoderCounts;
    invalid = invalidTransitions;
  }
  const float degrees = counts * 360.0f /
    (COUNTS_PER_ENCODER_REV * ENCODER_REVS_PER_TABLE_REV);
  Serial.print(now);
  Serial.print(','); Serial.print(referenceId);
  Serial.print(','); Serial.print(counts);
  Serial.print(','); Serial.print(degrees, 4);
  Serial.print(','); Serial.println(invalid);
}
