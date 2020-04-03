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
    int osc_tune[NUM_VOICES];
    int delay_feedback, delay_time, delay_wet_raw, mod_value, mod_value_raw;
    int delay_wet;
    int envBypass[NUM_VOICES];
    int envState[NUM_VOICES];
    int envStateCountdown[NUM_VOICES];
    int waveState[NUM_VOICES];
    int waveSpecialMode[NUM_VOICES];
    int waveSpecialModeCountdown[NUM_VOICES];
    int sequencerDataTime[SEQUENCER_MAX_STEPS];
    int sequencerDataTune[NUM_VOICES][SEQUENCER_MAX_STEPS];
    int sequencerCurrentStep, sequencerMaxSteps;
    int sequencerStepLen;
    int sequencerIntraStepCount;

    synthCtx()
    {
      int i, j;

      for (i = 0; i < NUM_VOICES; i++)
      {
        osc_slew[i] = OSC_WAVE_SLEW_LOW;
        osc[i].setSampleRate(SAMPLE_RATE);
        osc[i].setSlew(osc_slew[i]);
        env[i].setMax(ENV_MAX);
        env[i].setMin(ENV_MIN);
        env[i].setAttack(ENV_ATTACK);
        env[i].setRelease(ENV_RELEASE);
        envBypass[i] = 0;
        envState[i] = 0;
        envStateCountdown[i] = 0;
        waveSpecialMode[i] = 0;
        waveState[i] = 0;
        waveSpecialModeCountdown[i] = 0;
        tune_value[i] = 0;
        env_speed[i] = 1;
      }

      sequencerCurrentStep = sequencerMaxSteps = 0;
      sequencerStepLen = SEQUENCER_MIN_STEP_LEN;
      sequencerIntraStepCount = 0;
  
      for (i = 0; i < SEQUENCER_MAX_STEPS; i++)
      {
        sequencerDataTime[i] = 0;

        for (j = 0; j < NUM_VOICES; j++)
        {
          sequencerDataTune[j][i] = 0;
        }
      }

      dly.setSampleRate(SAMPLE_RATE);
      dly.setTime(delay_time = 0);
      dly.setFeedback(delay_feedback = 0);
      delay_wet = 0;
    }
};

#endif
