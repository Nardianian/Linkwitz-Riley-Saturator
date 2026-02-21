/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
LR_SaturatorAudioProcessorEditor::LR_SaturatorAudioProcessorEditor (LR_SaturatorAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);
    crossoverSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    crossoverSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible(crossoverSlider);
    crossoverAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "crossover", crossoverSlider);

    gainSlider.setSliderStyle(juce::Slider::LinearVertical);
    addAndMakeVisible(gainSlider);
    gainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "gain", gainSlider);

    mixSlider.setSliderStyle(juce::Slider::LinearVertical);
    mixSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible(mixSlider);
    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "mix", mixSlider);
}

LR_SaturatorAudioProcessorEditor::~LR_SaturatorAudioProcessorEditor()
{
}

//==============================================================================
void LR_SaturatorAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.setColour(juce::Colours::white);
    g.setFont(juce::FontOptions(18.0f));

    // Scriviamo i titoli sopra i componenti per chiarezza
    g.drawText("Crossover", 30, 20, 100, 20, juce::Justification::centred);
    g.drawText("Saturation", 150, 20, 100, 20, juce::Justification::centred);
    g.drawText("Output", 270, 20, 100, 20, juce::Justification::centred);
}

void LR_SaturatorAudioProcessorEditor::resized()
{
    // Abbiamo 400px di larghezza totale. Distribuiamo i 3 controlli in modo equo.
    auto area = getLocalBounds().reduced(20); // Margine di sicurezza di 20px ai bordi
    auto sliderWidth = 100;
    auto sliderHeight = 180;
    auto yOffset = 40; // Spazio per le label in alto

    crossoverSlider.setBounds(30, yOffset, sliderWidth, sliderHeight);
    mixSlider.setBounds(150, yOffset, sliderWidth, sliderHeight);
    gainSlider.setBounds(270, yOffset, sliderWidth, sliderHeight);
}

