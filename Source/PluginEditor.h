/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"


struct RotarySliderNoLabel : juce::Slider
{
    RotarySliderNoLabel() :juce::Slider(juce::Slider::RotaryHorizontalVerticalDrag,
                                        juce::Slider::TextEntryBoxPosition::NoTextBox)
    {
        
    }
};


//==============================================================================
/**
*/
class ECombFilterAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    ECombFilterAudioProcessorEditor (ECombFilterAudioProcessor&);
    ~ECombFilterAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    RotarySliderNoLabel dryWetSlider, hzSlider, gainSlider;
    ECombFilterAudioProcessor& audioProcessor;
    std::vector<juce::Component*> getComps();
    juce::Label dryWetLabel, hzLabel, gainLabel;
    
    using APVTS = juce::AudioProcessorValueTreeState;
    using Attachment = APVTS::SliderAttachment;
            
    Attachment dryWetAttachment,  hzAttachment, gainAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ECombFilterAudioProcessorEditor)
};
