/*
  ==============================================================================

    Oscillator.h
    Created: 29 Sep 2016 5:10:23pm
    Author:  Alex Jones

  ==============================================================================
*/

#ifndef OSCILLATOR_H_INCLUDED
#define OSCILLATOR_H_INCLUDED

#include <JuceHeader.h>
#include <cmath>

# define M_PI           3.14159265358979323846  /* pi */

class Oscillator
{
public:
    //==============================================================================
    /**
     SinOscillator constructor
     */
    Oscillator();
    
    /**
     SinOscillator destructor
     */
    virtual ~Oscillator();
    
    /**
     sets the frequency of the oscillator
     */
    void setFrequency (float freq);
    
    /**
     sets frequency using a midi note number
     */
    void setNote (int noteNum);
    
    /**
     sets the amplitude of the oscillator
     */
    void setAmplitude (float amp);
    
    /**
     resets the oscillator
     */
    void reset();
    
    /**
     sets the sample rate
     */
    void setSampleRate (float sr);
    
    /**
     Returns the next sample
     */
    float nextSample();
    
    /**
     function that provides the execution of the waveshape
     */
    virtual float renderWaveShape(const float currentPhase) = 0;
    
private:
    float frequency;
    float amplitude;
    float sampleRate;
    float phase;
    float phaseInc;
};

#endif  // OSCILLATOR_H_INCLUDED
