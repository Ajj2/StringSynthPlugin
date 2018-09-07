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

	addAndMakeVisible(noteButton);
	noteButton.addListener(this);
	noteButton.setButtonText("NOTE ON/OFF");
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

	noteButton.setBounds(r.reduced(20));
}

void StringSynthPluginAudioProcessorEditor::buttonClicked(Button* b)
{
	if (Switch)
	{
		processor.triggerNoteOn();
		Switch = false;
	}
	else if (!Switch)
	{
		processor.triggerNoteOff();
		Switch = true;
	}
}