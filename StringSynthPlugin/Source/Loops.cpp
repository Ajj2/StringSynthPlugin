//
//  Loops.cpp
//  Thinstrument
//
//  Created by Alex Jones on 08/06/2018.
//
//

#include "Loops.hpp"


Loops::Loops(float maxDelayTime_) : maxDelayTime(maxDelayTime_)
{
    readPosition[0] = 0;
    readPosition[1] = 0;
    
    reverse = 0;
}

Loops::~Loops()
{
    
}

/** Tells the source to prepare for playing.*/
void Loops::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    loopBuffer.setSize(2, (double)maxDelayTime * sampleRate);
    closeBrace = loopBuffer.getNumSamples()-1;
}

/** Allows the source to release anything it no longer needs after playback has stopped.*/
void Loops::releaseResources()
{
    
}

/** Called repeatedly to fetch subsequent blocks of audio data.*/
void Loops::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    for (int ch = 0; ch < bufferToFill.buffer->getNumChannels(); ch++)
    {
        float* outP = bufferToFill.buffer->getWritePointer(ch);
        int numSamp = bufferToFill.numSamples;
        
        while (numSamp--)
        {
            *outP = loopBuffer.getSample(ch, readPosition[ch]);
            outP++;
            
            if (reverse.get() == 1)
            {
                readPosition[ch]--;
                if (readPosition[ch] <= (int)openBrace.get())
                {
                    readPosition[ch] = (int)closeBrace.get();
                }
            }
            else
            {
                readPosition[ch]++;
                if (readPosition[ch] >= (int)closeBrace.get())
                {
                    readPosition[ch] = (int)openBrace.get();
                }
            }
        }
    }
}

void Loops::writeToBuffer(AudioBuffer<float>& sourceBuffer, int index)
{    
    loopBuffer.copyFrom(index, 0, sourceBuffer, 0, 0, sourceBuffer.getNumSamples());
}

AudioBuffer<float>& Loops::readFromBuffer()
{
    return loopBuffer;
}

void Loops::setReadPosition(int newReadPos, int channel)
{
    readPosition[channel] = newReadPos;
}

void Loops::setReverseState(int newState)
{
    reverse = newState;
}

void Loops::setOpenBrace(float openBrace_)
{
    openBrace = openBrace_;
}

void Loops::setCloseBrace(float closeBrace_)
{
    closeBrace = closeBrace_;
}

void Loops::setCentreBrace(float centreBrace_)
{
    centreBrace = centreBrace_;
    updateLoopBraces();
}

void Loops::setBraceWidth(float braceWidth_)
{
    braceWidth = braceWidth_;
    updateLoopBraces();
}

void Loops::updateLoopBraces()
{
    float nS = (float)loopBuffer.getNumSamples();
    
    float ob = centreBrace.get() - (braceWidth.get() * 0.5);
    float cb = centreBrace.get() + (braceWidth.get() * 0.5);
    
    ob *= nS;
 
    if (ob < 0) {ob = 0;}
    else if (ob > nS - 1) {ob = nS-1;}
    
    cb *= nS;
    
    if (cb < 0) {cb = 0;}
    else if (cb > nS - 1) {cb = nS-1;}
    
    DBG ("ob: " << ob << " - " << "cb: " << cb);
    
    setOpenBrace(ob);
    setCloseBrace(cb);
}

