#include "Arduino.h"
#include "LowPass.h"
const float maxval = 0.8;
float getSin(int index,int waveform_len)
{
  return maxval*sin(2.0 * (PI / (float)waveform_len) * index);
}


void initWaveforms(float* waveforms, int waveform_len, int waveform_count,bool bandlimit,float freq,float q)
{

  
  float sin_f = 1.0;
  
  for (int w = 0; w < waveform_count; w++)
  {
      for (int i = 0; i < waveform_len; i++)
      {
          waveforms[w*waveform_len + i] = getSin(i, waveform_len);
      }
  }

}
