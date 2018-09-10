/*
  ==============================================================================

    VarDelay.cpp
    Created: 21 Jul 2017 2:58:25pm
    Author:  Alex Jones

  ==============================================================================
*/

#include "VarDelay.h"

using namespace juce;

//==============================================================================

Threshold::Threshold()
{
    threshold = 0.0001;
    previous  = 0.0002;
}

Threshold::~Threshold(){}

void Threshold::setThreshold(float newThreshold)
{
    threshold = newThreshold;
}

bool Threshold::testThreshold(float input)
{
    bool result;
    if (input < threshold)
    {
        
    }
    // If the value is rising & we are above the threshold
    if (previous <= threshold && input > threshold)
    {
        result = true;
    }
    // If the value is falling or we are below the threshold
    else
    {
        result = false;
    }
    previous = input;
    return result;
}

//==============================================================================

VarDelay::VarDelay(int indexID) : ID (indexID), emphasisState(false), freezeState(0), onUnfreeze(false), onFreeze(false), loops(maxDelayTime)
{
    counter = 0;
    mix = 1;
    feedback = 0.95;
    freq = 10;
    freqSmooth = 0.5;
    pos1 = pos2 = Pdiff = Vdiff = 0;
    readableDelayTime = 0;
    freqRoundVal = 1;
    emphasisSmooth = 1;
    
    modSpeed = 0.f;
    modDepth = 0.f;
    outGain = 1;
     
    modOsc[0] = &sinOsc[0];
    modOsc[1] = &sinOsc[1];
    
    parameters = {"delayTime", "feedback", "mix", "modSpeed", "modDepth"};
}

VarDelay::~VarDelay()
{
    
}

void VarDelay::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    scratchBuffer[0].setSize (2, samplesPerBlockExpected);
    scratchBuffer[1].setSize (2, samplesPerBlockExpected);
    
    for (int ch = 0; ch < 2; ch++)
    {
        sampleBuffer[ch].setSize(2, maxDelayTime * sampleRate);
        sampleBuffer[ch].clear();
        bufferSize[ch] = (int)(maxDelayTime * sampleRate);
        bufferWritePos[ch] = 0;
    }
    sR = sampleRate;
    for (int ch = 0; ch < 2; ch++)
    {
        sinOsc[ch].setSampleRate(sR);
        triOsc[ch].setSampleRate(sR);
        sqrOsc[ch].setSampleRate(sR);
        
        winOsc[ch].setSampleRate(sR);
        winOsc[ch].setAmplitude(1);
        winOsc[ch].setFrequency(10);
        
        delWindowValues[ch][0].setValue(0);
        delWindowValues[ch][1].setValue(0);
    }
    
    modValues[0][0].reset(sR, 0.5);
    modValues[0][1].reset(sR, 0.5);
    modValues[1][0].reset(sR, 0.5);
    modValues[1][1].reset(sR, 0.5);
    
    emphasisSmooth.reset(sR, 0.5);
    freqSmooth.reset(sR, 1);
    outGainSmooth.reset(sR, 0.3);
    
    filterCoeff[0] = IIRCoefficients::makeHighPass(sampleRate, 180);
    filterCoeff[1] = IIRCoefficients::makeHighPass(sampleRate, 180);

    filter[0].setCoefficients(filterCoeff[0]);
    filter[1].setCoefficients(filterCoeff[1]);
    
    loops.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void VarDelay::releaseResources()
{
    
}

void VarDelay::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    int numSamples = bufferToFill.numSamples;
    float totalNumInputChannels = bufferToFill.buffer->getNumChannels();
    
    //----------------------------------------------------------------------------------------------------
    juce::AudioBuffer<float> inputSampleBuffer[2];
    
    // Refer the buffer to the scratch buffers, which should be the correct size.
    for (int i = 0; i < 2; i++)
    {
        inputSampleBuffer[i] = juce::AudioBuffer<float>(scratchBuffer[i].getArrayOfWritePointers(), totalNumInputChannels, numSamples);
    }
    
    // Copy the data from the data from the incoming buffer in our buffer.
    for (int ch = 0; ch < totalNumInputChannels; ch++)
    {
        inputSampleBuffer[ch].copyFrom(ch, 0, *bufferToFill.buffer, 0, 0, numSamples);
    }
    
    //----------------------------------------------------------------------------------------------------
    
    for (int ch = 0; ch < bufferToFill.buffer->getNumChannels(); ch++)
    {
        float bufferReadPos[2];
        float delSig[2];
        float modValMultiplier;
        //float modOffset = 0;
        
        float windowVal[2];
        float invWindowVal[2];
        float numSamples = bufferToFill.numSamples;
        float* outP = bufferToFill.buffer->getWritePointer(ch);
        float* inP  = inputSampleBuffer[ch].getWritePointer(ch);
        
        float modDepthLocal = modDepth.get();
        float modSpeedLocal = modSpeed.get();
        modOsc[ch]->setAmplitude(modDepthLocal);
        modOsc[ch]->setFrequency(modSpeedLocal);
        
        float index = (float)ID;
        
        double fD = (freqDist.get() * index) + 1.0;
        
        double roundValLocal = (1.0f - freqRoundVal.get()) * 12.0f;
        int roundValLocalInt = (int)roundValLocal + 1;
        
        switch (roundValLocalInt)
        {
            case 1:
                roundValLocal = 1;
                break;
            case 2:
                roundValLocal = 1.0666666667;
                break;
            case 3:
                roundValLocal = 1.125;
                break;
            case 4:
                roundValLocal = 1.2;
                break;
            case 5:
                roundValLocal = 1.25;
                break;
            case 6:
                roundValLocal = 1.3333333333;
                break;
            case 7:
                roundValLocal = 1.40625;
                break;
            case 8:
                roundValLocal = 1.5;
                break;
            case 9:
                roundValLocal = 1.6;
                break;
            case 10:
                roundValLocal = 1.6666666667;
                break;
            case 11:
                roundValLocal = 1.7777777778;
                break;
            case 12:
                roundValLocal = 1.875;
                break;
            case 13:
                roundValLocal = 2.0;
                break;
            default:
                roundValLocal = 1.0;
                break;
        }
        
        double harmonicityLocal = harmonicityVal.get();
        
        double harm = floor(fD * roundValLocal) / roundValLocal;
        double noHarm = (fD * roundValLocal) / roundValLocal;
        
        double fDRound = (harm * harmonicityLocal) + (noHarm * (1.0f - harmonicityLocal));
        
        if (onUnfreeze)
        {
            bufferReadPos[0] = loops.getReadPosition(0);
            bufferReadPos[1] = loops.getReadPosition(1);
            
            sampleBuffer[0].copyFrom(0, 0, loops.readFromBuffer(), 0, 0, loops.readFromBuffer().getNumSamples());
            sampleBuffer[0].copyFrom(1, 0, loops.readFromBuffer(), 0, 0, loops.readFromBuffer().getNumSamples());
            
            sampleBuffer[1].copyFrom(0, 0, loops.readFromBuffer(), 1, 0, loops.readFromBuffer().getNumSamples());
            sampleBuffer[1].copyFrom(1, 0, loops.readFromBuffer(), 1, 0, loops.readFromBuffer().getNumSamples());
            
            onUnfreeze = false;
        }
        
        while (numSamples--)
        {
            double feedbackLocal = feedback.get() * emphasisSmooth.getNextValue();
            bufferWritePos[ch]++;
            
            if (bufferWritePos[ch] > bufferSize[ch])
                bufferWritePos[ch] = 0;
            
            // Get the next value from the windowing oscillator and an inverted version.
            windowVal[ch] = (winOsc[ch].nextSample() * 0.5) + 0.5;
            invWindowVal[ch] = 1.0f - windowVal[ch];
            
            (modSpeedLocal != 0) ? modValMultiplier = modOsc[ch]->nextSample() : modValMultiplier = 0;
            modValMultiplier = ((modValMultiplier * 0.5) + 1.0f);

            double localFreq = freq.get() * modValMultiplier;
            double distributedFreq = localFreq * fDRound;
            
            double delTimeInSec = 1.0 / distributedFreq;
            
            if (delTimeInSec > maxDelayTime)
            {
                delTimeInSec = maxDelayTime;
            }
            
            //((delTimeInSec - modDepthLocal) < 0) ? modOffset = (modDepthLocal - delTimeInSec): modOffset = 0;
            
            readableDelayTime = delTimeInSec;
        
            float delTimeInSamples = (delTimeInSec * sR);
            
            // The threshold class returns true when the signal passes the given threshold. This allows the delay time to be changed for that buffer.
            if (threshold[ch][0].testThreshold(windowVal[ch]))
            {
                delWindowValues[ch][0].setValue(delTimeInSamples);
            }
            if (threshold[ch][1].testThreshold(invWindowVal[ch]))
            {
                delWindowValues[ch][1].setValue(delTimeInSamples);
            }
            
            float buf1Delay = delWindowValues[ch][0].getNextValue();
            float buf2Delay = delWindowValues[ch][1].getNextValue();
            
            bufferReadPos[0] = bufferWritePos[ch] - buf1Delay;
            bufferReadPos[1] = bufferWritePos[ch] - buf2Delay;
            
            for (int i = 0 ; i < 2; i++)
            {
                if (bufferReadPos[i] < 0)
                {
                    bufferReadPos[i] += sampleBuffer[i].getNumSamples()-1;
                }
                bufferReadPosRef[i][0] = bufferReadPos[0];
                bufferReadPosRef[i][1] = bufferReadPos[1];
            }
                            // Readpos         ch  window
            delSig[0] = read(bufferReadPos[0], ch, 0);
            delSig[1] = read(bufferReadPos[1], ch, 1);
            
            // The outgoing signal is combination of the output of the 2 windowed buffers, mulitplied by the sine wave used in the threshold test
            float reconstructedSignal = (delSig[0] * windowVal[ch]) + (delSig[1] * invWindowVal[ch]);
            
            //         writePos     |                  input                      | ch | win
             write(bufferWritePos[ch], *inP + (reconstructedSignal * feedbackLocal), ch, 0);
             write(bufferWritePos[ch], *inP + (reconstructedSignal * feedbackLocal), ch, 1);
            
            *outP += ( (reconstructedSignal * mix.get()) + (*inP * (1.0f - mix.get() ) ) ) * outGainSmooth.getNextValue();
            
            inP++;
            outP++;
        }
    }
    filter[0].processSamples(bufferToFill.buffer->getWritePointer(0), bufferToFill.numSamples);
    if (bufferToFill.buffer->getNumChannels() > 1)
    {
        filter[1].processSamples(bufferToFill.buffer->getWritePointer(1), bufferToFill.numSamples);
    }
    
    if (freezeState.get() != 0)
    {
        if (onFreeze)
        {
            for (int i = 0 ; i < 2; i++)
            {
                loops.setReadPosition(bufferReadPosRef[i][0], 0);
                loops.setReadPosition(bufferReadPosRef[i][1], 1);
            }
            loops.writeToBuffer(sampleBuffer[0], 0);
            loops.writeToBuffer(sampleBuffer[1], 1);
            onFreeze = false;
        }
        loops.getNextAudioBlock(bufferToFill);
        bufferToFill.buffer->applyGain(outGainSmooth.getNextValue());
    }
}

void VarDelay::write(int writePos, float input, int ch, int win)
{
    if(writePos < 0)
        writePos = 0;
    else if (writePos > bufferSize[ch]-1)
        writePos = bufferSize[ch]-1;
    
    float* sampleP = sampleBuffer[ch].getWritePointer(win, writePos);
    *sampleP = input;
}

float VarDelay::read(float readPos, int ch, int win)
{
    pos1 = (int)readPos;
    
    pos2 = pos1 + 1;
    
    if(pos1 == sampleBuffer[ch].getNumSamples()-1)
    {
        pos2 = pos1;
    }
    
    if(pos2 == bufferSize[ch])
        pos2 = 0;
    
    Pdiff = readPos - pos1;
    
    float* samplePos2 = sampleBuffer[ch].getWritePointer(win, pos2);
    float* samplePos1 = sampleBuffer[ch].getWritePointer(win, pos1);
    
    Vdiff = *samplePos2 - *samplePos1;
    
    return *samplePos1 + (Vdiff * Pdiff);
}

void VarDelay::setParameter (int paramType, float newValue)
{
    switch (paramType)
    {
        case freqP:
            freq = newValue;
            freqSmooth.setValue(freq.get());
            break;
        case freqDistP:
            freqDist = newValue;
            break;
        case feedbackP:
            feedback = newValue;
            break;
        case mixP:
            mix = newValue;
            break;
        case modDepthP:
            modDepth = newValue;
            break;
        case modSpeedP:
            modSpeed = newValue;
            break;
        case freqRoundP:
            freqRoundVal = newValue;
            break;
        case harmonicityP:
            harmonicityVal = newValue;
            break;
        case emphasisP:
            setEmphasis(newValue);
            break;
        case freezeP:
            setFreezeState(newValue);
            break;
        case reverseP:
            setReverseState(newValue);
            break;
        case braceWidthP:
            loops.setBraceWidth(newValue);
            break;
        case braceCentreP:
            loops.setCentreBrace(newValue);
            break;
        default:
            break;
    }
}

float VarDelay::getParameter (int paramType)
{
    float result = 0;
    switch (paramType)
    {
        case freqP:
            result = freq.get();
            break;
        case freqDistP:
            result = freqDist.get();
            break;
        case feedbackP:
            result = feedback.get();
            break;
        case mixP:
            result = mix.get();
            break;
        case modDepthP:
            result = modDepth.get();
            break;
        case modSpeedP:
            result = modSpeed.get();
            break;
        case freqRoundP:
            result = freqRoundVal.get();
            break;
        case harmonicityP:
            result = harmonicityVal.get();
            break;
        default:
            break;
    }
    return result;
}

float VarDelay::getModulatedDelayTime()
{
    return readableDelayTime.get();
}

void VarDelay::setOutGain (float newOutGain)
{
    outGain = newOutGain;
    outGainSmooth.setValue(outGain.get());
}

void VarDelay::setModWaveShape (int newModWaveshape)
{
    switch (newModWaveshape)
    {
        case sine:
            for (int i = 0; i < 2; i++)
            {
                modOsc[i] = &sinOsc[i];
                modOsc[i]->reset();
            }
            break;
        case triangle:
            for (int i = 0; i < 2; i++)
            {
                modOsc[i] = &triOsc[i];
                modOsc[i]->reset();
            }
            break;
        case square:
            for (int i = 0; i < 2; i++)
            {
                modOsc[i] = &sqrOsc[i];
                modOsc[i]->reset();
            }
            break;
        default:
            for (int i = 0; i < 2; i++)
            {
                modOsc[i] = &sinOsc[i];
                modOsc[i]->reset();
            }
            break;
    }
}

void VarDelay::resetModOscillators ()
{
    modOsc[0]->reset();
    modOsc[1]->reset();
}

float VarDelay::getMaxDelayTime()
{
    return maxDelayTime;
}

float VarDelay::getDelayTime()
{
    return readableDelayTime.get();
    //return freq.get();
}

juce::StringArray VarDelay::getParameters()
{
    return parameters;
}

juce::String VarDelay::getParameterByIndex(int index)
{
    return parameters[index];
}

int VarDelay::getIndexID()
{
    return ID;
}

void VarDelay::setEmphasis (float state)
{
    // For some reason the footcontrol is inverted, so this corrects for that.
    //float invertedState = 1 - state;
    float target = 1;
    
    if (!emphasisState)
    {
        emphasisState = true;
        target = 1.1;
    }
    else if (emphasisState)
    {
        emphasisState = false;
    }
    
    emphasis = target;
    emphasisSmooth.setValue(emphasis.get());
}

void VarDelay::setFreezeState (float state)
{
    if (freezeState.get() == 0)
    {
        freezeState = 1;
        onFreeze = true;
    }
    else
    {
        freezeState = 0;
        onUnfreeze = true;
    }
}

void VarDelay::setReverseState (float state)
{
    loops.setReverseState(state);
}

void VarDelay::valueTreePropertyChanged (ValueTree& treeWhosePropertyHasChanged, const Identifier& property) {}

//==============================================================================
