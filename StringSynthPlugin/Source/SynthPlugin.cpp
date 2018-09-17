/*
  ==============================================================================

    SynthPlugin.cpp
    Created: 4 Sep 2018 4:33:18pm
    Author:  Alex

  ==============================================================================
*/

#include "SynthPlugin.h"


StringSynthVoice::StringSynthVoice(int ID_) : ID(ID_)
{
	float a = 0.05;
	float d = 0.1;
	envelopeADinMS = (a * 1000.0f) + (d * 1000.0f);
	for (int ch = 0; ch < 2; ch++)
	{
		envelope[ch].setAttackTime(a);
		envelope[ch].setDecayTime(d);
		envelope[ch].setSustainLevel(1);
		envelope[ch].setReleaseTime(2);

		excitation[ch].setAttackTime(0.01);
		excitation[ch].setDecayTime(0.02);
		excitation[ch].setSustainLevel(0.1);
		excitation[ch].setReleaseTime(0.1);

		excitation[ch].keyOff();
		envelope[ch].keyOff();
	}
	tailOff = 0.0;
	state = inActive;

	vardle = new VarDelay(ID);

	randy.setSeedRandomly();

	float modSpeed = (randy.nextFloat() * 4.0f) + 10.0f;
	float modDepth = (randy.nextFloat() * 0.03f) + 0.03f;

	vardle->setParameter(VarDelay::feedbackP, 0.9f);
	vardle->setParameter(VarDelay::freqP, 200);
	vardle->setParameter(VarDelay::freqDistP, 0);
	vardle->setParameter(VarDelay::mixP, 1);
	vardle->setParameter(VarDelay::modSpeedP, modSpeed);
	vardle->setParameter(VarDelay::modDepthP, modDepth);
	vardle->setParameter(VarDelay::harmonicityP, 1);
	vardle->setParameter(VarDelay::freqRoundP, 1);

	vardle->setOutGain(3.5);
	outGain[0].setValue(0.0);
	outGain[1].setValue(0.0);

	noise.setSeed(Time::getMillisecondCounter());

	sinOsc[0].reset();
	sinOsc[1].reset();

	sinOsc[0].setFrequency(100);
	sinOsc[1].setFrequency(200);

	sinOsc[0].setAmplitude(0.8);
	sinOsc[1].setAmplitude(0.8);
}

StringSynthVoice::~StringSynthVoice()
{

}

bool StringSynthVoice::canPlaySound(SynthesiserSound* ss)
{
	return dynamic_cast<StringSynthSound*> (ss) != nullptr;
}

void StringSynthVoice::startNote(int midiNoteNumber, float velocity, SynthesiserSound* sound, int currentPitchWheelPosition)
{
	bool updateFilter = true;

	if (note == midiNoteNumber)
	{
		updateFilter = false;
	}
	note = midiNoteNumber;
	state = active_NotFading;
	parentSynth->incActiveNotes();
	counter[0] = counter[1] = 0;

	envelope[0].keyOn();
	envelope[1].keyOn();
	excitation[0].keyOn();
	excitation[1].keyOn();

	cutoff = MidiMessage::getMidiNoteInHertz(midiNoteNumber);

	DBG("SN   - Starting voice " << ID << " at freq " << cutoff);
	vardle->setParameter(VarDelay::freqP, cutoff);
	vardle->reset();

	numBlockProcessed = 0;

	outGain[0].setValue(1.0f);
	outGain[1].setValue(1.0f);
}

void StringSynthVoice::startNoteAtFreq(float frequency, float velocity)
{
	parentSynth->incActiveNotes();

	for (int i = 0; i < 2 ; i++)
	{
		envelope[i].keyOn();
		excitation[i].keyOn();
	}

	DBG("SNAF - Starting voice " << ID << " at freq " << frequency);

	float modSpeed = (randy.nextFloat() * 4.0f) + 10.0f;
	float modDepth = (randy.nextFloat() * 0.03f) + 0.03f;
	excitationNoiseGain = 0;// randy.nextFloat() * 0.001;

	vardle->setParameter(VarDelay::modSpeedP, modSpeed);
	vardle->setParameter(VarDelay::modDepthP, modDepth);
	vardle->setParameter(VarDelay::freqP, frequency);
	vardle->reset();

	elapsedTime[0] = elapsedTime[1] = 0;
	
	outGain[0].setValue(1.0f);
	outGain[1].setValue(1.0f);

	state = active_NotFading;
}

void StringSynthVoice::startNoteWhenFinished(float frequency, float velocity, SynthesiserSound* sound)
{
	DBG("startNoteWhenFinished");
	nextFreq = frequency;
	nextVelocity = velocity;
}

void StringSynthVoice::stopNote(float velocity, bool allowTailOff)
{
	DBG("stopNote");
	tailOff = 1.0;
	//state = active_Fading;
	counter[0] = counter[1] = 0;
	envelope[0].keyOff();
	envelope[1].keyOff();
}

bool StringSynthVoice::isVoiceActive() const
{
	if (state == inActive)
	{
		//DBG(" - - Q. " << ID << " is not active ");
		return false;
	}
	else if (state == active_Fading || state == active_NotFading)
	{
		//DBG(" - - Q. " << ID << " is active ");
		return true;
	}
	else
	{
		DBG("is Voice active returns unknown state");
		return true;
	}
}

void StringSynthVoice::pitchWheelMoved(int newPitchWheelValue)
{

}

void StringSynthVoice::controllerMoved(int controllerNumber, int newControllerValue)
{

}

void StringSynthVoice::aftertouchChanged(int newAftertouchValue)
{

}

void StringSynthVoice::channelPressureChanged(int newChannelPressureValue)
{

}

void StringSynthVoice::renderNextBlock(AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
	int numOutChannels = outputBuffer.getNumChannels();

	AudioSampleBuffer inputSamples(scratchInputBuffers.getArrayOfWritePointers(), 2, numSamples);
	AudioSampleBuffer outputSamples(scratchOutputBuffers.getArrayOfWritePointers(), 2, numSamples);

	for (int ch = 0; ch < 2; ch++)
	{
		outputSamples.copyFrom(ch, 0, outputBuffer.getWritePointer(ch), numSamples);
	}

	inputSamples.clear();
	for (int ch = 0; ch < numOutChannels; ch++)
	{
		int numsamp = numSamples;
		float* outp = inputSamples.getWritePointer(ch);

 		while (numsamp--)
		{
			if (excitation[ch].getState() != stk::ADSR::IDLE)
			{
				*outp = excitation[ch].tick();// +(noise.tick() * excitationNoiseGain);
				outp++;
				if (excitation[ch].getState() == stk::ADSR::SUSTAIN)
				{
					excitation[ch].keyOff();
				}
			}
		}
	}

	//for (int ch = 0; ch < numOutChannels; ch++)
	//{
	//	int numSamp = numSamples;
	//	float* outP = outputBuffer.getWritePointer(ch);

	//	while (numSamp--)
	//	{
	//		*outP = sinOsc[ch].nextSample();
	//		outP++;	
	//	}
	//}

	//AudioSourceChannelInfo info(&outputBuffer, startSample, numSamples);
	AudioSourceChannelInfo output(&outputSamples, 0, numSamples);
	AudioSourceChannelInfo input(&inputSamples, 0, numSamples);
	vardle->getNextAudioBlockCustom(output, input);

	int fadeInComplete[2] = {0, 0};
	int fadeOutComplete[2] = { 0, 0 };
	float envVal[2] = { 0, 0 };
	float localOutGain[2] = { 0, 0 };	

	for (int ch = 0; ch < numOutChannels; ch++)
	{
		currentTime[ch] = Time::getMillisecondCounterHiRes();
		deltaTime[ch] = currentTime[ch] - prevTime[ch];
		prevTime[ch] = currentTime[ch];

		int numSamp = numSamples;
		float* outP = outputSamples.getWritePointer(ch);

		while (numSamp--)
		{
			if (state == active_Fading)
			{
				envVal[ch] = envelope[ch].tick();
				*outP *= envVal[ch];
				localOutGain[ch] = outGain[ch].getNextValue();
				*outP *= localOutGain[ch];
				
				if (envelope[0].getState() == Envelope::IDLE && envelope[1].getState() == Envelope::IDLE)
				{
					if (outGain[ch].getTargetValue() != 0)
					{
						outGain[ch].setValue(0);
					}
					if (envVal[0] < 0.0001 && envVal[1] < 0.0001)
					{
						if (localOutGain[0] <= outGain[0].getTargetValue() && localOutGain[1] <= outGain[1].getTargetValue())
						{
							fadeOutComplete[0] = 1;
							fadeOutComplete[1] = 1;
						}
					}
				}
				outP++;
			}
			else if (state == active_NotFading)
			{
				float env = envelope[ch].tick();
				*outP *= env;
			    localOutGain[ch] = outGain[ch].getNextValue();
				*outP *= localOutGain[ch];

				counters[ch]++;
				
				if (envelope[ch].getState() == stk::ADSR::SUSTAIN)
				{
					elapsedTime[ch] += deltaTime[ch];
					if (elapsedTime[ch] >= (envelopeADinMS + 100))
					{
						fadeInComplete[ch] = 1;
					}
				}
				if (fadeInComplete[0] == 1 && fadeInComplete[1] == 1)
				{
					state = active_Fading;
					envelope[0].keyOff();
					envelope[1].keyOff();
				}
				outP++;
			}
			else
			{
				outputSamples.applyGainRamp(0, numSamples, localOutGain[ch], 0);
			}
		}
	}

	for (int ch = 0; ch < 2; ch++)
	{
		float* outP = outputBuffer.getWritePointer(ch);
		float* inP = outputSamples.getWritePointer(ch);

		int numSamps = numSamples;

		while (numSamps--)
		{
			*outP += *inP;
			outP++;
			inP++;
		}	
	}

	if (fadeOutComplete[0] == 1 && fadeOutComplete[1] == 1)
	{
		state = inActive;
		parentSynth->decActiveNotes();
		DBG("Stopping voice " << ID);
		checkForNonZero = true;
		elapsedTime[0] = elapsedTime[1] = 0;
		vardle->releaseResources();
	}
}

void StringSynthVoice::renderNextBlock(AudioBuffer<double>& outputBuffer, int startSample, int numSamples)
{

}

void StringSynthVoice::setCurrentPlaybackSampleRate(double newRate)
{
	sR = newRate;
	noise.setSampleRate(newRate);
	for (int i = 0; i < 2; i++)
	{
		sinOsc[i].setSampleRate(newRate);
		envelope[i].setSampleRate(newRate);
		excitation[i].setSampleRate(newRate);
	}
}

bool StringSynthVoice::isPlayingChannel(int midiChannel) const
{
	return true;
}

void StringSynthVoice::updateFreq(float newFreq)
{
	if (newFreq > 0.0f && newFreq < 15000.0f)
	{
		vardle->setParameter(VarDelay::freqP, newFreq);
	}
}

void StringSynthVoice::updateFdbk(float newFdbk)
{
	if (newFdbk >= 0.0 && newFdbk < 1.0)
	{
		vardle->setParameter(VarDelay::feedbackP, newFdbk);
	}
}

void StringSynthVoice::prepare(int samplesPerBlockExpected, double sampleRate)
{
	sR = sampleRate;
	scratchInputBuffers.setSize(2, samplesPerBlockExpected);
	scratchOutputBuffers.setSize(2, samplesPerBlockExpected);

	vardle->prepareToPlay(samplesPerBlockExpected, sampleRate);
	
	sR = sampleRate;
	noise.setSampleRate(sampleRate);

	for (int i = 0; i < 2; i++)
	{
		outGain[i].reset(sampleRate, 0.01);
		sinOsc[i].setSampleRate(sampleRate);
		envelope[i].setSampleRate(sampleRate);
		excitation[i].setSampleRate(sampleRate);
	}
}


void StringSynthVoice::release()
{
	DBG("release");
	//excitation[0].keyOff();
	//excitation[1].keyOff();
	//envelope[0].keyOff();
	//envelope[1].keyOff();

	//counter[0] = counter[1] = 0;
	//state = inActive;
	//clearCurrentNote();
}

//---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++//


Synth::Synth()
{
	activeNotes = 0;
}

Synth::~Synth()
{
}

void Synth::renderVoices(AudioBuffer<float>& buffer, int startSample, int numSamples) 
{
	for (auto* voice : voices)
	{
		voice->renderNextBlock(buffer, startSample, numSamples);
	}
}

void Synth::noteOn(int midiChannel, int midiNoteNumber, float velocity)
{
	const ScopedLock sl(lock);

	for (auto* sound : sounds)
	{
		if (sound->appliesToNote(midiNoteNumber) && sound->appliesToChannel(midiChannel))
		{
			// If hitting a note that's still ringing, stop it first (it could be
			// still playing because of the sustain or sostenuto pedal).
			//for (auto* voice : voices)
			//	if (voice->getCurrentlyPlayingNote() == midiNoteNumber && voice->isPlayingChannel(midiChannel))
			//		stopVoice(voice, 1.0f, true);

			bool steal = isNoteStealingEnabled();
			
			startVoice(findFreeVoice(sound, midiChannel, midiNoteNumber, steal),
				sound, midiChannel, midiNoteNumber, velocity);
		}
	}
}

//void Synth::noteOff(int midiChannel, int midiNoteNumber, float velocity, bool allowTailOff)
//{
//
//}
//
//void Synth::allNotesOff(int midiChannel, bool allowTailOff)
//{
//
//}
//
//void Synth::handleController(int midiChannel, int controllerNumber, int controllerValue)
//{
//
//}
//
//void Synth::handleChannelPressure(int midiChannel, int channelPressureValue)
//{
//
//}
//
//void Synth::handleSustainPedal(int midiChannel, bool isDown)
//{
//
//}
//
//void Synth::handleSostenutoPedal(int midiChannel, bool isDown)
//{
//
//}
//
//void Synth::handleSoftPedal(int midiChannel, bool isDown)
//{
//
//}
//
//void Synth::handleProgramChange(int midiChannel, int programNumber)
//{
//
//}

void Synth::startNoteAtFreq(float frequency)
{
	float closestNote = 12.0f * log2(frequency / 440.0f) + 69.0f;
	noteOn(1, closestNote, 1);
}

bool Synth::noteOnForVoice(int index, float frequency)
{
	StringSynthVoice* voice = reinterpret_cast<StringSynthVoice*> (getVoice(index));

	if (voice->isVoiceActive())
	{
		//voice->startNoteWhenFinished(frequency, 1.0f, &synthSound);
		return false;
	}
	else
	{
		voice->startNoteAtFreq(frequency, 1.0f);
		//float closestNote = 12.0f * log2(frequency / 440.0f) + 69.0f;
		//voice->startNote(closestNote, 1.0f, &synthSound, 0);
	}
}

void Synth::audioOff()
{
	for (int v = 0; v < getNumVoices(); v++)
	{
		StringSynthVoice* voice = reinterpret_cast<StringSynthVoice*> (getVoice(v));

		if (voice != nullptr)
		{
			voice->release();
		}
		//allNotesOff(1, true);
	}
}

void Synth::updateVoiceFreq(int index, float newFreq)
{
	StringSynthVoice* voice = reinterpret_cast<StringSynthVoice*> (getVoice(index));

	if (voice != nullptr) { voice->updateFreq(newFreq); }
}

void Synth::updateVoiceFdbk(int index, float newFdbk)
{
	StringSynthVoice* voice = reinterpret_cast<StringSynthVoice*> (getVoice(index));

	if (voice != nullptr) { voice->updateFdbk(newFdbk); }
}

void Synth::rememberNote(int note)
{
	lastNote = note;
}

int Synth::getLastNote()
{
	return lastNote;
}

void Synth::incActiveNotes()
{
	activeNotes++;
}

void Synth::decActiveNotes()
{
	activeNotes--;
	if (activeNotes < 0)
	{
		activeNotes = 0;
	}
}

int Synth::getNumActiveNotes()
{
	return activeNotes;
}