#include <cassert>
#include <cmath>
#include <iostream>
#include "../firmware/motor_jog/motion_logic.h"
using namespace rig;

void ready(Guard& g, int micro = 1) {
  assert(!g.armed && g.phase == DISABLED);
  assert(g.arm(0));  // Missing explicit configuration.
  assert(!g.configure(micro));
  assert(!g.arm(100));
  assert(g.zero(200, 0, 0));  // Reject zero before energization has settled.
  g.ping(700); g.update(700, 0, 0, 0, true);
  assert(!g.zero(700, 0, 0));
}
void finish(Guard& g, uint32_t& now, int sign) {
  const int direction = g.target > g.position ? 1 : -1;
  while (g.position != g.target) {
    int step = g.position + direction;
    int raw = (int)lround(g.expectedCounts(step)) * sign;
    now += 50; g.ping(now);
    g.update(now, raw, 0, step, step == g.target);
    assert(g.armed && g.fault == OK);
  }
  now += 300; g.ping(now);
  g.update(now, (int)lround(g.expectedCounts(g.position)) * sign, 0, g.position, true);
  assert(g.phase == HOLDING);
}
int main() {
  for (const char* bad : {"", "ARM extra", "CONFIG", "CONFIG 1 2", "JOG nan", "JOG inf", "JOG 1x", "STOPlater"})
    assert(parse(bad).command == INVALID);
  assert(parse("CONFIG 8").value == 8);
  assert(parse("JOG -9").value == -9);
  assert(parse("STOP").command == STOP);
  assert(parse("TICK").command == TICK);
  assert(parse("TICK 1").command == INVALID);
  assert(parse("BELTTEST").command == BELTTEST);
  assert(parse("BELTTEST 50").command == INVALID);
  assert(parse("SWEEP180").command == SWEEP180);
  assert(parse("SWEEP180 2").command == INVALID);
  for (int polarity : {-1,0,1}) for (int residual : {0,30}) {
    Guard g; ready(g); assert(!g.sweep(700));
    assert(g.target==-100 && g.sweepLeg==1 && g.diagnosticMode()==3);
    uint32_t now=700;
    for (int step=-1;step>=-100;--step) {
      now+=360;g.ping(now);g.update(now,step*12*polarity,0,step,step==-100);
      assert(g.armed && g.fault==OK && g.sweepLeg==1);
    }
    now+=1999;g.ping(now);g.update(now,-1200*polarity,0,-100,true);
    assert(g.sweepLeg==1 && g.armed);
    ++now;g.ping(now);g.update(now,-1200*polarity,0,-100,true);
    assert(g.sweepLeg==2 && g.target==0 && g.sweepOutCounts==-1200*polarity);
    for (int step=-99;step<=0;++step) {
      now+=360;g.ping(now);
      int raw=step*12*polarity+residual*(step+100)/100;
      g.update(now,raw,0,step,step==0);
      assert(g.armed && g.fault==OK && g.sweepLeg==2);
    }
    now+=1999;g.ping(now);g.update(now,residual,0,0,true);assert(g.armed);
    ++now;g.ping(now);g.update(now,residual,0,0,true);
    assert(!g.armed && g.sweepLeg==3 && g.position==0 && g.completed==1);
    assert(g.sweepHomeCounts==residual && !g.referenced && !g.sign);
    assert(g.sweep(now+1));
  }
  { Guard g; assert(g.sweep(0)); ready(g,8); assert(g.sweep(700)); }
  { Guard g; ready(g); assert(!g.sweep(700));g.ping(800);g.update(800,1401,0,-1,false);
    assert(!g.armed && g.fault==TRAVEL); }
  { Guard g; ready(g); assert(!g.sweep(700));g.ping(800);g.update(800,0,0,-101,false);
    assert(!g.armed && g.fault==TRAVEL); }
  { Guard g; ready(g); assert(!g.sweep(700));g.ping(45701);g.update(45701,0,0,-99,false);
    assert(!g.armed && g.fault==MOVE_TIMEOUT); }
  { Guard g; ready(g); assert(!g.sweep(700));g.ping(1000);g.update(1000,-1200,0,-100,true);
    g.ping(5001);g.update(5001,-1200,0,-100,true);
    assert(!g.armed && g.fault==SETTLE_TIMEOUT && g.sweepLeg==1); }
  { Guard g; ready(g); assert(!g.sweep(700));g.update(2801,0,0,-1,false);
    assert(!g.armed && g.fault==COMM_TIMEOUT); }
  { Guard g; ready(g); assert(!g.sweep(700));g.ping(800);g.update(800,0,1,-1,false);
    assert(!g.armed && g.fault==BAD_ENCODER); }
  { Guard g; ready(g); assert(!g.sweep(700));g.stop();
    g.ping(5000);g.update(5000,0,0,0,true);
    assert(!g.armed && g.phase==DISABLED && g.sweepLeg==1 && g.completed==0); }
  for (int observed : {0, 1, -1}) {
    Guard g; ready(g); assert(!g.belt(700)); assert(g.target == 20 && g.diagnosticMode() == 2);
    for (int step=1; step<=20; ++step) {
      uint32_t now=700+step*400; g.ping(now);
      g.update(now, step*10*observed, 0, step, step==20);
      assert(g.armed && g.fault==OK);
    }
    g.ping(9699); g.update(9699, 200*observed, 0, 20, true); assert(g.armed);
    g.ping(9700); g.update(9700, 200*observed, 0, 20, true);
    assert(!g.armed && g.phase==DISABLED && g.completed==1 && !g.sign && !g.referenced);
    assert(g.belt(9701));
  }
  { Guard g; assert(g.belt(0)); ready(g,8); assert(g.belt(700)); }
  { Guard g; ready(g); assert(!g.belt(700)); g.ping(800); g.update(800,334,0,1,false);
    assert(!g.armed && g.fault==TRAVEL); }
  { Guard g; ready(g); assert(!g.belt(700)); g.update(2801,0,0,1,false);
    assert(!g.armed && g.fault==COMM_TIMEOUT); }
  { Guard g; ready(g); assert(!g.belt(700)); g.ping(800); g.update(800,0,1,1,false);
    assert(!g.armed && g.fault==BAD_ENCODER); }
  { Guard g; ready(g); assert(!g.belt(700)); g.ping(15701); g.update(15701,0,0,1,false);
    assert(!g.armed && g.fault==MOVE_TIMEOUT); }
  { Guard g; ready(g); assert(!g.belt(700)); g.stop();
    assert(!g.armed && g.phase==DISABLED && g.belt(701)); }
  for (int polarity : {-1, 1}) {
    Guard g; ready(g); assert(!g.tick(700)); assert(g.target == 1);
    g.ping(1000); g.update(1000, 0, 0, 1, true);
    assert(g.phase == SETTLING && g.armed);
    g.ping(1500); g.update(1500, 12 * polarity, 0, 1, true);
    assert(g.armed && g.completed == 0);
    g.ping(1999); g.update(1999, 12 * polarity, 0, 1, true);
    assert(g.armed);
    g.ping(2000); g.update(2000, 12 * polarity, 0, 1, true);
    assert(!g.armed && g.phase == DISABLED && g.completed == 1 && !g.referenced && !g.sign);
    assert(g.tick(2001));  // Never repeat automatically or reuse a reference.
  }
  { Guard g; ready(g); assert(!g.tick(700));
    g.ping(1000); g.update(1000, 0, 0, 1, true);
    g.ping(1999); g.update(1999, 0, 0, 1, true); assert(g.armed);
    g.ping(2000); g.update(2000, 0, 0, 1, true);
    assert(!g.armed && g.fault == FOLLOWING && g.completed == 0); }
  { Guard g; assert(g.tick(0)); ready(g, 8); assert(g.tick(700)); }
  { Guard g; ready(g); assert(!g.tick(700));
    g.update(2801, 0, 0, 1, true); assert(!g.armed && g.fault == COMM_TIMEOUT); }
  { Guard g; ready(g); assert(!g.tick(700));
    g.ping(800); g.update(800, 0, 1, 1, true); assert(!g.armed && g.fault == BAD_ENCODER); }
  for (int polarity : {-1, 1}) for (int subdivision : {1, 8, 32}) {
    Guard g; ready(g, subdivision);
    assert(g.jog(9, 700));  // A successful direction probe is required.
    assert(!g.probe(700));
    uint32_t now = 700; finish(g, now, polarity);
    assert(g.sign == polarity && g.completed == 1);
    assert(g.jog(10, now));
    assert(!g.jog(-9, now)); finish(g, now, polarity);
    assert(g.position == 0 && g.relative == 0 && g.completed == 2);
    for (int i=0; i<5; ++i) { assert(!g.jog(9, now)); finish(g, now, polarity); }
    assert(g.jog(9, now));  // Commanded travel beyond +45 degrees rejected.
    assert(g.configure(1));
    g.stop(); assert(!g.armed && !g.referenced && g.phase == DISABLED);
    assert(g.jog(1, now));
  }
  { Guard g; ready(g); assert(!g.probe(700)); g.ping(800);
    g.update(800,0,0,3,false); assert(g.fault==FOLLOWING && !g.armed); }
  { Guard g; ready(g); g.ping(800); g.update(800,0,1,0,true);
    assert(g.fault==BAD_ENCODER && !g.armed); }
  { Guard g; ready(g); g.update(2801,0,0,0,true);
    assert(g.fault==COMM_TIMEOUT && !g.armed);
    g.stop(); assert(g.fault==COMM_TIMEOUT && g.phase==FAULT);
    assert(g.arm(3000)); assert(!g.clear()); assert(!g.arm(3000)); }
  { Guard g; ready(g); g.ping(800); g.update(800,334,0,0,true);
    assert(g.fault==TRAVEL && !g.armed); }
  { Guard g; ready(g); assert(!g.probe(700)); g.ping(1000);
    g.update(1000,55,0,5,true); g.ping(1300); g.update(1300,50,0,5,true);
    g.ping(1600); g.update(1600,50,0,5,true);
    assert(g.fault==FOLLOWING && !g.armed); }
  { Guard g; ready(g); assert(!g.probe(700));
    g.ping(15701); g.update(15701,0,0,0,false);
    assert(g.fault==MOVE_TIMEOUT && !g.armed); }
  { Guard g; ready(g); assert(!g.probe(700)); g.ping(1000);
    g.update(1000,60,0,5,true);
    for (uint32_t now=1100;now<=2500;now+=100) {
      g.ping(now); g.update(now,now%200==0?59:60,0,5,true);
    }
    g.ping(2601); g.update(2601,60,0,5,true);
    assert(g.fault==SETTLE_TIMEOUT && !g.armed); }
  { Guard g; assert(!g.configure(1)); assert(!g.arm(0xfffffff0));
    g.update(0x10,0,0,0,true); assert(g.armed); }
  { Guard g; assert(g.configure(3)); assert(g.configure(1.5)); }
  LineInput line;
  for (int i=0;i<63;++i) assert(line.push('x') == LineInput::NONE);
  assert(line.push('x') == LineInput::TOO_LONG);
  assert(line.push('A') == LineInput::NONE);
  assert(line.push('\n') == LineInput::NONE);
  assert(line.push('!') == LineInput::EMERGENCY);
  assert(line.push('A') == LineInput::NONE);
  assert(line.push('\n') == LineInput::NONE);
  for (char c : {'S','T','O','P'}) assert(line.push(c) == LineInput::NONE);
  assert(line.push('\n') == LineInput::READY);
  assert(parse(line.text).command == STOP);
  std::cout << "Motion planning, both polarities, subdivisions, faults, watchdog, framing and stop checks passed\n";
}
