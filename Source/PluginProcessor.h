/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
inline float smoothing (float smoothed, float tosmooth)
{
return smoothed - 0.001 *  (smoothed-(tosmooth));
    
}

inline float periodinms (float delaytime)
{
    return (1000.0f/delaytime);
}

inline float t60 (float gaincoefficient, float period_ms)
{
    return (log(0.001)*period_ms)/gaincoefficient;
}


inline float gaincoefficient (float period_ms, float t_60)
{
    return (log(0.001) * period_ms)/t_60;
}


//==============================================================================
/**
*/
class ECombFilterAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    ECombFilterAudioProcessor();
    ~ECombFilterAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState apvts {*this, nullptr, "Parameters", createParameterLayout()};
private:
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear>
    ChainStageL1{96000},
    ChainStageR1{96000};
    float gainSmoothed, hzSmoothed, dryWetSmoothed, dcoffset, stage1L, stage1R, feedbackL, feedbackR;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ECombFilterAudioProcessor)
};
