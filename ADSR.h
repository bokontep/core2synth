#ifndef ADSR_h
#define ADSR_h

#include <Arduino.h>

class  ADSR {
public:
    ADSR();
    ADSR(float sampleRate);
    ~ADSR(void);
    float Process(void);
    float GetOutput(void);
    int GetState(void);
  void SetADSR(uint8_t attack, uint8_t decay, uint8_t sustain, uint8_t release);
  void Gate(int on);
  void SetAttack(float attack);
  void SetDecay(float decay);
  void SetRelease(float release);
  void SetSustain(float level);
  float GetAttack(){return attack;}
  float GetDecay(){return decay;}
    float GetSustain(){return sustainLevel;}
    float GetRelease(){return release;}
    void Reset(void);
    void SetAttackMidi(uint8_t attack);
    void SetDecayMidi(uint8_t decay);
    void SetSustainMidi(uint8_t sustain);
    void SetReleaseMidi(uint8_t release);
    uint8_t GetAttackMidi(){return midiAttack;}
    uint8_t GetDecayMidi(){return midiDecay;}
    uint8_t GetSustainMidi(){return midiSustain;}
    uint8_t GetReleaseMidi(){return midiRelease;}
    double GetAttackCoef(){return attackCoef;}
    double GetDecayCoef(){return decayCoef;}
    double GetReleaseCoef(){return releaseCoef;}
    double GetReleaseBase(){return releaseBase;}

    enum envState {
        env_idle = 0,
        env_attack,
        env_decay,
        env_sustain,
        env_release
    };

protected:

  int state;
  float output;
  float attack;
  float decay;
  float release;
  int32_t counter;
  double attackCoef;
  double decayCoef;
  double releaseCoef;
  double sustainLevel;
  double attackBase;
  double decayBase;
  double releaseBase;
  uint8_t midiAttack;
  uint8_t midiDecay;
  uint8_t midiSustain;
  uint8_t midiRelease;
  uint32_t sampleRate;
};

inline float ADSR::Process() {
  switch (state) {
        case env_idle:
      output = 0.0;
            break;
        case env_attack:
            output = output+attackBase +  attackCoef;
      
            if (output >= 1.0) {
                output = 1.0; //http://www.earlevel.com/main/2013/06/01/envelope-generators/
                state = env_decay;
        
            }
            break;
        case env_decay:
            output = output - decayCoef;
      
            if (output <= sustainLevel || output <=0) {
                output = sustainLevel;
                state = env_sustain;
            }
            break;
        case env_sustain:
          output = sustainLevel;
            break;
        case env_release:
            output = output - releaseCoef;
      
            if (output <= 0.0) {
                output = 0.0;
                state = env_idle;
            }
      break;
  }
  return output;
}

inline void ADSR::Gate(int gate) {
  if (gate)
  {

    output = 0.0;
    state = env_attack;
  }
  else if (state != env_idle)
  {
    state = env_release;
    
  }
}

inline int ADSR::GetState() {
    return state;
}

inline void ADSR::Reset() {
    state = env_idle;
    output = 0.0;
}

inline float ADSR::GetOutput() {
  return output;
}

#endif
