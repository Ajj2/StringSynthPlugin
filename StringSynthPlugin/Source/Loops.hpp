//
//  Loops.hpp
//  Thinstrument
//
//  Created by Alex Jones on 08/06/2018.
//
//

#ifndef Loops_hpp
#define Loops_hpp

#include <stdio.h>
#include  "../JuceLibraryCode/JuceHeader.h"


class Loops : public AudioSource
{
public:
    Loops(float maxDelayTime_);
    ~Loops();
    
    /** Tells the source to prepare for playing.*/
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    /** Allows the source to release anything it no longer needs after playback has stopped.*/
    void releaseResources() override;
    /** Called repeatedly to fetch subsequent blocks of audio data.*/
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    
    void writeToBuffer(AudioBuffer<float>& sourceBuffer, int index);
    AudioBuffer<float>& readFromBuffer();
    
    void setReadPosition(int newReadPos, int channel);
    int getReadPosition(int index) { return readPosition[index]; };
    
    void setReverseState(int newState);
    void setOpenBrace(float openBrace_);
    void setCloseBrace(float closeBrace_);
    void setCentreBrace(float centreBrace_);
    void setBraceWidth(float braceWidth_);
    void updateLoopBraces();
private:
    juce::AudioBuffer<float> loopBuffer;
    float maxDelayTime;
    float readPosition[2];
    Atomic<int> reverse = 0;
    Atomic<float> openBrace = 0;
    Atomic<float> closeBrace = 0;
    Atomic<float> centreBrace = 0;
    Atomic<float> braceWidth = 0;
};




#endif /* Loops_hpp */
