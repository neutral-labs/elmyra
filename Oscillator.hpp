#ifndef OSCILLATOR_HPP
#define OSCILLATOR_HPP

class Oscillator
{
  private:
    int index, state, frequency, halfCycleBase, halfCycleCurrent, sampleRate, slewRate, modAmount;
    
  public:
    Oscillator()
    {
      sampleRate = 1;
      index = 0;
      state = -AMP_MAX;
      frequency = 1;
      halfCycleBase = halfCycleCurrent = 1;
      slewRate = 0;
      modAmount = 0;
    }

    void setFreq(int mHz)
    {
      frequency = mHz;
      halfCycleBase = halfCycleCurrent = (1000 * sampleRate) / (frequency * 2);
    }

    void setSampleRate(int rate)
    {
      sampleRate = rate;
      setFreq(frequency);
    }

    void setModAmount(int mod)
    {
      modAmount = mod;
    }

    void setSlew(int slew)
    {
      slewRate = slew;
    }
  
    int getSample() 
    { 
      int maxOffset;
           
      if (index++ >= halfCycleCurrent)
      {  
        index = 0;
        state = -state;
        maxOffset = halfCycleBase / ((OSC_MOD_AMOUNT_MAX + 1) - modAmount);
        halfCycleCurrent = halfCycleBase - (maxOffset / 2) + random(0, maxOffset + 1);
      }

      if (state < 0)
      {
        return state + (slewRate * AMP_MAX * index) / (OSC_SLEW_SCALE * halfCycleCurrent);
      }
      else
      {
        return state - (slewRate * AMP_MAX * index) / (OSC_SLEW_SCALE * halfCycleCurrent);
      }
    }
};

#endif
