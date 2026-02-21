/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class LR_SaturatorAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    LR_SaturatorAudioProcessorEditor (LR_SaturatorAudioProcessor&);
    ~LR_SaturatorAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    LR_SaturatorAudioProcessor& audioProcessor;

    juce::Slider crossoverSlider, gainSlider, mixSlider;
    juce::Label crossoverLabel, gainLabel, mixLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> crossoverAttachment, gainAttachment, mixAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LR_SaturatorAudioProcessorEditor)
};
