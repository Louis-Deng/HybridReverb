/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "UniformPartitionConvolver.h"
#include "TimeDistributedFFTConvolver.h"
#include "ConvolutionManager.h"
#include "LateRevManager.h"
#include "IRStorage.h"

//==============================================================================
/**
*/
class LouisVerbAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    LouisVerbAudioProcessor();
    ~LouisVerbAudioProcessor() override;

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

    //=== custom ===
    void chAllMixe(float val);
    void chConvMixe(float val);
//    void chLateMixe(double val);
    void chPreset(paramdeviceLR& lparams, paramdeviceLR& rparams);
    //void chLateGain(double val);
    void chLateDecay(float val);   //replaces chLateGain()
    //void chLateDamp(double val);
    void switchFdn(bool val);
    
    
    
    

    
private:
    //==============================================================================
    ConvolutionManager<float> mConMan[2];
    LateRevManager<float> mLatRev[2];
    juce::CriticalSection mLoadingLock; //thread?
    int mBufferSize;
    float mSampleRate;
    juce::AudioProcessorValueTreeState parameters;
    int irBufferSize;
    juce::AudioBuffer<float> irBuffer;
    void loadIR(const std::vector<float>& impulseResponseInputL, const std::vector<float>& impulseResponseInputR);
    juce::AudioBuffer<float> dryBuff;
    juce::AudioBuffer<float> wetBuff;
    juce::AudioBuffer<float> outputBuff;
    float dwmixratio;
    //==============================================================================
    //JUCE_HEAVYWEIGHT_LEAK_DETECTOR (LouisVerbAudioProcessor)
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LouisVerbAudioProcessor)
};
