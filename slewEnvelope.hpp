#ifndef SLEWENVELOPE_HPP
#define SLEWENVELOPE_HPP

class slewEnvelope
{
  private:
    int currentValue, envAttack, envRelease, envMax, envMin;
    
  public:
    slewEnvelope()
    {
      currentValue = 0;
      envAttack = 1;
      envRelease = 1;
      envMax = 1;
      envMin = 1;
    }

    void setAttack(int attackValue)
    {
      envAttack = attackValue;
    }

    void setRelease(int releaseValue)
    {
      envRelease = releaseValue;
    }

    void setMax(int maxValue)
    {
      envMax = maxValue;
    }

    void setMin(int minValue)
    {
      envMin = minValue;
    }
  
    int getLevel(int val) 
    {
      int newValue;
      
      if (val > envMax)
      {
        newValue = envMax;
      }
      else if (val < envMin)
      {
        newValue = 0;
      }
      else
      {
        newValue = val;
      }

      newValue = (AMP_MAX * newValue) / envMax;
    
      if (newValue > currentValue)
      {  
        currentValue += envAttack;

        if (currentValue > newValue)
        {
          currentValue = newValue;
        }
      }
      else if (newValue != currentValue)
      {
        if (currentValue >= envRelease)
        {
          currentValue -= envRelease;
        }
        else
        {
          currentValue = 0;
        }

        if (currentValue < newValue)
        {
          currentValue = newValue;
        }
      }

      return currentValue;
    }
};

#endif
