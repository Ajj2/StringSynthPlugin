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
                       ), parameters(*this, nullptr)
#endif
{
	parameters.createAndAddParameter(ID1Amp, ID1Amp, ID1Amp, NormalisableRange<float>(0.0f, 1.0f), 0, nullptr, nullptr);
	parameters.createAndAddParameter(ID1Freq, ID1Freq, ID1Freq, NormalisableRange<float>(0.0f, 1.0f), 0, nullptr, nullptr);
	
	parameters.state = ValueTree(Identifier("Processor Parameters"));

	synth = new Synth();
	synth->clearVoices();
	synth->clearSounds();
	synth->addSound(new StringSynthSound());
}

StringSynthPluginAudioProcessor::~StringSynthPluginAudioProcessor()
{
}

void StringSynthPluginAudioProcessor::triggerNoteOn()
{
	DBG("INDEX. 1: " << index);
	synth->noteOn(1, notes[index], 1);

	index++;

	if (index > numNotes - 1)
	{
		index = (numNotes - 1);
	}

	DBG("getNumActiveNotes: " << synth->getNumActiveNotes());

	for (int i = 0; i < synth->getNumActiveNotes(); i++)
	{
		StringSynthVoice* test = reinterpret_cast<StringSynthVoice*> (synth->getVoice(i));
		int id = test->getNoteNum();
		DBG(i << " : " << (String)id);
	}
}

void StringSynthPluginAudioProcessor::triggerNoteOff()
{
	index--;

	if (index < 0)
	{
		index = 0;
	}

	DBG("INDEX. 2: " << index);

	if (index >= 0)
	{
		synth->noteOff(1, notes[index], 0, true);
	}

	Time::waitForMillisecondCounter(Time::getMillisecondCounter() + 100);

	DBG("getNumActiveNotes: " << synth->getNumActiveNotes());

	for (int i = 0; i < synth->getNumActiveNotes(); i++)
	{
		StringSynthVoice* voice = reinterpret_cast<StringSynthVoice*> (synth->getVoice(i));
		int id = voice->getNoteNum();
		float co = voice->getCutoff();
		DBG(i << " : " << (String)id << " --> " << (String)co);
	}
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
		StringSynthVoice* aVoice;
		synth->addVoice(aVoice = new StringSynthVoice(v));
		aVoice->setSynthesiser(synth);
		aVoice->prepare(samplesPerBlock, sampleRate);
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

	float freq1 = *parameters.getRawParameterValue(ID1Freq);

	synth->updateVoiceFreq(1, freq1);

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

void StringSynthPluginAudioProcessor::updateSynthVoiceFreq(int index, float freq)
{
	synth->updateVoiceFreq(index, freq);
}
//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new StringSynthPluginAudioProcessor();
}
