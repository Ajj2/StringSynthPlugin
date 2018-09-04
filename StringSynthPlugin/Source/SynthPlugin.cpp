/*
  ==============================================================================

    SynthPlugin.cpp
    Created: 4 Sep 2018 4:33:18pm
    Author:  Alex

  ==============================================================================
*/

#include "SynthPlugin.h"


StringSynthVoice::StringSynthVoice() : silent(true)
{
	for (int ch = 0; ch < 2; ch++)
	{
		envelope[ch].setAttackTime(0.1);
		envelope[ch].setDecayTime(0.05);
		envelope[ch].setSustainLevel(0.9);
		envelope[ch].setReleaseTime(2);
	}
	randy.setSeedRandomly();
	tailOff = 0.0;
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
	//synth->incActiveNotes();
	silent = false;
	for (int ch = 0; ch < 2; ch++)
	{
		envelope[ch].keyOn();
	}
}

void StringSynthVoice::stopNote(float velocity, bool allowTailOff)
{
	//synth->decActiveNotes();
	if (allowTailOff)
	{
		if (tailOff == 0.0)
		{
			tailOff = 1.0;
		}
	}
	else
	{
		clearCurrentNote();
		silent = true;
	}
}

bool StringSynthVoice::isVoiceActive() const
{
	if (envelope[0].getState() == stk::ADSR::IDLE)
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

	for (int ch = 0; ch < numOutChannels; ch++)
	{
		int numSamp = numSamples;
		float* outP = outputBuffer.getWritePointer(ch);
		while (numSamp--)
		{
			if (tailOff > 0)
			{
				*outP = ((randy.nextFloat() * 2.0) - 1.0) * 0.2 * tailOff;

				tailOff *= 0.99;

				if (!silent && tailOff < 0.005)
				{
					clearCurrentNote();
					tailOff == 0.0;
					silent = true;
				}
			}
			else
			{
				*outP = ((randy.nextFloat() * 2.0) - 1.0) * 0.2 * envelope[ch].tick();
				outP++;
			}
		}
	}

	//if (envelope[0].getState() == stk::ADSR::IDLE) { silent = true; }
}

void StringSynthVoice::renderNextBlock(AudioBuffer<double>& outputBuffer, int startSample, int numSamples)
{

}

void StringSynthVoice::setCurrentPlaybackSampleRate(double newRate)
{

}

bool StringSynthVoice::isPlayingChannel(int midiChannel) const
{
	return true;
}

//---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++//


Synth::Synth()
{

}

Synth::~Synth()
{
}

void Synth::noteOn(int midiChannel, int midiNoteNumber, float velocity)
{

}

void Synth::noteOff(int midiChannel, int midiNoteNumber, float velocity, bool allowTailOff)
{

}

void Synth::allNotesOff(int midiChannel, bool allowTailOff)
{

}

void Synth::handleController(int midiChannel, int controllerNumber, int controllerValue)
{

}

void Synth::handleChannelPressure(int midiChannel, int channelPressureValue)
{

}

void Synth::handleSustainPedal(int midiChannel, bool isDown)
{

}

void Synth::handleSostenutoPedal(int midiChannel, bool isDown)
{

}

void Synth::handleSoftPedal(int midiChannel, bool isDown)
{

}

void Synth::handleProgramChange(int midiChannel, int programNumber)
{

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