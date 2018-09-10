/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
StringSynthPluginAudioProcessorEditor::StringSynthPluginAudioProcessorEditor (StringSynthPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);

	addAndMakeVisible(noteOnButton);
	noteOnButton.addListener(this);
	noteOnButton.setButtonText("Add Note");

	addAndMakeVisible(noteOffButton);
	noteOffButton.addListener(this);
	noteOffButton.setButtonText("Dec Note");

	addAndMakeVisible(freq1Slider);
	freq1Slider.setRange(100, 1000);
	freq1Slider.setSliderStyle(Slider::LinearVertical);
	freq1Slider.addListener(this);
}

StringSynthPluginAudioProcessorEditor::~StringSynthPluginAudioProcessorEditor()
{
}

//==============================================================================
void StringSynthPluginAudioProcessorEditor::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (Colours::darkgrey);

    g.setColour (Colours::hotpink);
    g.setFont (10.0f);
	g.drawFittedText ("String Synth", getLocalBounds(), Justification::topLeft, 1);
}

void StringSynthPluginAudioProcessorEditor::resized()
{
	Rectangle<int> r = getLocalBounds();
	Rectangle<int> sliderBounds = r.removeFromLeft(r.getWidth()*0.2);

	freq1Slider.setBounds(sliderBounds);
	noteOnButton.setBounds(r.removeFromLeft((float)r.getWidth()*0.5));
	noteOffButton.setBounds(r);
}

void StringSynthPluginAudioProcessorEditor::buttonClicked(Button* b)
{
	if (b == &noteOnButton)
	{
		processor.triggerNoteOn();
	}
	else if (b == &noteOffButton)
	{
		processor.triggerNoteOff();
	}
}

void StringSynthPluginAudioProcessorEditor::sliderValueChanged(Slider* slider)
{
	if (slider == &freq1Slider)
	{
		processor.updateSynthVoiceFreq(0, slider->getValue());
	}
}