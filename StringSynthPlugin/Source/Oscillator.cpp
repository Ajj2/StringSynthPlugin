/*
  ==============================================================================

    Oscillator.cpp
    Created: 29 Sep 2016 5:10:23pm
    Author:  Alex Jones

  ==============================================================================
*/

#include "Oscillator.h"

using namespace juce;

OscillatorLoc::OscillatorLoc()
{
    reset();
}

OscillatorLoc::~OscillatorLoc()
{
    
}

void OscillatorLoc::setFrequency (float freq)
{
    frequency = freq;
    phaseInc = (2 * M_PI * frequency ) / sampleRate ;
}

void OscillatorLoc::setNote (int noteNum)
{
    setFrequency (440.f * pow (2, (noteNum - 69) / 12.0));
}

void OscillatorLoc::setAmplitude (float amp)
{
    amplitude = amp;
}

void OscillatorLoc::reset()
{
    phase = 0.f;
}

void OscillatorLoc::setSampleRate (float sr)
{
    sampleRate = sr;
    setFrequency (frequency );//just to update the phaseInc
}

float OscillatorLoc::nextSample()
{
    float out = renderWaveShape (phase ) * amplitude ;
    //phase = phase + phaseInc ;
	phase += phaseInc;
    if(phase  > (2.f * M_PI))
        phase -= (2.f * M_PI);
    
    return out;
}
