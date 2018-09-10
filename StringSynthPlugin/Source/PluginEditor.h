/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class StringSynthPluginAudioProcessorEditor  : public AudioProcessorEditor, public Button::Listener, Slider::Listener
{
public:
    StringSynthPluginAudioProcessorEditor (StringSynthPluginAudioProcessor&);
    ~StringSynthPluginAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

	void buttonClicked(Button* b) override;
	void sliderValueChanged(Slider* slider) override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    StringSynthPluginAudioProcessor& processor;

	TextButton noteOnButton;
	TextButton noteOffButton;

	Slider freq1Slider;
	bool Switch = false;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StringSynthPluginAudioProcessorEditor)
};
