#pragma once

#include "PluginProcessor.h"

namespace audio_plugin {

class Capacitor2Editor : public juce::AudioProcessorEditor, juce::Slider::Listener {
public:
  explicit Capacitor2Editor(Capacitor2Processor&);
  ~Capacitor2Editor() override;

  void paint(juce::Graphics &) override;
  void resized() override;

private:
  // This reference is provided as a quick way for your editor to
  // access the processor object that created it.
  Capacitor2Processor&processorRef;

  void sliderValueChanged(juce::Slider* slider) override;

  juce::Label highPassLabel;
  juce::Slider highPassSlider;

  juce::Label lowPassLabel;
  juce::Slider lowPassSlider;

  juce::Label nonlinLabel;
  juce::Slider nonlinSlider;

  juce::Label mixLabel;
  juce::Slider mixSlider;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Capacitor2Editor)
};
} // namespace audio_plugin
