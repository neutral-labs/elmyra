#ifndef SYNTHCTX_HPP
#define SYNTHCTX_HPP

#include "synthCtx.hpp"
#include "Delay.hpp"
#include "slewEnvelope.hpp"
#include "Oscillator.hpp"

class synthCtx
{
  public:
    Oscillator osc[NUM_VOICES];
    Delay dly;
    slewEnvelope env[NUM_VOICES];
    int touch_value[NUM_VOICES];
    int env_led_level[NUM_VOICES];
    int env_value[NUM_VOICES];
    int tune_value[NUM_VOICES];
    int env_speed[NUM_VOICES];
    int osc_slew[NUM_VOICES];
    int delay_feedback, delay_time, delay_wet_raw, mod_value, mod_value_raw;   
    int envBypass[NUM_VOICES];
    int envState[NUM_VOICES];
    int envStateCountdown[NUM_VOICES];
    int delay_wet;
};

#endif
