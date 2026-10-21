#include <cassert>
#include <cmath>
#include <iostream>
#include "../firmware/motor_jog/motion_logic.h"
using namespace rig;

void readyFeedback(Guard& g, uint32_t now=700, int micro=1) {
  assert(!g.configure(micro)); assert(!g.arm(now-600));
  g.ping(now);g.update(now,0,0,0,true);assert(!g.zero(now,0,0));
}

enum Plant { NORMAL, ASYMMETRIC, RELAXATION, FROZEN, REVERSED_RETURN, OSCILLATING, JITTER };
Guard simulate(double gain, int polarity, Plant plant=NORMAL, uint32_t now=700) {
  Guard g;readyFeedback(g,now);assert(!g.feedback(now));
  int32_t motor=0,raw=0;double encoder=0;uint32_t lastPulse=now;
  bool sawOut=false;int iterations=0;
  uint8_t driftLeg=0;uint32_t nearTime=0;
  while (g.armed && ++iterations<6000) {
    now+=50;
    if (g.phase==MOVING && motor!=g.target && (uint32_t)(now-lastPulse)>=360) {
      int direction=g.target>motor?1:-1;motor+=direction;lastPulse=now;
      if (!(plant==FROZEN && g.feedbackSign)) {
        double effectiveGain=(plant==ASYMMETRIC && g.sweepLeg==2)?12.2:gain;
        encoder+=direction*effectiveGain*((plant==REVERSED_RETURN && g.sweepLeg==2)?-1:1);
        raw=(int32_t)lround(encoder)*polarity;
      }
      if (plant==OSCILLATING) raw=motor*20+7;
    }
    int32_t input=raw;
    if (plant==RELAXATION && g.phase==SETTLING && g.feedbackSign &&
        std::abs(g.encoderGoal-raw*g.feedbackSign)<=6 && driftLeg!=g.sweepLeg) {
      if (!nearTime) nearTime=now;
      if ((uint32_t)(now-nearTime)>=1000) {
        encoder+=7;raw=(int32_t)lround(encoder)*polarity;input=raw;
        driftLeg=g.sweepLeg;nearTime=0;
      }
    }
    if (plant==JITTER && g.phase==SETTLING) input+=iterations%2;
    g.ping(now);g.update(now,input,0,motor,motor==g.target);
    if (g.sweepLeg==2 && !sawOut) {
      sawOut=true;assert(std::abs(g.sweepOutCounts*g.feedbackSign+1200)<=6);
      assert(g.encoderGoal==0 && g.referenced);
    }
  }
  assert(iterations<6000 && !g.armed);
  if ((plant==NORMAL || plant==ASYMMETRIC || plant==RELAXATION) && gain>=10) {
    assert(g.fault==OK && g.sweepLeg==3 && g.completed==1 && sawOut);
    assert(g.feedbackSign==polarity && std::abs(g.sweepHomeCounts)<=6);
    assert(!g.referenced && !g.sign);
  }
  return g;
}

int main() {
  assert(parse("FEEDBACK180").command==FEEDBACK180);
  assert(parse("FEEDBACK180 1").command==INVALID);
  for (double gain : {10.0,10.3,11.6,12.0,13.0}) for (int polarity : {-1,1})
    simulate(gain,polarity);
  simulate(12,1,NORMAL,0xffff0000u);  // Timing remains bounded across millis rollover.
  assert(simulate(11.6,1,ASYMMETRIC).position!=0);  // Encoder home, not motor step home.
  simulate(12,1,RELAXATION);simulate(12,-1,RELAXATION);
  assert(simulate(0,1).fault==NO_FEEDBACK);
  assert(simulate(12,1,FROZEN).fault==NO_FEEDBACK);
  assert(simulate(12,1,REVERSED_RETURN).fault==WRONG_DIRECTION);
  assert(simulate(20,1,OSCILLATING).fault==CORRECTION_LIMIT);
  assert(simulate(8,1).fault==TRAVEL);
  assert(simulate(12,1,JITTER).fault==SETTLE_TIMEOUT);
  { Guard g;assert(g.feedback(0));readyFeedback(g,700,8);assert(g.feedback(700)); }
  { Guard g;readyFeedback(g);assert(!g.feedback(700));assert(g.feedback(701));assert(g.jog(1,701));
    g.update(2801,0,0,0,false);assert(!g.armed && g.fault==COMM_TIMEOUT); }
  { Guard g;readyFeedback(g);assert(!g.feedback(700));g.ping(800);g.update(800,0,1,0,false);
    assert(!g.armed && g.fault==BAD_ENCODER); }
  { Guard g;readyFeedback(g);assert(!g.feedback(700));g.ping(800);g.update(800,1401,0,0,false);
    assert(!g.armed && g.fault==TRAVEL); }
  { Guard g;readyFeedback(g);assert(!g.feedback(700));g.ping(90701);g.update(90701,0,0,0,false);
    assert(!g.armed && g.fault==MOVE_TIMEOUT); }
  { Guard g;readyFeedback(g);assert(!g.feedback(700));
    g.ping(800);g.update(800,0,0,-70,false);
    g.ping(900);g.update(900,0,0,5,false);
    assert(!g.armed && g.fault==TRAVEL); }
  { Guard g;readyFeedback(g);assert(!g.feedback(700));g.stop();g.ping(5000);g.update(5000,0,0,0,true);
    assert(!g.armed && g.completed==0 && !g.referenced); }
  std::cout << "Encoder-targeted positioning: both polarities, variable gain, rollover, convergence, stalls, correction limits, bounds, STOP and faults passed\n";
}
