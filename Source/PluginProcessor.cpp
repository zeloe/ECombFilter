/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ECombFilterAudioProcessor::ECombFilterAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

ECombFilterAudioProcessor::~ECombFilterAudioProcessor()
{
}

//==============================================================================
const juce::String ECombFilterAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ECombFilterAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ECombFilterAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ECombFilterAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ECombFilterAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ECombFilterAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ECombFilterAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ECombFilterAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String ECombFilterAudioProcessor::getProgramName (int index)
{
    return {};
}

void ECombFilterAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void ECombFilterAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
        spec.maximumBlockSize = samplesPerBlock;
        
        spec.numChannels = 1;
        
        spec.sampleRate = sampleRate;
        
      
        
        ChainStageL1.prepare(spec);
        ChainStageR1.prepare(spec);
}

void ECombFilterAudioProcessor::releaseResources()
{
    ChainStageL1.reset();
    ChainStageR1.reset();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ECombFilterAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
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

void ECombFilterAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* inDataL = buffer.getReadPointer(0);
        auto* inDataR = buffer.getReadPointer(1);
        auto* outDataL = buffer.getWritePointer(0);
        auto* outDataR = buffer.getWritePointer(1);
        auto bufferSampels = buffer.getNumSamples();
        dryWetSmoothed = smoothing(dryWetSmoothed, apvts.getRawParameterValue("DryWet")->load());
        dcoffset = 1.0f - dryWetSmoothed;
        hzSmoothed = smoothing(hzSmoothed, apvts.getRawParameterValue("Hz")->load());
        gainSmoothed = smoothing(gainSmoothed, apvts.getRawParameterValue("Gain")->load());
       
       
        msperiod = periodinms(hzSmoothed);
        delayTimeInSampels =  ((msperiod*getSampleRate()/1000));
        t_60 = t60(gainSmoothed, delayTimeInSampels);
        gain_coefficient = gaincoefficient(delayTimeInSampels, t_60);
        for(int i = 0; i < bufferSampels; i++)
        {
            
            stage1L = (inDataL[i] *gain_coefficient) +ChainStageL1.popSample(0, static_cast<int>(hzSmoothed)) *gain_coefficient * 0.95f;
            ChainStageL1.pushSample(0, stage1L);
            outDataL[i] = dcoffset * (inDataL[i]) + dryWetSmoothed * stage1L;
        stage1R = (inDataR[i] *gain_coefficient) + ChainStageR1.popSample(0, static_cast<int>(hzSmoothed))  *gain_coefficient * 0.95f;
        ChainStageR1.pushSample(0, stage1R);
        outDataR[i] =  dcoffset * (inDataR[i]) + dryWetSmoothed * stage1R;
        }
        // ..do something to the data...
    }
}

//==============================================================================
bool ECombFilterAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* ECombFilterAudioProcessor::createEditor()
{
    return new ECombFilterAudioProcessorEditor (*this);
}

//==============================================================================
void ECombFilterAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream mos (destData, true);
        apvts.state.writeToStream(mos);
}

void ECombFilterAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto tree = juce::ValueTree::readFromData(data, sizeInBytes);
           if(tree.isValid() )
           {
               apvts.replaceState(tree);
           }
}

juce::AudioProcessorValueTreeState::ParameterLayout
ECombFilterAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add(std::make_unique<juce::AudioParameterFloat>("DryWet",
                                                           "DryWet",
                                                            0.f,
                                                            1.f,
                                                           0.5f));
    layout.add(std::make_unique<juce::AudioParameterInt>("Hz",
                                                         "Hz",
                                                          20,
                                                          2000,
                                                          100));
    layout.add(std::make_unique<juce::AudioParameterFloat>("Gain",
                                                           "Gain",
                                                           0.1f,
                                                           0.55f,
                                                           0.25));
    return layout;
    
               
    
}
//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ECombFilterAudioProcessor();
}
