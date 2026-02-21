/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
LR_SaturatorAudioProcessor::LR_SaturatorAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
    ), apvts(*this, nullptr, "Parameters", createParameterLayout())
{
}

LR_SaturatorAudioProcessor::~LR_SaturatorAudioProcessor()
{
}

//==============================================================================
const juce::String LR_SaturatorAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool LR_SaturatorAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool LR_SaturatorAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool LR_SaturatorAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double LR_SaturatorAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int LR_SaturatorAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int LR_SaturatorAudioProcessor::getCurrentProgram()
{
    return 0;
}

void LR_SaturatorAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String LR_SaturatorAudioProcessor::getProgramName (int index)
{
    return {};
}

void LR_SaturatorAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void LR_SaturatorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback initialisation that you need..
    saturator.Init();
    saturator.SetMaxChannels(2);
    saturator.SetMaxBlockSize(samplesPerBlock);
    saturator.SetCrossoverFrequency(5000.f);
    saturator.SetSampleRate(sampleRate);
    saturator.SetGain(1.f);
}

void LR_SaturatorAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    saturator.Release();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool LR_SaturatorAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported. In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void LR_SaturatorAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    auto numSamples = buffer.getNumSamples();
    auto numChannels = buffer.getNumChannels();

    float currentFreq = *apvts.getRawParameterValue("crossover");
    float currentGain = *apvts.getRawParameterValue("gain");
    saturator.SetCrossoverFrequency(currentFreq);
    saturator.SetGain(currentGain);

    // In case we have more outputs than inputs, this code clears any output channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage). This is here to avoid people getting screaming feedback when they first compile
    // a plugin, but obviously you don't need to keep this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's audio processing...
    // Make sure to reset the state if your inner loop is processing the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels interleaved by keeping the same state.
    
    auto writeData = const_cast<float **>(buffer.getArrayOfWritePointers());
    saturator.Process(writeData, totalNumInputChannels, numSamples);
}

//==============================================================================
bool LR_SaturatorAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* LR_SaturatorAudioProcessor::createEditor()
{
    return new LR_SaturatorAudioProcessorEditor (*this);
}

//==============================================================================
void LR_SaturatorAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes as intermediaries to make it easy to save and load complex data.
}

void LR_SaturatorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block, whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LR_SaturatorAudioProcessor();
}

juce::AudioProcessorValueTreeState::ParameterLayout LR_SaturatorAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("crossover", 1), "Crossover", juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.5f), 500.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("gain", 1), "Gain", 0.0f, 2.0f, 1.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("mix", 1), "Mix Saturation", 0.0f, 1.0f, 0.5f));
    return layout;
}

