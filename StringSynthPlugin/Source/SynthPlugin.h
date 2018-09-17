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

	void noteOn(int midiChannel, int midiNoteNumber, float velocity) override;

	void renderVoices(AudioBuffer<float>& buffer, int startSample, int numSamples) override;

	//void noteOff(int midiChannel, int midiNoteNumber, float velocity, bool allowTailOff) override;

	//void allNotesOff(int midiChannel, bool allowTailOff) override;

	//void handleController(int midiChannel, int controllerNumber, int controllerValue) override;

	//void handleChannelPressure(int midiChannel, int channelPressureValue) override;

	//void handleSustainPedal(int midiChannel, bool isDown) override;

	//void handleSostenutoPedal(int midiChannel, bool isDown) override;

	//void handleSoftPedal(int midiChannel, bool isDown) override;

	//void handleProgramChange(int midiChannel, int programNumber) override;

	void startNoteAtFreq(float frequency);

	void rememberNote(int note);

	int getLastNote();

	void incActiveNotes();

	void decActiveNotes();

	int getNumActiveNotes();

	void updateVoiceFreq(int index, float newFreq);

	void updateVoiceFdbk(int index, float newFdbk);

	bool noteOnForVoice(int index, float frequency);

	void audioOff();

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

	void startNoteAtFreq(float frequency, float velocity);

	void startNoteWhenFinished(float frequency, float velocity, SynthesiserSound* sound);

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

	void updateFreq(float newFreq);

	void updateFdbk(float newFdbk);

	void prepare(int samplesPerBlockExpected, double sampleRate);

	void release();
private:
	int counters[2] = { 0, 0 };
	int ID;
	int note = 0;
	stk::ADSR envelope[2];
	float envelopeADinMS;
	float tailOff;
	int state;
	Synth* parentSynth;
	int counter[2] = { 0 , 0 };
	int sR;
	stk::ADSR excitation[2];
	AudioBuffer<float> scratchInputBuffers;
	AudioBuffer<float> scratchOutputBuffers;
	ScopedPointer<VarDelay> vardle;

	LinearSmoothedValue<float> outGain[2];

	stk::Noise noise;

	Random randy;
	float excitationNoiseGain = 0;

	float nextFreq = 0;
	float nextVelocity = 0;

	float cutoff;
	IIRFilter bpf[2];

	float prevSample[2] = { 0, 0 };
	float sampleDelta[2] = {0, 0};

	float prevTime[2] = { 0, 0 };
	float currentTime[2] = { 0, 0 };
	float deltaTime[2] = { 0, 0 };
	float elapsedTime[2] = { 0, 0 };

	int numBlockProcessed = 0;
	bool hasPassedEnv = false;

	SinOsc sinOsc[2];
	bool checkForNonZero;
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

	void reset()
	{
		value_ = 0;
		target_ = 0;
	}

private:
	int stage = OFF;
	bool finished;
};

