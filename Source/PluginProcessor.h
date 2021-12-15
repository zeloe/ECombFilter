/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
const int MaxBufferDelaySize = 44100;
inline float linear_interp(float  v0, float  v1, float  t )
{
    return (1 - t) * v0 + t * v1;
};

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
    
    
        
   
    
    
     
     
    
    class Smoother
    {
        public:
        Smoother(float smoothingTimsMS, int sampleRate)
        {
            a = exp(-TWOPI / (smoothingTimsMS * 0.001f * sampleRate));
            b = 1.0f -a;
            z = 0;
        };
        ~Smoother()
        {
            
        };
       
        inline float process(float input)
        {
            z = (input * b) + (z * a);
            return z;
        };
        private:
        float a;
        float b;
        float z;
        const float TWOPI = 6.283185307179586476925286766559;
        
    };
    
    struct smoothValue
    {
        float targetValue;
        float currentValue;
        Smoother* smoother;
    };
    
    float mBufferL[44101] = {0};
    float mBufferR[44101] = {0};

    float getInterpolatedSample(float inDelayTimeInSamples, float mBuffer[], int MaxBufferDelaySize,int index);
private:
     
    float leftChannel, rightChannel;
    float msperiod, t_60, gain_coefficient, dcoffset;
    smoothValue hzValues, dryWetValues, gainCoefficientValues, scaleValues;// delayTimeInSamplesValues;
    int delayTimeInSampels = 0, mDelayIndexL = 0, mDelayIndexR = 0;
     float channelL = 0, channelR = 0;
     float FeedbackSampleL = 0 ,FeedbackSampleR = 0;
    float FeedforwardSampleL = 0, FeedforwardSampleR = 0; 
     float sample_y0L  = 0;
     float sample_y1L = 0 ;
     float sample_y0R = 0;
     float sample_y1R = 0 ;
     float t = 0;
     int index_y0 = 0 ;
    int index_y1 = 0;
    float samplerate;
    double readPosition = 0;
    float dataForDelayL = 0, dataForDelayR = 0;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ECombFilterAudioProcessor)
};
