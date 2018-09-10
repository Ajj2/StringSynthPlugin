/*
  ==============================================================================

    SynthPlugin.h
    Created: 4 Sep 2018 4:33:18pm
    Author:  Alex

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "Identifiers.h"
#include "VarDelay.h"


class StringSynthSound : public SynthesiserSound
{
public:
	StringSynthSound() {}
	bool appliesToNote(int midiNoteNumber) override { return true; }
	bool appliesToChannel(int midiChannel) override { return true; }
private:
};

//---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++//

class Synth : public Synthesiser
{
public:
	Synth();
	~Synth();

	//void noteOn(int midiChannel, int midiNoteNumber, float velocity) override;

	//void noteOff(int midiChannel, int midiNoteNumber, float velocity, bool allowTailOff) override;

	//void allNotesOff(int midiChannel, bool allowTailOff) override;

	//void handleController(int midiChannel, int controllerNumber, int controllerValue) override;

	//void handleChannelPressure(int midiChannel, int channelPressureValue) override;

	//void handleSustainPedal(int midiChannel, bool isDown) override;

	//void handleSostenutoPedal(int midiChannel, bool isDown) override;

	//void handleSoftPedal(int midiChannel, bool isDown) override;

	//void handleProgramChange(int midiChannel, int programNumber) override;

	void rememberNote(int note);

	int getLastNote();

	void incActiveNotes();

	void decActiveNotes();

	int getNumActiveNotes();

	void updateVoiceFreq(int index, float newFreq);

private:
	StringSynthSound synthSound;
	int activeNotes;
	int lastNote;
};

//---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++//

class StringSynthVoice : public SynthesiserVoice
{
public:
	enum State
	{
		inActive = 0,
		active_Fading,
		active_NotFading,
		numStates
	};
	StringSynthVoice(int ID_);
	~StringSynthVoice();

	void setSynthesiser(Synth* synth) { parentSynth = synth; }
	Synth* getSynthesiser() { return parentSynth; }
	
	bool canPlaySound(SynthesiserSound* ss) override;

	void startNote(int midiNoteNumber, float velocity, SynthesiserSound* sound, int currentPitchWheelPosition) override;

	void stopNote(float velocity, bool allowTailOff) override;

	bool isVoiceActive() const override;

	void pitchWheelMoved(int newPitchWheelValue) override;

	void controllerMoved(int controllerNumber, int newControllerValue) override;

	void aftertouchChanged(int newAftertouchValue) override;

	void channelPressureChanged(int newChannelPressureValue) override;

	void renderNextBlock(AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override;

	void renderNextBlock(AudioBuffer<double>& outputBuffer, int startSample, int numSamples) override;

	void setCurrentPlaybackSampleRate(double newRate)override;

	bool isPlayingChannel(int midiChannel) const override;

	int getID() { return ID; }

	int getNoteNum() { return note; }

	float getCutoff() { return cutoff; }

	void updateFreq( float newFreq );

	void prepare(int samplesPerBlockExpected, double sampleRate);
private:
	int ID;
	int note = 0;
	stk::ADSR envelope[2];
	Random randy;
	float tailOff;
	int state;
	Synth* parentSynth;

	ScopedPointer<VarDelay> vardle;

	float cutoff;
	//IIRCoefficients coeff[2];
	IIRFilter bpf[2];
	
	float phaseInc = 0;
	float phasePos[2] = { 0, 0 };
	float twoPi = M_PI * 2.0f;
};

//---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++---+++//
class Envelope : public stk::ADSR
{
public:
	enum stage
	{
		OFF,
		ATK,
		DCY,
		STN,
		RLS,
		END
	};
	int getStage() 
	{
		/*int currentStage = state_;
		float val = value_;

		if (val <= 0.0)
		{

		}*/
		return state_;
	}

private:
	int stage = OFF;
	bool finished;
};

