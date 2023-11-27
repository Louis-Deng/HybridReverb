/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
LouisVerbAudioProcessorEditor::LouisVerbAudioProcessorEditor
    (LouisVerbAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p)
    , audioProcessor (p)
    , valueTreeState(vts)
/*
    , drywetMix     ("00-allmix")
    , convMix       ("01-convmix")
    , lateMix       ("02-latemix")
 */
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (900, 600);
    setResizable(false, false);
    
    // Add drop down menu
    menuSelectLabel.setText("Auditoria: ", juce::dontSendNotification);
    menuSelect.setBounds       (20, 20, 120, 40 );
    
    addAndMakeVisible(&menuSelectLabel);
    addAndMakeVisible(&menuSelect);
    menuSelect.addSectionHeading ("phones");
    menuSelect.addItem ("Lecture  1", 1);
    menuSelect.addItem ("Lecture  2", 2);
    menuSelect.addItem ("Stairway 1", 3);
    menuSelect.addItem ("Stairway 2", 4);
    menuSelect.addItem ("Stairway 3", 5);
    menuSelect.addItem ("Bathroom  ", 6);
    menuSelect.addItem ("Office    ", 7);
    menuSelect.addItem ("Meeting   ", 8);
    menuSelect.addSeparator();
    menuSelect.addSectionHeading ("binaurals");
    menuSelect.addItem ("Lecture   ", 9);
    menuSelect.addItem ("Stairway  ", 10);
    menuSelect.addItem ("Office    ", 11);
    menuSelect.addItem ("Meeting   ", 12);
    menuSelect.addItem ("Aula Carolina", 13);
    menuSelect.addSeparator();
    

    setSize (400, 200);
    
    // Define slider object
    drywetMix.setSliderStyle (juce::Slider::RotaryVerticalDrag);
    drywetMix.setMouseDragSensitivity(100);
    drywetMix.setTextBoxStyle (juce::Slider::TextBoxBelow, true, 180, 20);
    
    decayTime.setSliderStyle (juce::Slider::RotaryVerticalDrag);
    decayTime.setMouseDragSensitivity(80);
    decayTime.setSkewFactor(0.5);
    decayTime.setTextBoxStyle (juce::Slider::TextBoxBelow, true, 90, 15);
    //decayTime.setTextValueSuffix (" s");
    
    
    convMix.setSliderStyle (juce::Slider::RotaryVerticalDrag);
    convMix.setMouseDragSensitivity(100);
    convMix.setTextBoxStyle (juce::Slider::TextBoxBelow, true, 180, 20);
    
    // adds slider to editor
    addAndMakeVisible(&drywetMix);
    addAndMakeVisible(&decayTime);
    addAndMakeVisible(&convMix);
    addAndMakeVisible(&fdnon);
    
    
    addAndMakeVisible (drywetMixLabel);
    drywetMixLabel.setText ("Dry/Wet Mix", juce::dontSendNotification);
    //true is to the left, false is above
    drywetMixLabel.attachToComponent (&drywetMix, false);
    
    addAndMakeVisible (decayTimeLabel);
    decayTimeLabel.setText ("Decay Time", juce::dontSendNotification);
    decayTimeLabel.attachToComponent (&decayTime, false);
    
    addAndMakeVisible (convMixLabel);
    convMixLabel.setText ("Early Echo", juce::dontSendNotification);
    convMixLabel.attachToComponent (&convMix, false);
    
    addAndMakeVisible(fdnonLabel);
    fdnonLabel.setText("Modulation", juce::dontSendNotification);
    fdnonLabel.attachToComponent (&fdnon, false);
    
    //4 params: x,y,width,height
    drywetMix.setBounds     (getWidth()-140, 40, 120, 120 );
    decayTime.setBounds     (getWidth()-250, 40, 90, 90 );
    convMix.setBounds       (20, getHeight()-100, 90, 90 );
    fdnon.setBounds(getWidth()-250, 150, 60, 20);
    
    drywetMix.addListener(this);
    decayTime.addListener(this);
    convMix.addListener(this);
    fdnon.addListener(this);
    
    menuSelect.addListener(this);
    
    drywetMixAtt.reset (new SliderAttachment (valueTreeState, "00-allmix", drywetMix));
    decayTimeAtt.reset (new SliderAttachment (valueTreeState, "02-decay", decayTime));
    convMixAtt.reset (new SliderAttachment (valueTreeState, "01-convmix", convMix));
    menuAtt.reset (new ComboBoxAttachment (valueTreeState, "de-menuitem", menuSelect));
    fdnonAtt.reset (new ButtonAttachment (valueTreeState, "04-fdnon", fdnon));
}

LouisVerbAudioProcessorEditor::~LouisVerbAudioProcessorEditor()
{
}

//==============================================================================
void LouisVerbAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (juce::Colours::yellowgreen);

    g.setColour (juce::Colours::grey);
    g.setFont (15.0f);
    g.drawFittedText ("ReBirbVerb v1.3", getLocalBounds(), juce::Justification::topLeft, 1);
    
    
}

void LouisVerbAudioProcessorEditor::sliderValueChanged(juce::Slider* subject)
{
    if (subject == &drywetMix){
        // change drywetmix a and 1.0-a
        audioProcessor.chAllMixe(drywetMix.getValue());
    }

    else if (subject == &decayTime){
        audioProcessor.chLateDecay(decayTime.getValue());
    }
//    
//    else if (subject == &gcgainValue){
//        audioProcessor.chLateGain(gcgainValue.getValue());
//    }
//    
//    else if (subject == &dampValue){
//        audioProcessor.chLateDamp(dampValue.getValue());
//    }
//
//    else if (subject == &lateMix){
//        //gib slider value to function in PluginProcessor.h/cpp
//        audioProcessor.chLateMixe(lateMix.getValue());
//    }
    
    else if (subject == &convMix){
        audioProcessor.chConvMixe(convMix.getValue());
    }

}

void LouisVerbAudioProcessorEditor::buttonClicked(juce::Button* subject)
{
    if (subject == &fdnon){
        audioProcessor.switchFdn(fdnon.getToggleState());
    }
}

void LouisVerbAudioProcessorEditor::comboBoxChanged(juce::ComboBox* subject)
{
    switch (subject->getSelectedId())
    {
        case 1:  audioProcessor.chPreset(phone_lecture1_hfrp_1,phone_lecture1_hfrp_2);          break;
        case 2:  audioProcessor.chPreset(phone_lecture2_hfrp_1,phone_lecture2_hfrp_2);          break;
        case 3:  audioProcessor.chPreset(phone_stairway1_hfrp_1,phone_stairway1_hfrp_2);        break;
        case 4:  audioProcessor.chPreset(phone_stairway2_hfrp_1,phone_stairway2_hfrp_2);        break;
        case 5:  audioProcessor.chPreset(phone_stairway3_hfrp_1,phone_stairway3_hfrp_2);        break;
        case 6:  audioProcessor.chPreset(phone_bathroom_hfrp_1,phone_bathroom_hfrp_2);          break;
        case 7:  audioProcessor.chPreset(phone_office1_hfrp_1,phone_office1_hfrp_2);            break;
        case 8:  audioProcessor.chPreset(phone_meeting_hfrp_1,phone_meeting_hfrp_2);            break;
        case 9:  audioProcessor.chPreset(binaural_lecture_azi0_1,binaural_lecture_azi0_2);      break;
        case 10: audioProcessor.chPreset(binaural_stairway_azi0_1,binaural_stairway_azi0_2);    break;
        case 11: audioProcessor.chPreset(binaural_office_azi0_1,binaural_office_azi0_2);        break;
        case 12: audioProcessor.chPreset(binaural_meeting_azi0_1,binaural_meeting_azi0_2);      break;
        case 13: audioProcessor.chPreset(binaural_aula_carolina_azi0_1,binaural_aula_carolina_azi0_2);     break;
        default: break;
    }

    //textLabel.setFont (textFont);
}

void LouisVerbAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
    
}
