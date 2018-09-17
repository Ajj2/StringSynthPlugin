/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "SynthPlugin.h"
#include "Identifiers.h"

//==============================================================================
/**
*/
class StringSynthPluginAudioProcessor  : public AudioProcessor
{
public:
    //==============================================================================
    StringSynthPluginAudioProcessor();
    ~StringSynthPluginAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

	void updateSynthVoiceFreq(int index, float freq);

	/*void timerCallback() override;*/

private:
	AudioProcessorValueTreeState parameters;

	ScopedPointer<Synth> synth;
	int prevNote = -1;
	bool voiceAdded = false;

	HashMap<int, String> idMapFreq;
	HashMap<int, String> idMapFdbk;
	HashMap<int, String> idMapTrig;

	float fundamentalFreq = 65.41f; // C2

	const static int numMultipliers = 20; 
	float harmonicMultipliers[numMultipliers];

	const static int numVoices = 8;

	float prevTrigger[numVoices] = { 0 };

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StringSynthPluginAudioProcessor)
};

