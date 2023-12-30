/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "SincFilter.hpp"
#include "util.h"
//#include "juce_Logger.h"

//==============================================================================
LouisVerbAudioProcessor::LouisVerbAudioProcessor()
    : mBufferSize(0)
    , mSampleRate(0.0)
    , parameters (*this, nullptr, juce::Identifier ("PVT"),
    {
        std::make_unique<juce::AudioParameterFloat> ("00-allmix",       // parameterID
                                                   "Dry/Wet Mix",       // parameter name
                                                   0.000f,                // minimum value
                                                   1.000f,              // maximum value
                                                   1.000f),             // default value
        std::make_unique<juce::AudioParameterFloat> ("01-convmix",
                                                  "Convolution part-Mix",
                                                  0.000f,1.000f,1.000f),
        std::make_unique<juce::AudioParameterFloat> ("02-decay",
                                                  "Decay Time",
                                                  0.308f,0.998f,0.708f),
        std::make_unique<juce::AudioParameterInt> ("de-menuitem", "Menu Item", 1, 13, 1),
        std::make_unique<juce::AudioParameterBool> ("04-fdnon", "Modulation", false)
    })
    , irBufferSize(8192)
    , irBuffer(2,irBufferSize)
{
}

LouisVerbAudioProcessor::~LouisVerbAudioProcessor()
{
}

//==============================================================================
const juce::String LouisVerbAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool LouisVerbAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool LouisVerbAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool LouisVerbAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double LouisVerbAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int LouisVerbAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int LouisVerbAudioProcessor::getCurrentProgram()
{
    return 0;
}

void LouisVerbAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String LouisVerbAudioProcessor::getProgramName (int index)
{
    return {};
}

void LouisVerbAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

void LouisVerbAudioProcessor::chConvMixe(float val)
{
    juce::ScopedLock lock(mLoadingLock);
    mConMan[0].changeMixe(val);
    mConMan[1].changeMixe(val);
}
void LouisVerbAudioProcessor::chAllMixe(float val)
{
    dwmixratio = val;
}
void LouisVerbAudioProcessor::chLateDecay(float val){
    mLatRev[0].changeDecay(val);
    mLatRev[1].changeDecay(val);
}
void LouisVerbAudioProcessor::switchFdn(bool val){
    mLatRev[0].switchFdnMod(val);
    mLatRev[1].switchFdnMod(val);
}
void LouisVerbAudioProcessor::chPreset(paramdeviceLR& lparams, paramdeviceLR& rparams)
{
    juce::ScopedLock lock(mLoadingLock);
    //IRs
    this->loadIR(lparams.imp, rparams.imp);
    mLatRev[0].loadParams(lparams.lbcfl,lparams.gc,lparams.lpd,lparams.gk,lparams.beta,lparams.apl,lparams.ga,lparams.allg,lparams.pred);
    mLatRev[1].loadParams(rparams.lbcfl,rparams.gc,rparams.lpd,rparams.gk,rparams.beta,rparams.apl,rparams.ga,rparams.allg,rparams.pred);
}
void LouisVerbAudioProcessor::loadIR (const std::vector<float>& inL, const std::vector<float>& inR)
{
    juce::ScopedLock lock(mLoadingLock);
    
    for (int i=0;i<irBufferSize;i++)
    {
        irBuffer.setSample(0,i,inL[i]);
        irBuffer.setSample(1,i,inR[i]);
    }
    
    const float *impulseResponseL = irBuffer.getWritePointer(0);
    const float *impulseResponseR = irBuffer.getWritePointer(1);
    mConMan[0].setImpulseResponse(impulseResponseL, irBufferSize);
    mConMan[1].setImpulseResponse(impulseResponseR, irBufferSize);
}
//==============================================================================
void LouisVerbAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // initialize convolution and late reverb managers
    mConMan[0].setBufferSize(samplesPerBlock);
    mConMan[1].setBufferSize(samplesPerBlock);
    mLatRev[0].setBufferSize(samplesPerBlock);
    mLatRev[1].setBufferSize(samplesPerBlock);
    mLatRev[0].setSampleRate(sampleRate);
    mLatRev[1].setSampleRate(sampleRate);
    
    // set size of stuff
    dryBuff.setSize(2,samplesPerBlock);
    wetBuff.setSize(2,samplesPerBlock);
    outputBuff.setSize(2,samplesPerBlock);
    
}

void LouisVerbAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool LouisVerbAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (/*layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && */layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
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

void LouisVerbAudioProcessor::processBlock (juce::AudioBuffer<float>& thisBuff, juce::MidiBuffer& midiMessages)
{
    juce::ScopedTryLock tryLock(mLoadingLock);
    
    if (tryLock.isLocked())
    {
        int numsamps = thisBuff.getNumSamples();
        // copy this audio buffer block into an input buffer
        dryBuff.copyFrom(0, 0, thisBuff, 0, 0, numsamps);
        dryBuff.copyFrom(1, 0, thisBuff, 1, 0, numsamps);
        
        // push dry into dry/wet mix knob
        
        // writable pointer for this audio buffer block
        float* channelDataL = thisBuff.getWritePointer(0);
        float* channelDataR = thisBuff.getWritePointer(1);
        
        // do convolution
        mConMan[0].processInput(channelDataL);
        mConMan[1].processInput(channelDataR);
        
        // readable pointer for convolution
        // here cL/cR are already mixed by convolution's dry/wet knob
        const float* cL = mConMan[0].getOutputBuffer();
        const float* cR = mConMan[1].getOutputBuffer();
        
        // copy result from convolution to late reverb
        mLatRev[0].processInput(cL);
        mLatRev[1].processInput(cR);
        
        // readable pointer for late reverb
        const float* yL = mLatRev[0].getOutputBuffer();
        const float* yR = mLatRev[1].getOutputBuffer();
        
        // copy result from late reverb to output buffer
        wetBuff.copyFrom(0, 0, yL, thisBuff.getNumSamples());
        wetBuff.copyFrom(1, 0, yR, thisBuff.getNumSamples());
                
        // mix wet result with dry input
        dryBuff.applyGain(1-dwmixratio);
        wetBuff.applyGain(dwmixratio);
        for (int i=0; i<2; i++){
            outputBuff.copyFrom(i, 0, dryBuff, i, 0, numsamps);
            outputBuff.addFrom(i, 0, wetBuff, i, 0, numsamps);
        }
        
        // copy output buffer into this audio buffer block
        memcpy(channelDataL, outputBuff.getReadPointer(0), thisBuff.getNumSamples() * sizeof(float));
        memcpy(channelDataR, outputBuff.getReadPointer(1), thisBuff.getNumSamples() * sizeof(float));
        
    }
    else
    {
        thisBuff.clear();
    }
    
    
}

//==============================================================================
bool LouisVerbAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* LouisVerbAudioProcessor::createEditor()
{
    return new LouisVerbAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void LouisVerbAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
    
}

void LouisVerbAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
     
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
            parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
    
    
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LouisVerbAudioProcessor();
}
