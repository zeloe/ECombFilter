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
    for(int i = 0; i < 44101; ++i)
    {
        mBufferL[i] = 0;
        mBufferR[i] = 0;
    }
    dryWetValues.smoother = new Smoother(1, sampleRate);
    hzValues.smoother = new Smoother(1, sampleRate);
    gainCoefficientValues.smoother = new Smoother(1, sampleRate);
    scaleValues.smoother = new Smoother(1,sampleRate);
    mDelayIndex = 0;
   
}

void ECombFilterAudioProcessor::releaseResources()
{
    juce::zeromem(mBufferL, sizeof(double) * 44001);
    juce::zeromem(mBufferR, sizeof(double) * 44001);
}
#ifndef JucePlugin_PreferredChannelConfigurations
bool ECombFilterAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    /*
    if (layouts.getMainInputChannelSet()  == juce::AudioChannelSet::disabled()
     || layouts.getMainOutputChannelSet() == juce::AudioChannelSet::disabled())
        return false;
 
    if //  &&(layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono())
    (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
 
    return layouts.getMainInputChannelSet() == layouts.getMainOutputChannelSet();
    */
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
    
    dryWetValues.targetValue = apvts.getRawParameterValue("DryWet")->load();
    dryWetValues.currentValue = dryWetValues.smoother->process(dryWetValues.targetValue);
   

    hzValues.targetValue = apvts.getRawParameterValue("Delay")->load();
    hzValues.currentValue = hzValues.smoother->process(hzValues.targetValue);

    gainCoefficientValues.targetValue = apvts.getRawParameterValue("Gain")->load();
    gainCoefficientValues.currentValue = gainCoefficientValues.smoother->process(gainCoefficientValues.targetValue);
    dcoffset = 1.0f - dryWetValues.currentValue;

    t_60 = t60(gainCoefficientValues.currentValue, hzValues.currentValue);
    scaleValues.currentValue = scaleValues.smoother->process(gaincoefficient(hzValues.currentValue, t_60));
     
    const int buffersampels = buffer.getNumSamples();
        auto* inDataL = buffer.getWritePointer(0);
        auto* outDataL = buffer.getWritePointer(0);
        auto* outDataR = buffer.getWritePointer(1);
        auto* inDataR = buffer.getWritePointer(1);
        for(int i = 0; i < buffersampels; i++)
        {
            readPosition = (double)mDelayIndex - hzValues.currentValue;
           
            if (readPosition < 0.0f) {
                readPosition = readPosition + MaxBufferDelaySize;
            }
             index_y0 = (int)readPosition - 1;
            if(index_y0 <= 0){
                index_y0 = index_y0 + MaxBufferDelaySize;
            }
             index_y1 = readPosition;
            
            if(index_y1 > MaxBufferDelaySize){
                index_y1 = index_y1 - MaxBufferDelaySize;
            }
            
             sample_y0L = mBufferL[index_y0];
             sample_y1L = mBufferL[index_y1];
            sample_y0R = mBufferR[index_y0];
             sample_y1R = mBufferR[index_y1];
             t = readPosition - (int)readPosition;
            
            FeedbackSampleL = linear_interp(sample_y0L, sample_y1L, t);
            FeedbackSampleR = linear_interp(sample_y0R, sample_y1R, t);
            mDelayIndex += 1;
            mBufferL[mDelayIndex] = inDataL[i] + channelL ;
           
            channelL = dcoffset *  inDataL[i] +FeedbackSampleL * dryWetValues.currentValue * scaleValues.currentValue * gainCoefficientValues.currentValue *0.95f;
            outDataL[i] = channelL;
           
         
           
            
            mBufferR[mDelayIndex] = inDataR[i] + channelR;
            channelR  = dcoffset * inDataR[i] +FeedbackSampleR * dryWetValues.currentValue * scaleValues.currentValue * gainCoefficientValues.currentValue * 0.95f;
            outDataR[i] = channelR;
            
            if (mDelayIndex >= MaxBufferDelaySize){
                mDelayIndex -= MaxBufferDelaySize;
                }
             
       
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
    layout.add(std::make_unique<juce::AudioParameterInt>("Delay",
                                                         "Delay",
                                                          5,
                                                          2000,
                                                          100));
    layout.add(std::make_unique<juce::AudioParameterFloat>("Gain",
                                                           "Gain",
                                                           0.1f,
                                                           0.95f,
                                                           0.25));
    return layout;
    
               
    
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ECombFilterAudioProcessor();
}
