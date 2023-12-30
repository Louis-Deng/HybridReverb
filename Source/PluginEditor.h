/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "RefCountedAudioBuffer.h"

//==============================================================================
/**
*/
class LouisVerbAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Slider::Listener, public juce::ComboBox::Listener, public juce::Button::Listener
{
public:
    LouisVerbAudioProcessorEditor (LouisVerbAudioProcessor&, juce::AudioProcessorValueTreeState&);
    ~LouisVerbAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void sliderValueChanged(juce::Slider*) override;
    void comboBoxChanged(juce::ComboBox*) override;
    void buttonClicked(juce::Button*) override;
    //no need for action on slider drag, setValue() already called
private:
    typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
    typedef juce::AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;
    typedef juce::AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    LouisVerbAudioProcessor& audioProcessor;
    juce::AudioProcessorValueTreeState& valueTreeState;
    
    ///overall stuff
    juce::Slider drywetMix;
    juce::Label drywetMixLabel;
    std::unique_ptr<SliderAttachment> drywetMixAtt;
    juce::Label menuSelectLabel;
    juce::ComboBox menuSelect;
    std::unique_ptr<ComboBoxAttachment> menuAtt;
    
    ///convolution stuff
    juce::Slider convMix;
    juce::Label convMixLabel;
    std::unique_ptr<SliderAttachment> convMixAtt;
    
    ///late reverb stuff
    juce::Slider decayTime;
    juce::Label decayTimeLabel;
    std::unique_ptr<SliderAttachment> decayTimeAtt;
    juce::ToggleButton fdnon;
    juce::Label fdnonLabel;
    std::unique_ptr<ButtonAttachment> fdnonAtt;
//    juce::Slider gcgainValue;
//    juce::Label gcgainValueLabel;
//    std::unique_ptr<SliderAttachment> gcgainValueAtt;
//    juce::Slider dampValue;
//    juce::Label dampValueLabel;
//    std::unique_ptr<SliderAttachment> dampValueAtt;
//    juce::Slider lateMix;
//    juce::Label lateMixLabel;
//    std::unique_ptr<SliderAttachment> lateMixAtt;
    
    
    
    
    //JUCE_HEAVYWEIGHT_LEAK_DETECTOR (LouisVerbAudioProcessorEditor)
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LouisVerbAudioProcessorEditor)
};
