#pragma once

#include <JuceHeader.h>

class VelvetVoice : public juce::SynthesiserVoice
{
public:
    VelvetVoice();

    bool canPlaySound(juce::SynthesiserSound*) override { return true; }

    void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound*, int currentPitchWheelPosition) override;
    void stopNote(float velocity, bool allowTailOff) override;
    void pitchWheelMoved(int newPitchWheelValue) override {}
    void controllerMoved(int controllerNumber, int newControllerValue) override {}

    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override;

    void setSampleRate(double rate);
    void setParameters(float cutoff, float resonance, float attack, float decay, float sustain, float release, float vibratoDepth);
    void setAftertouch(float value);

private:
    double sampleRate = 44100.0;
    double phase1 = 0.0, phase2 = 0.0;
    double phaseIncrement1 = 0.0, phaseIncrement2 = 0.0;
    float velocity = 0.0f;
    float aftertouch = 0.0f;
    float currentCutoff = 1000.0f;
    float currentRes = 0.5f;
    float vibDepth = 0.2f;

    juce::ADSR adsr;
    juce::ADSR::Parameters adsrParams;

    // Simple one-pole lowpass
    float filterState = 0.0f;
    float filterCoeff = 0.5f;

    void updateFilterCoeff();
    float processFilter(float input);
};

class VelvetAfterglowAudioProcessor : public juce::AudioProcessor
{
public:
    VelvetAfterglowAudioProcessor();
    ~VelvetAfterglowAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    juce::Synthesiser synth;
    juce::AudioProcessorValueTreeState apvts;

    juce::dsp::Limiter<float> limiter;
    juce::SmoothedValue<float> aftertouchSmoothed{0.0f};
    juce::SmoothedValue<float> masterGainSmoothed{0.8f};

    std::atomic<float>* cutoffParam = nullptr;
    std::atomic<float>* resonanceParam = nullptr;
    std::atomic<float>* attackParam = nullptr;
    std::atomic<float>* decayParam = nullptr;
    std::atomic<float>* sustainParam = nullptr;
    std::atomic<float>* releaseParam = nullptr;
    std::atomic<float>* vibratoParam = nullptr;
    std::atomic<float>* masterParam = nullptr;

    double currentSampleRate = 44100.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VelvetAfterglowAudioProcessor)
};
