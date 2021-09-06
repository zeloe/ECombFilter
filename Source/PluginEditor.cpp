/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ECombFilterAudioProcessorEditor::ECombFilterAudioProcessorEditor (ECombFilterAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
dryWetAttachment(audioProcessor.apvts,"DryWet", dryWetSlider),
hzAttachment(audioProcessor.apvts, "Hz", hzSlider),
gainAttachment(audioProcessor.apvts, "Gain", gainSlider)
{
    for (auto* comp : getComps()){
        addAndMakeVisible(comp);
    }
    setSize (400, 400);
    hzLabel.setText("Hz", juce::dontSendNotification);
    gainLabel.setText("Gain", juce::dontSendNotification);
    dryWetLabel.setText("DryWet", juce::dontSendNotification);
}

ECombFilterAudioProcessorEditor::~ECombFilterAudioProcessorEditor()
{
}

//==============================================================================
void ECombFilterAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)

    g.setColour (juce::Colours::black);
    
    hzSlider.setBounds((42),
                       (110),
                       (110),
                       (104));
    gainSlider.setBounds((256),
                         (110),
                         (110),
                         (104));
    dryWetSlider.setBounds((150),
                           (210),
                           (110),
                           (104));
    hzLabel.setBounds((84),
                      (157),
                      (110),
                      (110));
    gainLabel.setBounds((293),
                        (157),
                        (110),
                        (110));
    dryWetLabel.setBounds((178),
                          (257),
                          (110),
                          (110));
}

void ECombFilterAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}
std::vector<juce::Component*> ECombFilterAudioProcessorEditor::getComps()
{
    return
    {
        &dryWetSlider,
        &hzSlider,
        &gainSlider,
        &dryWetLabel,
        &hzLabel,
        &gainLabel
    };
}
