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
	for (int ch = 0; ch < 2; ch++)
	{
		envelope[ch].setAttackTime(2);
		envelope[ch].setDecayTime(0.05);
		envelope[ch].setSustainLevel(0.9);
		envelope[ch].setReleaseTime(2);
	}
	randy.setSeedRandomly();
	tailOff = 0.0;
	
	//sinOsc.reset();
	state = inActive;
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
	DBG("\nON = " << midiNoteNumber << " - " << ID);
	bool updateFilter = true;

	if (note == midiNoteNumber)
	{
		updateFilter = false;
	}
	note = midiNoteNumber;
	state = active_NotFading;
	parentSynth->incActiveNotes();
	envelope[0].keyOn();
	envelope[1].keyOn();

	cutoff = MidiMessage::getMidiNoteInHertz(midiNoteNumber);
	if (updateFilter)
	{
		for (int ch = 0; ch < 2; ch++)
		{
			bpf[ch].reset();
			IIRCoefficients coeff = IIRCoefficients::makeBandPass(parentSynth->getSampleRate(), cutoff, 1);
			bpf[ch].setCoefficients(coeff);
		}
	}
	for (int ch = 0; ch < 2; ch++)
	{
		phaseInc = (2 * M_PI * cutoff) / parentSynth->getSampleRate();
	}
}

void StringSynthVoice::stopNote(float velocity, bool allowTailOff)
{
	DBG("\nOFF = " << ID);
	tailOff = 1.0;
	state = active_Fading;
	envelope[0].keyOff();
	envelope[1].keyOff();

	for (int i = 0; i < 2; i++)
	{
		bpf[i].reset();
	//	sinOsc[i].reset();
	}
	/*if (allowTailOff)
	{
		if (tailOff == 0.0)
		{
			tailOff = 1.0;
		}
	}
	else
	{
		clearCurrentNote();
		state = inActive;
	}
	*/
}

bool StringSynthVoice::isVoiceActive() const
{
	if (state == inActive)
	{
		return false;
	}
	else
	{
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
	static int count = 0;

	for (int ch = 0; ch < numOutChannels; ch++)
	{
		int numSamp = numSamples;
		float* outP = outputBuffer.getWritePointer(ch);
		while (numSamp--)
		{
			if (state == active_Fading)
			{
				tailOff *= 0.99;
				phasePos[ch] += phaseInc;
				if (phasePos[ch] >= twoPi)
				{
					phasePos[ch] = 0;
				}
				float out = std::sinf(phasePos[ch]) * 0.2 * envelope[ch].tick();
				*outP += out;
				outP++;

				count++;
				if ((count % 10000) == 0 && ch == 0 && ID == 0)
				{
					DBG("state: " << state);
				}
				if (envelope[ch].getState() == Envelope::IDLE)
				{
					parentSynth->decActiveNotes();
					tailOff = 0.0;
					state = inActive;
					clearCurrentNote();
				}
			}
			else if (state == active_NotFading)
			{
				float envValue = envelope[ch].tick();
				int state = envelope[ch].getState();

				count++;
				if ((count % 10000) == 0 && ch == 0 && ID == 0)
				{
					DBG("state: " << state);
				}

				phasePos[ch] += phaseInc;
				if (phasePos[ch] >= twoPi)
				{
					phasePos[ch] = 0;
				}
				float out = std::sinf(phasePos[ch]) * 0.2 * envValue;
				*outP += out;
				outP++;
			}
		}
		//bpf[ch].processSamples(outputBuffer.getWritePointer(ch), numSamples);
	}
	//if (envelope[0].getState() == stk::ADSR::IDLE) { silent = true; }
}

void StringSynthVoice::renderNextBlock(AudioBuffer<double>& outputBuffer, int startSample, int numSamples)
{

}

void StringSynthVoice::setCurrentPlaybackSampleRate(double newRate)
{
	envelope->setSampleRate(newRate);
}

bool StringSynthVoice::isPlayingChannel(int midiChannel) const
{
	return true;
}

void StringSynthVoice::updateFreq(float newFreq)
{
	phaseInc = (2 * M_PI * newFreq) / parentSynth->getSampleRate();
	phasePos[0] = 0;
	phasePos[1] = 0;
}

//---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++//


Synth::Synth()
{
	activeNotes = 0;
}

Synth::~Synth()
{
}

//void Synth::noteOn(int midiChannel, int midiNoteNumber, float velocity)
//{
//}
//
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

void Synth::updateVoiceFreq(int index, float newFreq)
{
	StringSynthVoice* voice = reinterpret_cast<StringSynthVoice*> (getVoice(index));

	if (voice != nullptr)
	{
		voice->updateFreq(newFreq);
	}
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