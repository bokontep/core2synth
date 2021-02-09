#include "ADSR.h"



ADSR::ADSR() {
  Reset();
  this->sampleRate = 44100;
  SetADSR(1.0, 1.0, 1.0, 1.0);
    
}

ADSR::ADSR(float sampleRate) {
  Reset();
  this->sampleRate = sampleRate;
  SetADSR(1.0, 1.0, 1.0, 1.0);
    
}

ADSR::~ADSR(void) {
}

void ADSR::SetADSR(uint8_t attack, uint8_t decay, uint8_t sustain, uint8_t release)
{
  SetSustainMidi(sustain);
  SetAttackMidi(attack);
  SetDecayMidi(decay);
  SetReleaseMidi(release);
}
void ADSR::SetAttack(float attack) {

  this->attack = attack;
  if (attack < 0.001)
  {
    attack = 0.001;
  }
  
    attackCoef = 1.0/attack;
    attackBase = 0.0;
    
}


void ADSR::SetDecay(float decay) {
  this->decay = decay;
  if (decay < 0.001)
  {
    decay = 0.001;
  }
  decayCoef = (1.0 - this->sustainLevel) / this->decay;
}

    


void ADSR::SetSustain(float level) {
    sustainLevel = level;
    decayBase = sustainLevel;
}

void ADSR::SetRelease(float releaseval)
{
  release = releaseval;
  if (release < 0.001)
  {
    release = 0.001;
  }
  releaseCoef = (this->sustainLevel / this->release);
  releaseBase = this->sustainLevel;
}

void ADSR::SetAttackMidi(uint8_t attack)
{
  this->midiAttack=attack;
  this->attack = ((float)midiAttack)/127.0;
  double maxlength = 1.0*sampleRate;
  if(midiAttack==0)
  {
    attackCoef = 1.0;
  }
  else
  {
    attackCoef = 127.0 /(maxlength*(double)midiAttack);
  }
  attackBase = 0.0;
  
}
void ADSR::SetDecayMidi(uint8_t decay)
{
  this->midiDecay=decay;
  this->decay = ((float)this->midiDecay)/127.0;
  double maxlength = 1.0*sampleRate;
  if(midiDecay==0)
  {
    decayCoef=1.0 - this->sustainLevel;  
  }
  else
  {
    decayCoef = ((1.0 - this->sustainLevel)*127.0) / (maxlength*(float)midiDecay);
  }
  
}

void ADSR::SetSustainMidi(uint8_t sustain)
{
  this->midiSustain=sustain;
  this->sustainLevel = ((float)midiSustain)/127.0;
  decayBase = sustainLevel;
}

void ADSR::SetReleaseMidi(uint8_t release)
{
  this->midiRelease=release;
  this->release = ((float)this->midiRelease)/127.0;
  double maxlength = sampleRate*5.0;
  
  if(midiRelease==0)
  {
    releaseCoef=this->sustainLevel;
  }
  else
  {
    releaseCoef = (this->sustainLevel*127.0) / (maxlength*((double)midiRelease));
    //releaseCoef = ((this->sustainLevel * 127.0)/maxlength)/(float)midiRelease; 
  }
  releaseBase = this->sustainLevel;
}
