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

	for (int v = 1; v <= 8; v++)
	{
		String tempFreqID = (String)v << "Freq";
		String tempFdbkID = (String)v << "Fbdk";
		String tempTrigID = (String)v << "Trigger";

		idMapFdbk.set(v-1, tempFdbkID);
		idMapFreq.set(v-1, tempFreqID);
		idMapTrig.set(v-1, tempTrigID);

		parameters.createAndAddParameter(tempFdbkID, tempFdbkID, tempFdbkID, NormalisableRange<float>(0.0f, 1.0f), 0.9, nullptr, nullptr);

		parameters.createAndAddParameter(tempFreqID, tempFreqID, tempFreqID,
			NormalisableRange<float>(0.0f, 1.0f), 0,
			[](float value)
		{
			value = (value * 19.f) + 1.0f;
			return String(value, 3);
		},
			[](const String& text)
		{
			return ((text.getFloatValue() - 1.0f) / 19.0f);
		});

		parameters.createAndAddParameter(tempTrigID, tempTrigID, tempTrigID, NormalisableRange<float>(0.0f, 1.0f), 0, nullptr, nullptr);
	}
	parameters.state = ValueTree(Identifier("Processor Parameters"));

	synth = new Synth();
	synth->clearVoices();
	synth->clearSounds();
	synth->addSound(new StringSynthSound());

	if (!voiceAdded)
	{
		for (int v = 0; v < numVoices; v++)
		{
			StringSynthVoice* aVoice;
			synth->addVoice(aVoice = new StringSynthVoice(v));
			aVoice->setSynthesiser(synth);
		}
		voiceAdded = true;
	}

	for (int i = 0; i < numMultipliers; i++)
	{
		harmonicMultipliers[i] = (float)i + 1;
	}

	//startTimer(1500);
}

StringSynthPluginAudioProcessor::~StringSynthPluginAudioProcessor()
{
}

//void StringSynthPluginAudioProcessor::timerCallback()
//{
//	Random randy;
//	randy.setSeedRandomly();
//
//	int aRandomNumberOfVoices = randy.nextFloat() * 7.0f;
//
//	static int counter = 0;
//
//	
//	for (int i = 0; i < aRandomNumberOfVoices; i++)
//	{
//		if (counter++ >= 7)
//		{
//			counter = 0;
//		}
//
//		float harmmultindex = randy.nextFloat() * 19.0f;
//		float harmfreq = harmonicMultipliers[(int)harmmultindex] * fundamentalFreq;
//	
//	    int wait = (randy.nextFloat() * 200.0f) + 100.0f;
//
//		Time::waitForMillisecondCounter(Time::getMillisecondCounter() + wait);
//
//		synth->updateVoiceFdbk(counter, 0.999);
//		synth->noteOnForVoice(counter, harmfreq);
//	}
//
//	startTimerHz((randy.nextFloat() * 2.0) + 1.0);
//}

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
	synth->setNoteStealingEnabled(false);

	for (int v = 0 ; v < synth->getNumVoices(); v++)
	{
		StringSynthVoice* voice = reinterpret_cast<StringSynthVoice*> (synth->getVoice(v));
		voice->prepare(samplesPerBlock, sampleRate);
	}
}

void StringSynthPluginAudioProcessor::releaseResources()
{
	synth->audioOff();
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

	//(String)v << "Fbdk";
	float fdbk = *parameters.getRawParameterValue(idMapFdbk[0]);

	float freqs[numVoices] = {1};
	float triggers[numVoices] = { 0 };

	for (int i = 0; i < numVoices; i++)
	{
		//freqs[i] = (*parameters.getRawParameterValue(idMapFreq[i+1]) * 499.f) + 1.f;
		float harmMultIndex = (*parameters.getRawParameterValue(idMapFreq[i]) * 19.0f);
		float harmFreq = harmonicMultipliers[(int)harmMultIndex] * fundamentalFreq;

		triggers[i] = *parameters.getRawParameterValue(idMapTrig[i]);
		
		synth->updateVoiceFdbk(i, fdbk);

		if (prevTrigger[i] == 0 && triggers[i] != 0)
		{
			synth->noteOnForVoice(i, harmFreq);
		}
		prevTrigger[i] = triggers[i];
	}

	synth->renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
}

//==============================================================================
bool StringSynthPluginAudioProcessor::hasEditor() const
{
    return false; // (change this to false if you choose to not supply an editor)
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
