#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
VelvetVoice::VelvetVoice()
{
    adsrParams.attack = 0.1f;
    adsrParams.decay = 0.3f;
    adsrParams.sustain = 0.7f;
    adsrParams.release = 0.5f;
    adsr.setParameters(adsrParams);
}

void VelvetVoice::setSampleRate(double rate)
{
    sampleRate = rate;
    adsr.setSampleRate(rate);
    updateFilterCoeff();
}

void VelvetVoice::setParameters(float cutoff, float resonance, float attack, float decay, float sustain, float release, float vibratoDepth)
{
    currentCutoff = cutoff;
    currentRes = juce::jlimit(0.0f, 0.95f, resonance);
    vibDepth = vibratoDepth;
    adsrParams.attack = attack;
    adsrParams.decay = decay;
    adsrParams.sustain = sustain;
    adsrParams.release = release;
    adsr.setParameters(adsrParams);
    updateFilterCoeff();
}

void VelvetVoice::setAftertouch(float value)
{
    aftertouch = juce::jlimit(0.0f, 1.0f, value);
}

void VelvetVoice::updateFilterCoeff()
{
    float fc = juce::jlimit(20.0f, 18000.0f, currentCutoff * (0.5f + aftertouch * 0.5f));
    float q = 0.5f + currentRes * 2.0f;
    // Simple bilinear approx for one-pole
    float omega = 2.0f * juce::MathConstants<float>::pi * fc / static_cast<float>(sampleRate);
    filterCoeff = juce::jlimit(0.01f, 0.99f, omega / (omega + 1.0f));
}

float VelvetVoice::processFilter(float input)
{
    filterState += filterCoeff * (input - filterState);
    return filterState;
}

void VelvetVoice::startNote(int midiNoteNumber, float vel, juce::SynthesiserSound*, int)
{
    velocity = vel;
    double freq = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
    phaseIncrement1 = freq / sampleRate;
    phaseIncrement2 = (freq * 1.005) / sampleRate; // slight detune
    phase1 = 0.0;
    phase2 = 0.0;
    adsr.noteOn();
    filterState = 0.0f;
}

void VelvetVoice::stopNote(float, bool allowTailOff)
{
    adsr.noteOff();
    if (!allowTailOff || !adsr.isActive())
        clearCurrentNote();
}

void VelvetVoice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
    if (!adsr.isActive())
    {
        clearCurrentNote();
        return;
    }

    for (int i = 0; i < numSamples; ++i)
    {
        // Vibrato from aftertouch
        float vib = std::sin(phase1 * 6.0f * juce::MathConstants<float>::twoPi) * vibDepth * aftertouch * 0.02f;
        double freqMod = phaseIncrement1 * (1.0 + vib);

        float s1 = std::sin(phase1 * juce::MathConstants<float>::twoPi);
        float s2 = std::sin(phase2 * juce::MathConstants<float>::twoPi) * 0.7f;
        float sample = (s1 + s2) * 0.5f * velocity;

        sample = processFilter(sample);
        sample *= adsr.getNextSample();

        for (int ch = 0; ch < outputBuffer.getNumChannels(); ++ch)
            outputBuffer.addSample(ch, startSample + i, sample);

        phase1 += freqMod;
        if (phase1 >= 1.0) phase1 -= 1.0;
        phase2 += phaseIncrement2;
        if (phase2 >= 1.0) phase2 -= 1.0;
    }

    if (!adsr.isActive())
        clearCurrentNote();
}

//==============================================================================
VelvetAfterglowAudioProcessor::VelvetAfterglowAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    for (int i = 0; i < 16; ++i)
        synth.addVoice(new VelvetVoice());

    synth.addSound(new juce::SynthesiserSound());

    cutoffParam = apvts.getRawParameterValue("cutoff");
    resonanceParam = apvts.getRawParameterValue("resonance");
    attackParam = apvts.getRawParameterValue("attack");
    decayParam = apvts.getRawParameterValue("decay");
    sustainParam = apvts.getRawParameterValue("sustain");
    releaseParam = apvts.getRawParameterValue("release");
    vibratoParam = apvts.getRawParameterValue("vibrato");
    masterParam = apvts.getRawParameterValue("master");
}

VelvetAfterglowAudioProcessor::~VelvetAfterglowAudioProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout VelvetAfterglowAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterFloat>("cutoff", "Cutoff", juce::NormalisableRange<float>(100.0f, 12000.0f, 1.0f, 0.4f), 2000.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("resonance", "Resonance", 0.0f, 0.9f, 0.3f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("attack", "Attack", 0.01f, 2.0f, 0.1f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("decay", "Decay", 0.01f, 2.0f, 0.4f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("sustain", "Sustain", 0.0f, 1.0f, 0.7f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("release", "Release", 0.01f, 4.0f, 0.8f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("vibrato", "Vibrato Depth", 0.0f, 1.0f, 0.3f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("master", "Master", 0.0f, 1.0f, 0.8f));

    return layout;
}

void VelvetAfterglowAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    synth.setCurrentPlaybackSampleRate(sampleRate);

    for (int i = 0; i < synth.getNumVoices(); ++i)
        if (auto* voice = dynamic_cast<VelvetVoice*>(synth.getVoice(i)))
            voice->setSampleRate(sampleRate);

    juce::dsp::ProcessSpec spec{ sampleRate, static_cast<juce::uint32>(samplesPerBlock), static_cast<juce::uint32>(getTotalNumOutputChannels()) };
    limiter.prepare(spec);
    limiter.setThreshold(-1.0f);
    limiter.setRelease(50.0f);

    aftertouchSmoothed.reset(sampleRate, 0.05);
    masterGainSmoothed.reset(sampleRate, 0.05);
}

void VelvetAfterglowAudioProcessor::releaseResources() {}

void VelvetAfterglowAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    // Handle aftertouch (channel pressure)
    for (const auto metadata : midiMessages)
    {
        auto msg = metadata.getMessage();
        if (msg.isChannelPressure())
            aftertouchSmoothed.setTargetValue(msg.getChannelPressureValue() / 127.0f);
    }

    aftertouchSmoothed.skip(buffer.getNumSamples());
    float currentAftertouch = aftertouchSmoothed.getCurrentValue();

    for (int i = 0; i < synth.getNumVoices(); ++i)
        if (auto* voice = dynamic_cast<VelvetVoice*>(synth.getVoice(i)))
            voice->setAftertouch(currentAftertouch);

    // Update parameters on voices (after aftertouch so filter can use it)
    float cutoff = cutoffParam->load();
    float res = resonanceParam->load();
    float att = attackParam->load();
    float dec = decayParam->load();
    float sus = sustainParam->load();
    float rel = releaseParam->load();
    float vib = vibratoParam->load();

    for (int i = 0; i < synth.getNumVoices(); ++i)
        if (auto* voice = dynamic_cast<VelvetVoice*>(synth.getVoice(i)))
            voice->setParameters(cutoff, res, att, dec, sus, rel, vib);

    synth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());

    // Master gain
    float master = masterParam->load();
    masterGainSmoothed.setTargetValue(master);
    masterGainSmoothed.skip(buffer.getNumSamples());
    buffer.applyGain(masterGainSmoothed.getCurrentValue());

    // Safety limiter
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    limiter.process(context);
}

juce::AudioProcessorEditor* VelvetAfterglowAudioProcessor::createEditor()
{
    return new VelvetAfterglowAudioProcessorEditor(*this);
}

void VelvetAfterglowAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void VelvetAfterglowAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr)
        if (xmlState->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new VelvetAfterglowAudioProcessor();
}
