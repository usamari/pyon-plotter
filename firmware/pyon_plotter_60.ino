// Pyon plotter 60 firmware
// Copyright (c) 2026 USAMARI
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.
//
// Third-party components, not written by the copyright holder above:
//   AccelStepper, Copyright (C) Mike McCauley, GPL
//   https://www.airspayce.com/mikem/arduino/AccelStepper/
//   Arduino Servo library, LGPL-2.1
//   https://github.com/arduino-libraries/Servo

#include <AccelStepper.h>
#include <MultiStepper.h>
#include <Servo.h>
#include <avr/pgmspace.h>
#include <math.h>

// 青 Abort: テスト描画 / blue: test draw
const int BTN_ABORT  = A0;
// 黄 Hold: 本番描画 / yellow: artwork
const int BTN_HOLD   = A1;
// 赤 Resume: ペンと停止 / red: pen and stop
const int BTN_RESUME = A2;

const int PIN_A_STEP = 2;
const int PIN_A_DIR  = 5;
const int PIN_B_STEP = 3;
const int PIN_B_DIR  = 6;
const int PIN_EN     = 8;
const int PIN_SERVO  = 11;

const float STEPS_PER_MM = 50.0;
const float MAX_SPEED    = 500.0;
const float ACCEL        = 5000.0;
const float MAX_X        = 60.0;  // 可動 60mm角 / 60 mm square
const float MAX_Y        = 60.0;
const float PARK_X       = 0.0;
const float PARK_Y       = 60.0;

const int SERVO_UP   = 80;
const int SERVO_DOWN = 130;
const int SERVO_WAIT = 400;
const int LONG_MS    = 800;  // 長押し判定 / long-press threshold

AccelStepper motorA(AccelStepper::DRIVER, PIN_A_STEP, PIN_A_DIR);
AccelStepper motorB(AccelStepper::DRIVER, PIN_B_STEP, PIN_B_DIR);
MultiStepper steppers;
Servo pen;

enum State { IDLE, DRAWING };
State state = IDLE;

bool lastAbort = HIGH, lastHold = HIGH, lastResume = HIGH;
unsigned long stopLowSince = 0;

// ============================================================
// 絵柄1: 黄ボタン短押し / artwork 1: yellow short press
// 下の R"NC( と )NC" の間だけを差し替える。引用符は消さない。
// Replace only the lines between R"NC( and )NC". Keep the quotes.
// ============================================================
const char pattern1_nc[] PROGMEM = R"NC(
)NC";

// ============================================================
// 絵柄2: 黄ボタン長押し / artwork 2: yellow long press
// 下の R"NC( と )NC" の間だけを差し替える。引用符は消さない。
// Replace only the lines between R"NC( and )NC". Keep the quotes.
// ============================================================
const char pattern2_nc[] PROGMEM = R"NC(
)NC";

void motorsEnable(bool on) {
  digitalWrite(PIN_EN, on ? LOW : HIGH);
}

void setPenUp() {
  pen.attach(PIN_SERVO);
  pen.write(SERVO_UP);
  delay(SERVO_WAIT);
}

void setPenDown() {
  pen.attach(PIN_SERVO);
  pen.write(SERVO_DOWN);
  delay(SERVO_WAIT);
}

// 押した瞬間だけ true / true on press edge
bool clicked(int pin, bool &last) {
  bool now = digitalRead(pin);
  bool hit = (last == HIGH && now == LOW);
  last = now;
  return hit;
}

// 離すか2秒まで測る / hold time until release or 2 s
int holdTime(int pin) {
  unsigned long t0 = millis();
  while (digitalRead(pin) == LOW) {
    if (millis() - t0 > 2000) break;
  }
  delay(30);
  return (int)(millis() - t0);
}

// 描画中の赤短押し / red button held while drawing
bool stopHeld() {
  if (digitalRead(BTN_RESUME) != LOW) {
    stopLowSince = 0;
    return false;
  }
  if (stopLowSince == 0) stopLowSince = millis();
  return (millis() - stopLowSince) >= 100;
}

void xyToMotors(float x, float y, long pos[2]) {
  pos[0] = lround((x + y) * STEPS_PER_MM);
  pos[1] = lround((x - y) * STEPS_PER_MM);
}

bool moveToXY(float x, float y) {
  if (x < 0) x = 0;
  if (y < 0) y = 0;
  if (x > MAX_X) x = MAX_X;
  if (y > MAX_Y) y = MAX_Y;
  long pos[2];
  xyToMotors(x, y, pos);
  steppers.moveTo(pos);
  while (steppers.run()) {
    if (stopHeld()) return false;
  }
  return true;
}

void doEstop() {
  setPenUp();
  motorsEnable(false);
  motorA.stop();
  motorB.stop();
  state = IDLE;
}

bool beginDraw() {
  state = DRAWING;
  motorsEnable(true);
  setPenUp();
  motorA.setCurrentPosition(0);
  motorB.setCurrentPosition(0);
  stopLowSince = 0;
  return true;
}

bool endDraw() {
  setPenUp();
  delay(200);
  if (!moveToXY(PARK_X, PARK_Y)) {
    doEstop();
    return false;
  }
  motorsEnable(false);
  state = IDLE;
  return true;
}

// 中央55mm正方形 / 55 mm square, centered
bool drawSquare55() {
  beginDraw();
  const float a = 2.5f;   // (60-55)/2
  const float b = 57.5f;
  if (!moveToXY(a, a)) { doEstop(); return false; }
  setPenDown();
  if (!moveToXY(b, a) || !moveToXY(b, b) ||
      !moveToXY(a, b) || !moveToXY(a, a)) {
    doEstop();
    return false;
  }
  return endDraw();
}

// 直径25mm円、24辺 / 25 mm circle, 24 chords
bool drawCircle25() {
  beginDraw();
  const float cx = 30.0f, cy = 30.0f, r = 12.5f;
  const int n = 24;
  if (!moveToXY(cx + r, cy)) { doEstop(); return false; }
  setPenDown();
  for (int i = 1; i <= n; i++) {
    float a = 2.0f * PI * i / n;
    if (!moveToXY(cx + r * cos(a), cy + r * sin(a))) {
      doEstop();
      return false;
    }
  }
  return endDraw();
}

bool readLine_P(const char *src, int &idx, char *buf, int buflen) {
  int n = 0;
  while (true) {
    char c = pgm_read_byte(src + idx);
    if (c == 0) {
      buf[n] = 0;
      return n > 0;
    }
    idx++;
    if (c == '\r') continue;
    if (c == '\n') break;
    if (n < buflen - 1) buf[n++] = c;
  }
  buf[n] = 0;
  return true;
}

bool parseNumber(const char *s, int &i, float &out) {
  while (s[i] == ' ') i++;
  char *end;
  out = strtod(s + i, &end);
  if (end == s + i) return false;
  i = end - s;
  return true;
}

bool runGCode_P(const char *src) {
  if (!src || !pgm_read_byte(src)) return true;
  beginDraw();
  float x = 0, y = 0;
  int idx = 0;
  char line[96];

  while (readLine_P(src, idx, line, sizeof(line))) {
    if (stopHeld()) { doEstop(); return false; }
    char *p = line;
    while (*p == ' ' || *p == '\t') p++;
    if (*p == 0 || *p == ';' || *p == '(') continue;

    int g = -1, m = -1, s = -1;
    bool hasX = false, hasY = false;
    float nx = x, ny = y, f = -1;
    int i = 0;
    while (p[i]) {
      char c = p[i];
      if (c == ';' || c == '(') break;
      if (c == ' ' || c == '\t') { i++; continue; }
      if (c >= 'a' && c <= 'z') c -= 32;
      i++;
      float v;
      if (!parseNumber(p, i, v)) break;
      if      (c == 'G') g = (int)v;
      else if (c == 'M') m = (int)v;
      else if (c == 'X') { nx = v; hasX = true; }
      else if (c == 'Y') { ny = v; hasY = true; }
      else if (c == 'S') s = (int)v;
      else if (c == 'F') f = v;
    }
    if (m == 5) setPenUp();
    if (m == 3 || m == 4) {
      if (s <= 0) setPenUp();
      else setPenDown();
    }
    if (f >= 1500) setPenUp();
    if (f > 0 && f < 1500) setPenDown();
    if ((g == 0 || g == 1 || hasX || hasY) && (hasX || hasY)) {
      if (hasX) x = nx;
      if (hasY) y = ny;
      if (!moveToXY(x, y)) { doEstop(); return false; }
    }
  }
  return endDraw();
}

void setup() {
  pinMode(PIN_EN, OUTPUT);
  motorsEnable(false);
  pinMode(BTN_ABORT, INPUT_PULLUP);
  pinMode(BTN_HOLD, INPUT_PULLUP);
  pinMode(BTN_RESUME, INPUT_PULLUP);
  lastAbort  = digitalRead(BTN_ABORT);
  lastHold   = digitalRead(BTN_HOLD);
  lastResume = digitalRead(BTN_RESUME);

  motorA.setMaxSpeed(MAX_SPEED);
  motorB.setMaxSpeed(MAX_SPEED);
  motorA.setAcceleration(ACCEL);
  motorB.setAcceleration(ACCEL);
  steppers.addStepper(motorA);
  steppers.addStepper(motorB);

  setPenUp();
  motorA.setCurrentPosition(0);
  motorB.setCurrentPosition(0);
  state = IDLE;
}

void loop() {
  // 赤: 短押し=上げ（描画中は停止）、長押し=下げ
  // red: short = pen up (estop if drawing), long = pen down
  if (clicked(BTN_RESUME, lastResume)) {
    if (holdTime(BTN_RESUME) >= LONG_MS) setPenDown();
    else if (state == DRAWING) doEstop();
    else setPenUp();
    return;
  }
  if (state != IDLE) return;

  // 青: 短押し=正方形、長押し=円 / blue: short square, long circle
  if (clicked(BTN_ABORT, lastAbort)) {
    if (holdTime(BTN_ABORT) >= LONG_MS) drawCircle25();
    else drawSquare55();
    return;
  }
  // 黄: 短押し=絵柄1、長押し=絵柄2 / yellow: short art1, long art2
  if (clicked(BTN_HOLD, lastHold)) {
    runGCode_P(holdTime(BTN_HOLD) >= LONG_MS ? pattern2_nc : pattern1_nc);
  }
}
