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

	void triggerNoteOn();
	void triggerNoteOff();

	void updateSynthVoiceFreq(int index, float freq);

private:
	AudioProcessorValueTreeState parameters;

	ScopedPointer<Synth> synth;
	int prevNote = -1;
	bool noteStarted = false;
	bool noteStopped = false;

	int index = 0;
	const int numNotes = 10;
	int notes[10] = {60, 61 , 62, 63, 64, 65, 66, 67, 68, 69};
	//ScopedPointer<Synth> synth;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StringSynthPluginAudioProcessor)
};

