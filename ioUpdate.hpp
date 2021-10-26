#ifndef IOUPDATE_HPP
#define IOUPDATE_HPP

#include "synthCtx.hpp"

#define ENV_BYPASS (-1)

static inline int getValueFromTouchSensor(int input, int *oldValue, int smoothing_factor)
{
  int newValue = analogRead(input);

  *oldValue = (*oldValue * (smoothing_factor - 1) + newValue) / smoothing_factor;

  return *oldValue;
}

static inline int getValueFromPot(int input, int *oldValue, int dead_zone, int smoothing_factor)
{
  int newValue = analogRead(input);

  if (newValue < *oldValue - dead_zone || newValue > *oldValue + dead_zone)
  {
    *oldValue = (*oldValue * (smoothing_factor - 1) + newValue) / smoothing_factor;
  }

  return *oldValue;
}

static inline int getValueFromPotTune(int input, int *oldValue, int dead_zone, int smoothing_factor)
{
  int newValue = analogRead(input);
  int factor;

  if (newValue < *oldValue - 200 || newValue > *oldValue + 200)
  {
    factor = 5;
  }
  else
  {
    factor = 2000 / abs(*oldValue - newValue) + 2;
  }

  *oldValue = (*oldValue * (factor - 1) + newValue) / factor;

  return *oldValue;
}

static inline int scalePotValue(int potValue, int scale)
{
  return (potValue * scale) / POT_MAX;
}

static inline void updateInputTouch(synthCtx *ctx)
{
  int i;
  int old_touch_value_0 = ctx->touch_value[0];
  
  // temporarily swap touch sensor pins to fix hardware problem
  ctx->touch_value[0] = getValueFromTouchSensor(PIN_IN_GSR_1, &ctx->touch_value[0], SMOOTHING_FACTOR_TOUCH);
  ctx->touch_value[1] = getValueFromTouchSensor(PIN_IN_GSR_2, &ctx->touch_value[1], SMOOTHING_FACTOR_TOUCH);
  ctx->touch_value[2] = getValueFromTouchSensor(PIN_IN_GSR_3, &ctx->touch_value[2], SMOOTHING_FACTOR_TOUCH);

  if (ctx->waveSpecialMode[SPECIAL_MODE_SEQUENCER_RECORD_NUM])
  {
    if (old_touch_value_0 >= ENV_MIN && ctx->touch_value[0] < ENV_MIN)
    {
      ctx->sequencerDataTime[ctx->sequencerCurrentStep] = ctx->delay_time;

      for (i = 0; i < NUM_VOICES; i++)
      {
        if (ctx->env_speed[i] != 1)
        {
          ctx->sequencerDataTune[i][ctx->sequencerCurrentStep] = ctx->osc_tune[i];
        }
        else
        {
          ctx->sequencerDataTune[i][ctx->sequencerCurrentStep] = 0;
        }
      }
      
      ctx->sequencerCurrentStep++;

      if (ctx->sequencerCurrentStep >= SEQUENCER_MAX_STEPS)
      {
        ctx->sequencerCurrentStep = 0;
        ctx->waveSpecialMode[SPECIAL_MODE_SEQUENCER_RECORD_NUM] = 0;
      }
    }
  }

#ifdef SERIAL_DEBUG_TOUCH
  Serial.print("val1: ");
  Serial.print(ctx->touch_value[0]);
  Serial.print("\tval2: ");
  Serial.print(ctx->touch_value[1]);
  Serial.print("\tval3: ");
  Serial.println(ctx->touch_value[2]);
#endif
}

static inline int updateSingleEnvSpeed(synthCtx *ctx, int voice, int input)
{
  int stateChanged;
  int state = ((digitalRead(input) == HIGH) ? 1 : ENV_SPEED_FACTOR);

  if (ctx->envStateCountdown[voice] > 0)
  {
    ctx->envStateCountdown[voice]--;
  }
  
  if (ctx->envState[voice] != state)
  {
    stateChanged = 1;
    ctx->envState[voice] = state;
    ctx->envBypass[voice] = 0;
  }
  else
  {
    stateChanged = 0;
  }
  
  if (stateChanged)
  {
    if (ctx->envStateCountdown[voice] > 0)
    {
      ctx->envBypass[voice] = 1;
      ctx->envStateCountdown[voice] = 0;
      return ENV_BYPASS;
    }
    else
    {
      ctx->envStateCountdown[voice] = SPECIAL_MODE_ENABLE_TIME * IO_UPDATE_FREQ;
    }
  }
  else
  {
    if (ctx->envBypass[voice])
    {
      return ENV_BYPASS;
    }
  }

  ctx->envBypass[voice] = 0;
  return state; 
}

static inline void updateInputEnvSpeed(synthCtx *ctx)
{
  ctx->env_speed[0] = updateSingleEnvSpeed(ctx, 0, PIN_IN_ENV_1);
  ctx->env_speed[1] = updateSingleEnvSpeed(ctx, 1, PIN_IN_ENV_2);
  ctx->env_speed[2] = updateSingleEnvSpeed(ctx, 2, PIN_IN_ENV_3);
}

static inline int updateSingleOscWave(synthCtx *ctx, int voice, int input)
{
  int stateChanged;
  int state = ((digitalRead(input) == HIGH) ? OSC_WAVE_SLEW_LOW : OSC_WAVE_SLEW_HIGH);
  
  if (ctx->waveSpecialModeCountdown[voice] > 0)
  {
    ctx->waveSpecialModeCountdown[voice]--;
  }

  if (ctx->waveState[voice] != state)
  {
    stateChanged = 1;
    ctx->waveState[voice] = state;
  }
  else
  {
    stateChanged = 0;
  }
  
  if (stateChanged)
  {
    if (ctx->waveSpecialModeCountdown[voice] > 0)
    {
      if (ctx->waveSpecialMode[voice] == 0)
      {
        ctx->waveSpecialMode[voice] = 1;
      }
      else
      {
        ctx->waveSpecialMode[voice] = 0;
      }
      
      ctx->waveSpecialModeCountdown[voice] = 0;
      return state;
    }
    else
    {
      ctx->waveSpecialModeCountdown[voice] = SPECIAL_MODE_ENABLE_TIME * IO_UPDATE_FREQ;
    }
  }
  
  return state;
}

static inline void updateInputOscWave(synthCtx *ctx)
{
  int sequencerRunning = ctx->waveSpecialMode[SPECIAL_MODE_SEQUENCER_RUN_NUM];
  int sequencerRecording = ctx->waveSpecialMode[SPECIAL_MODE_SEQUENCER_RECORD_NUM];
  
  ctx->osc_slew[0] = updateSingleOscWave(ctx, 0, PIN_IN_WAVE_1);
  ctx->osc_slew[1] = updateSingleOscWave(ctx, 1, PIN_IN_WAVE_2);
  ctx->osc_slew[2] = updateSingleOscWave(ctx, 2, PIN_IN_WAVE_3);

  if (sequencerRunning && ctx->waveSpecialMode[SPECIAL_MODE_SEQUENCER_RECORD_NUM])
  {
    ctx->waveSpecialMode[SPECIAL_MODE_SEQUENCER_RUN_NUM] = 0;
    ctx->sequencerCurrentStep = 0;
    return;
  }

  if (sequencerRecording && ctx->waveSpecialMode[SPECIAL_MODE_SEQUENCER_RUN_NUM])
  {
    ctx->waveSpecialMode[SPECIAL_MODE_SEQUENCER_RECORD_NUM] = 0;
    ctx->sequencerMaxSteps = ctx->sequencerCurrentStep;
    ctx->sequencerCurrentStep = 0;
    return;
  }

  if (sequencerRecording && !ctx->waveSpecialMode[SPECIAL_MODE_SEQUENCER_RECORD_NUM])
  {
    ctx->sequencerMaxSteps = ctx->sequencerCurrentStep;
    ctx->sequencerCurrentStep = 0;
    return;
  }
}

static inline void updateInputOscTune(synthCtx *ctx)
{
  if (ctx->waveSpecialMode[SPECIAL_MODE_SCALE_NUM])
  {
    ctx->osc_tune[0] = cMinorScale[(getValueFromPotTune(PIN_IN_TUNE_1, &ctx->tune_value[0], POT_DEAD_ZONE_TUNE, SMOOTHING_FACTOR_TUNE) * MINOR_SCALE_MAX) / POT_MAX];
    ctx->osc_tune[1] = cMinorScale[(getValueFromPotTune(PIN_IN_TUNE_2, &ctx->tune_value[1], POT_DEAD_ZONE_TUNE, SMOOTHING_FACTOR_TUNE) * MINOR_SCALE_MAX) / POT_MAX];
    ctx->osc_tune[2] = cMinorScale[(getValueFromPotTune(PIN_IN_TUNE_3, &ctx->tune_value[2], POT_DEAD_ZONE_TUNE, SMOOTHING_FACTOR_TUNE) * MINOR_SCALE_MAX) / POT_MAX];
  }
  else
  {
    ctx->osc_tune[0] = POT_TUNE_BASE + getValueFromPotTune(PIN_IN_TUNE_1, &ctx->tune_value[0], POT_DEAD_ZONE_TUNE, SMOOTHING_FACTOR_TUNE) * 100;
    ctx->osc_tune[1] = POT_TUNE_BASE + getValueFromPotTune(PIN_IN_TUNE_2, &ctx->tune_value[1], POT_DEAD_ZONE_TUNE, SMOOTHING_FACTOR_TUNE) * 100;
    ctx->osc_tune[2] = POT_TUNE_BASE + getValueFromPotTune(PIN_IN_TUNE_3, &ctx->tune_value[2], POT_DEAD_ZONE_TUNE, SMOOTHING_FACTOR_TUNE) * 100;
  }

  ctx->mod_value = scalePotValue(getValueFromPot(PIN_IN_MOD, &ctx->mod_value_raw, POT_DEAD_ZONE_NORMAL, SMOOTHING_FACTOR_NORMAL), OSC_MOD_AMOUNT_MAX);
}

static inline void updateOutputEnvLED(synthCtx *ctx)
{
  digitalWrite(PIN_OUT_ENV_1, ctx->env_value[0] ? HIGH : LOW);
  digitalWrite(PIN_OUT_ENV_2, ctx->env_value[1] ? HIGH : LOW);
  digitalWrite(PIN_OUT_ENV_3, ctx->env_value[2] ? HIGH : LOW);
}

static inline void updateInputDelay(synthCtx *ctx)
{
  int new_time;

  if (ctx->waveSpecialMode[SPECIAL_MODE_SEQUENCER_RUN_NUM])
  {
    ctx->sequencerStepLen = SEQUENCER_MIN_STEP_LEN + ((SEQUENCER_MAX_STEP_LEN * getValueFromPot(PIN_IN_DELAY_TIME, &ctx->delay_time, POT_DEAD_ZONE_NORMAL, SMOOTHING_FACTOR_NORMAL)) / POT_MAX);
    Serial.println(ctx->sequencerStepLen);
  }
  else
  {
    new_time = getValueFromPot(PIN_IN_DELAY_TIME, &ctx->delay_time, POT_DEAD_ZONE_NORMAL, SMOOTHING_FACTOR_NORMAL);

    if (new_time < DELAY_TIME_FINETUNE_POINT)
    {
      new_time /= DELAY_TIME_FINETUNE_FACTOR;
    }
    else
    {
      new_time = DELAY_POT_SCALE_TIME + (new_time - DELAY_TIME_FINETUNE_POINT);
    }
  
    new_time = (DELAY_POT_SCALE_TIME * new_time) / (POT_MAX - (DELAY_TIME_FINETUNE_POINT / DELAY_TIME_FINETUNE_FACTOR));
  
    ctx->dly.setTime(new_time);
  }
  
  ctx->dly.setFeedback(scalePotValue(getValueFromPot(PIN_IN_DELAY_FEEDBACK, &ctx->delay_feedback, POT_DEAD_ZONE_NORMAL, SMOOTHING_FACTOR_NORMAL), DELAY_POT_SCALE_FEEDBACK));
  ctx->delay_wet = scalePotValue(getValueFromPot(PIN_IN_DELAY_MIX, &ctx->delay_wet_raw, POT_DEAD_ZONE_NORMAL, SMOOTHING_FACTOR_NORMAL), DELAY_POT_SCALE_MIX);
}

static inline void updateFromSequencer(synthCtx *ctx)
{
  int i;
  
  if (ctx->waveSpecialMode[SPECIAL_MODE_SEQUENCER_RUN_NUM])
  {
    ctx->dly.setTime(ctx->sequencerDataTime[ctx->sequencerCurrentStep]);

    for (i = 0; i < NUM_VOICES; i++)
    {
      if (ctx->sequencerDataTune[i][ctx->sequencerCurrentStep] != 0)
      {
        ctx->osc_tune[i] = ctx->sequencerDataTune[i][ctx->sequencerCurrentStep];
      }
    }
    
    if (++ctx->sequencerIntraStepCount >= ctx->sequencerStepLen)
    {
      ctx->sequencerIntraStepCount = 0;
      ctx->sequencerCurrentStep++;

      if (ctx->sequencerCurrentStep >= ctx->sequencerMaxSteps)
      {
        ctx->sequencerCurrentStep = 0;
      }
    }
  }
}

static inline void updateVoices(synthCtx *ctx)
{
  int i;
  
  for (i = 0; i < NUM_VOICES; i++)
  {
    ctx->osc[i].setSlew(ctx->osc_slew[i]);
    ctx->osc[i].setFreq(ctx->osc_tune[i]);
    ctx->osc[i].setModAmount(ctx->mod_value);
    
    if (ctx->env_speed[i] == ENV_BYPASS)
    {
      ctx->env_value[i] = AMP_MAX;
    }
    else
    {
      ctx->env[i].setAttack(ENV_ATTACK * ctx->env_speed[i]);
      ctx->env[i].setRelease(ENV_RELEASE * ctx->env_speed[i]);
      ctx->env_value[i] = ctx->env[i].getLevel(ctx->touch_value[i]);  
    }
  }
}

void ioUpdate(synthCtx *ctx)
{
  updateInputTouch(ctx);
  updateInputEnvSpeed(ctx);
  updateInputOscWave(ctx);
  updateInputOscTune(ctx);
  updateInputDelay(ctx);
  updateFromSequencer(ctx);
  updateVoices(ctx);
  updateOutputEnvLED(ctx);
}

#endif
