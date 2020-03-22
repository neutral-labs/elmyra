#ifndef DELAY_HPP
#define DELAY_HPP

class Delay
{
  private:
    short buf[DELAY_BUF_SIZE];
    int delayFeedback, delayTimeSamplesNew, delayTimeSamplesCurrent, sampleRate, slewCount, slewCountMax;
    int idx_read, idx_write;

    inline void wrap(int *p)
    {
      if (*p == DELAY_BUF_SIZE)
      {
        *p = 0;
      }
    }

    inline void adjustTime()
    {
      if (delayTimeSamplesNew == delayTimeSamplesCurrent)
      {
        return;
      }
      else if (delayTimeSamplesNew > delayTimeSamplesCurrent)
      {
        delayTimeSamplesCurrent++;
      }
      else
      {
        delayTimeSamplesCurrent--;
      }

      slewCountMax = DELAY_TIME_SLEW / abs(delayTimeSamplesNew - delayTimeSamplesCurrent);
    }
    
  public:
    Delay()
    {
      int i;
      
      idx_read = 0;
      idx_write = 0;
      delayFeedback = 0;
      delayTimeSamplesNew = 1;
      delayTimeSamplesCurrent = 1;
      slewCount = 0;

      for (i = 0; i < DELAY_BUF_SIZE; i++)
      {
        buf[i] = BIAS;
      }
    }

    void setSampleRate(int rate)
    {
      sampleRate = rate;
    }

    void setTime(int milliSeconds)
    {
      delayTimeSamplesNew = (milliSeconds * sampleRate) / 1000;

      if (delayTimeSamplesNew == delayTimeSamplesCurrent)
      {
        return;
      }

      if (delayTimeSamplesNew > DELAY_BUF_SIZE - 1)
      {
        delayTimeSamplesNew = DELAY_BUF_SIZE - 1;
      }
      else if (delayTimeSamplesNew < 1)
      {
        delayTimeSamplesNew = 1;
      }
      
      if (idx_write - delayTimeSamplesCurrent >= 0)
      {
        idx_read = idx_write - delayTimeSamplesCurrent;
      }
      else
      {
        idx_read = DELAY_BUF_SIZE + (idx_write - delayTimeSamplesCurrent);
      }

      slewCountMax = DELAY_TIME_SLEW / abs(delayTimeSamplesNew - delayTimeSamplesCurrent);
    }

    void setFeedback(int feedback)
    {
      delayFeedback = feedback;
    }
  
    int getSample(int inSample)
    {
      int outSample;

      if (delayTimeSamplesNew != delayTimeSamplesCurrent)
      {
        if (slewCount++ >= slewCountMax)
        {
          slewCount = 0;
          adjustTime();
        }
      }

      outSample = buf[idx_read++];
      wrap(&idx_read);
      
      buf[idx_write++] = (inSample + (delayFeedback * outSample) / AMP_MAX) / 2;
      wrap(&idx_write);

      return outSample;
    }
};

#endif
