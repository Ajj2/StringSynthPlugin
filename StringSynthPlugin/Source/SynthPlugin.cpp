/*
  ==============================================================================

    SynthPlugin.cpp
    Created: 4 Sep 2018 4:33:18pm
    Author:  Alex

  ==============================================================================
*/

#include "SynthPlugin.h"


StringSynthVoice::StringSynthVoice() 
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
	state = active_NotFading;
	//synth->incActiveNotes();
	envelope[0].keyOn();
	envelope[1].keyOn();
}

void StringSynthVoice::stopNote(float velocity, bool allowTailOff)
{
	//synth->decActiveNotes();
	tailOff = 1.0;
	state = active_Fading;
	envelope[0].keyOff();
	envelope[1].keyOff();
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

	static int counter = 0;
	counter++;

	for (int ch = 0; ch < numOutChannels; ch++)
	{
		int numSamp = numSamples;
		float* outP = outputBuffer.getWritePointer(ch);
		while (numSamp--)
		{
			if (state == active_Fading)
			{
				tailOff *= 0.9999;
				float out = ((randy.nextFloat() * 2.0) - 1.0) * 0.2 * tailOff;
				*outP = out;
				outP++;
				if (tailOff < 0.005)
				{
					clearCurrentNote();
					tailOff = 0.0;
					state = inActive;
				}
			}
			else if (state == active_NotFading)
			{
				float envValue = envelope[ch].tick();

				*outP = ((randy.nextFloat() * 2.0) - 1.0) * 0.2;
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
	envelope->setSampleRate(newRate);
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