#include "PluginEditor.h"
#include "PluginProcessor.h"

namespace audio_plugin {
Capacitor2Editor::Capacitor2Editor(Capacitor2Processor& p)
    : AudioProcessorEditor(&p),
      processorRef(p),
      highPassLabel("Highpass", "Highpass"),
      lowPassLabel("Lowpass", "Lowpass"),
      nonlinLabel("Nonlin", "Nonlin"),
      mixLabel("Mix", "Mix") {
  setSize(560, 300);

  // ===========================================================================
  // highpass config
  highPassSlider.setSliderStyle(juce::Slider::Rotary);
  highPassSlider.setRange(0, 1000.0f, 1.0);
  highPassSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 100, 40);
  highPassSlider.setPopupDisplayEnabled(true, false, this);
  highPassSlider.setTextValueSuffix(" Hz");
  highPassSlider.setValue(processorRef.highpass->get());
  highPassSlider.addListener(this);
  highPassLabel.setJustificationType(juce::Justification::centred);
  highPassLabel.attachToComponent(&highPassSlider, false);
  addAndMakeVisible(&highPassSlider);

  // ===========================================================================
  // lowpass config
  lowPassSlider.setSliderStyle(juce::Slider::Rotary);
  lowPassSlider.setRange(1000, 20000.0f, 1.0);
  lowPassSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 100, 40);
  lowPassSlider.setPopupDisplayEnabled(true, false, this);
  lowPassSlider.setTextValueSuffix(" Hz");
  lowPassSlider.setValue(processorRef.lowpass->get());
  lowPassSlider.addListener(this);
  lowPassLabel.setJustificationType(juce::Justification::centred);
  lowPassLabel.attachToComponent(&lowPassSlider, false);
  addAndMakeVisible(&lowPassSlider);

  // ===========================================================================
  // nonlin config
  nonlinSlider.setSliderStyle(juce::Slider::Rotary);
  nonlinSlider.setRange(0, 1, 0.01);
  nonlinSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 100, 40);
  nonlinSlider.setPopupDisplayEnabled(true, false, this);
  nonlinSlider.setTextValueSuffix("");
  nonlinSlider.setValue(processorRef.nonlin->get());
  nonlinSlider.addListener(this);
  nonlinLabel.setJustificationType(juce::Justification::centred);
  nonlinLabel.attachToComponent(&nonlinSlider, false);
  addAndMakeVisible(&nonlinSlider);

  // ===========================================================================
  // mix config
  mixSlider.setSliderStyle(juce::Slider::Rotary);
  mixSlider.setRange(0.0, 100, 1.0);
  mixSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 100, 40);
  mixSlider.setPopupDisplayEnabled(true, false, this);
  mixSlider.setTextValueSuffix(" %");
  mixSlider.setValue(processorRef.mix->get() * 100);
  mixSlider.addListener(this);
  mixLabel.setJustificationType(juce::Justification::centred);
  mixLabel.attachToComponent(&mixSlider, false);
  addAndMakeVisible(&mixSlider);
}

Capacitor2Editor::~Capacitor2Editor() {}

void Capacitor2Editor::paint(juce::Graphics& g) {
  // (Our component is opaque, so we must completely fill the background with a
  // solid colour)
  g.setColour(juce::Colours::white);
  g.setFont(18.0f);
  g.drawFittedText("Capacitor 2 by Airwindows", getLocalBounds(),
                   juce::Justification::centredTop, 1);

  g.setFont(12.0f);
  g.drawFittedText("Juce Plugin by 091Audio", getLocalBounds(),
                   juce::Justification::bottomRight, 1);
}

void Capacitor2Editor::resized() {
  auto area = getLocalBounds();

  highPassSlider.setBounds(10, 60, 100, 150);
  lowPassSlider.setBounds(120, 60, 100, 150);
  nonlinSlider.setBounds(240, 60, 100, 150);
  mixSlider.setBounds(360, 60, 100, 150);
}

void Capacitor2Editor::sliderValueChanged(juce::Slider* slider) {
  if (slider == &highPassSlider) {
    *processorRef.highpass = (float)highPassSlider.getValue();
  } else if (slider == &lowPassSlider) {
    *processorRef.lowpass = (float)lowPassSlider.getValue();
  } else if (slider == &nonlinSlider) {
    *processorRef.nonlin = (float)nonlinSlider.getValue();
  } else if (slider == &mixSlider) {
    *processorRef.mix = (float)mixSlider.getValue() / 100;
  }
}

}  // namespace audio_plugin
