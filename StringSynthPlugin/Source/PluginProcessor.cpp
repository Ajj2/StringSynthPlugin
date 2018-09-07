/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
StringSynthPluginAudioProcessor::StringSynthPluginAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
	synth = new Synthesiser();
	synth->clearVoices();
	synth->clearSounds();
	synth->addSound(new StringSynthSound());
	//startTimerHz(2);
}

StringSynthPluginAudioProcessor::~StringSynthPluginAudioProcessor()
{
}

void StringSynthPluginAudioProcessor::timerCallback()
{
	/*static int noteInc = 59;

	synth->noteOn(1, ++noteInc, 0.5);

	if (prevNote != -1)
	{
		synth->noteOff(1, prevNote, 0, true);
	}

	prevNote = noteInc;*/

	if (noteStarted == false)
	{
		synth->noteOn(1, 60, 1);
		noteStarted = true;
		DBG("1");
	}

	static int counter = 0;
	counter++;
	
	if (counter == 10)
	{
		DBG("2");
		if (noteStarted == true && noteStopped == false)
		{
			DBG("3");
			synth->noteOff(1, 60, 1, true);
			noteStopped = true;
		}
	}
}

void StringSynthPluginAudioProcessor::triggerNoteOn()
{
	DBG("ON");
	synth->noteOn(1, 60, 1);
}

void StringSynthPluginAudioProcessor::triggerNoteOff()
{
	DBG("OFF");
	synth->noteOff(1, 60, 1, true);
}


//==============================================================================
const String StringSynthPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool StringSynthPluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool StringSynthPluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool StringSynthPluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double StringSynthPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int StringSynthPluginAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int StringSynthPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void StringSynthPluginAudioProcessor::setCurrentProgram (int index)
{
}

const String StringSynthPluginAudioProcessor::getProgramName (int index)
{
    return {};
}

void StringSynthPluginAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void StringSynthPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	synth->setCurrentPlaybackSampleRate(sampleRate);
	synth->setNoteStealingEnabled(true);

	for (int v = 0; v < 32; v++)
	{
		synth->addVoice(new StringSynthVoice());
	}
}

void StringSynthPluginAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool StringSynthPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void StringSynthPluginAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    for (int channel = 0; channel < totalNumOutputChannels; ++channel)
    {
		auto* channelData = buffer.getWritePointer(channel);
    }
	synth->renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
}

//==============================================================================
bool StringSynthPluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* StringSynthPluginAudioProcessor::createEditor()
{
    return new StringSynthPluginAudioProcessorEditor (*this);
}

//==============================================================================
void StringSynthPluginAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void StringSynthPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new StringSynthPluginAudioProcessor();
}
