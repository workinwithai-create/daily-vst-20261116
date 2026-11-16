#include "PluginEditor.h"

VelvetAfterglowAudioProcessorEditor::VelvetAfterglowAudioProcessorEditor(VelvetAfterglowAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setSize(700, 450);

    // Dark cinematic colors
    auto darkBg = juce::Colour(0xFF0D0D1A);
    auto accent = juce::Colour(0xFF8B5CF6);
    auto textCol = juce::Colour(0xFFE0E0E0);

    titleLabel.setText("Velvet Afterglow", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(28.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, accent);
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);

    auto setupSlider = [&](juce::Slider& slider, juce::Label& label, const juce::String& name, const juce::String& paramId)
    {
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 20);
        slider.setColour(juce::Slider::rotarySliderFillColourId, accent);
        slider.setColour(juce::Slider::thumbColourId, juce::Colours::white);
        addAndMakeVisible(slider);

        label.setText(name, juce::dontSendNotification);
        label.setFont(juce::Font(14.0f));
        label.setColour(juce::Label::textColourId, textCol);
        label.setJustificationType(juce::Justification::centred);
        label.attachToComponent(&slider, false);
        addAndMakeVisible(label);
    };

    setupSlider(cutoffSlider, cutoffLabel, "Cutoff", "cutoff");
    setupSlider(resonanceSlider, resonanceLabel, "Resonance", "resonance");
    setupSlider(attackSlider, attackLabel, "Attack", "attack");
    setupSlider(decaySlider, decayLabel, "Decay", "decay");
    setupSlider(sustainSlider, sustainLabel, "Sustain", "sustain");
    setupSlider(releaseSlider, releaseLabel, "Release", "release");
    setupSlider(vibratoSlider, vibratoLabel, "Vibrato", "vibrato");
    setupSlider(masterSlider, masterLabel, "Master", "master");

    cutoffAttachment = std::make_unique<Attachment>(audioProcessor.getAPVTS(), "cutoff", cutoffSlider);
    resonanceAttachment = std::make_unique<Attachment>(audioProcessor.getAPVTS(), "resonance", resonanceSlider);
    attackAttachment = std::make_unique<Attachment>(audioProcessor.getAPVTS(), "attack", attackSlider);
    decayAttachment = std::make_unique<Attachment>(audioProcessor.getAPVTS(), "decay", decaySlider);
    sustainAttachment = std::make_unique<Attachment>(audioProcessor.getAPVTS(), "sustain", sustainSlider);
    releaseAttachment = std::make_unique<Attachment>(audioProcessor.getAPVTS(), "release", releaseSlider);
    vibratoAttachment = std::make_unique<Attachment>(audioProcessor.getAPVTS(), "vibrato", vibratoSlider);
    masterAttachment = std::make_unique<Attachment>(audioProcessor.getAPVTS(), "master", masterSlider);
}

VelvetAfterglowAudioProcessorEditor::~VelvetAfterglowAudioProcessorEditor() {}

void VelvetAfterglowAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xFF0D0D1A));

    // Subtle cinematic gradient
    juce::ColourGradient gradient(juce::Colour(0xFF1A1A2E), 0, 0, juce::Colour(0xFF0A0A14), 0, static_cast<float>(getHeight()), false);
    g.setGradientFill(gradient);
    g.fillAll();

    g.setColour(juce::Colour(0xFF8B5CF6).withAlpha(0.15f));
    g.fillEllipse(getWidth() * 0.7f, -50.0f, 300.0f, 300.0f);
}

void VelvetAfterglowAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(20);
    titleLabel.setBounds(area.removeFromTop(50));

    auto sliderArea = area.reduced(10);
    int sliderW = 80;
    int gap = 15;
    int totalW = 8 * sliderW + 7 * gap;
    int startX = (getWidth() - totalW) / 2;
    int y = area.getY() + 40;

    auto place = [&](juce::Slider& s) {
        s.setBounds(startX, y, sliderW, sliderW + 30);
        startX += sliderW + gap;
    };

    place(cutoffSlider);
    place(resonanceSlider);
    place(attackSlider);
    place(decaySlider);
    place(sustainSlider);
    place(releaseSlider);
    place(vibratoSlider);
    place(masterSlider);
}
