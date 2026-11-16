#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class VelvetAfterglowAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    VelvetAfterglowAudioProcessorEditor(VelvetAfterglowAudioProcessor&);
    ~VelvetAfterglowAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    VelvetAfterglowAudioProcessor& audioProcessor;

    juce::Slider cutoffSlider, resonanceSlider, attackSlider, decaySlider, sustainSlider, releaseSlider, vibratoSlider, masterSlider;
    juce::Label cutoffLabel, resonanceLabel, attackLabel, decayLabel, sustainLabel, releaseLabel, vibratoLabel, masterLabel;
    juce::Label titleLabel;

    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<Attachment> cutoffAttachment, resonanceAttachment, attackAttachment, decayAttachment, sustainAttachment, releaseAttachment, vibratoAttachment, masterAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VelvetAfterglowAudioProcessorEditor)
};
