#ifndef AUDIOBUFFER_HPP
#define AUDIOBUFFER_HPP

class audioBuffer
{
  private:
    short buf[AUDIO_BUF_SIZE];
    int idx_read, idx_write, lvl;
    bool locked;

    inline void wrap(int *p)
    {
      if (*p == AUDIO_BUF_SIZE)
      {
        *p = 0;
      }
    }
    
  public:
    audioBuffer()
    {
      locked = false;
      idx_read = 0;
      idx_write = 0;
      lvl = 0;
    }

    bool putSample(int sample) 
    {
      locked = true;

      if (lvl >= AUDIO_BUF_SIZE)
      {
        locked = false;
        return false;
      }
      
      lvl++;
      buf[idx_write++] = sample;
      wrap(&idx_write);

      locked = false;
      return true;
    }
  
    bool getSample(int *sample) 
    {
      if (locked)
      {
        return false;
      }
      
      if (lvl == 0)
      {
        return false;
      }

      lvl--;
      *sample = buf[idx_read++];
      wrap(&idx_read);

      return true;
    }
};

#endif
