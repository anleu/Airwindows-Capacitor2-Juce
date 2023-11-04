#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace audio_plugin {
class Capacitor2Processor : public juce::AudioProcessor {
public:
  Capacitor2Processor();
  ~Capacitor2Processor() override;

  void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  void releaseResources() override;

  bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

  void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
  using AudioProcessor::processBlock;

  juce::AudioProcessorEditor* createEditor() override;
  bool hasEditor() const override;

  const juce::String getName() const override;

  bool acceptsMidi() const override;
  bool producesMidi() const override;
  bool isMidiEffect() const override;
  double getTailLengthSeconds() const override;

  int getNumPrograms() override;
  int getCurrentProgram() override;
  void setCurrentProgram(int index) override;
  const juce::String getProgramName(int index) override;
  void changeProgramName(int index, const juce::String& newName) override;

  void getStateInformation(juce::MemoryBlock& destData) override;
  void setStateInformation(const void* data, int sizeInBytes) override;

  int getNumParameters() override;

  void processDoubleReplacing(const float** inputs,
                              float** outputs,
                              int32_t sampleFrames);

  juce::AudioParameterFloat* highpass;
  juce::AudioParameterFloat* lowpass;
  juce::AudioParameterFloat* nonlin;
  juce::AudioParameterFloat* mix;

  double iirHighpassAL;
  double iirHighpassBL;
  double iirHighpassCL;
  double iirHighpassDL;
  double iirHighpassEL;
  double iirHighpassFL;
  double iirLowpassAL;
  double iirLowpassBL;
  double iirLowpassCL;
  double iirLowpassDL;
  double iirLowpassEL;
  double iirLowpassFL;

  double iirHighpassAR;
  double iirHighpassBR;
  double iirHighpassCR;
  double iirHighpassDR;
  double iirHighpassER;
  double iirHighpassFR;
  double iirLowpassAR;
  double iirLowpassBR;
  double iirLowpassCR;
  double iirLowpassDR;
  double iirLowpassER;
  double iirLowpassFR;

  int count;

  double lowpassChase;
  double highpassChase;
  double wetChase;

  double lowpassBaseAmount;
  double highpassBaseAmount;
  double wet;

  double lastLowpass;
  double lastHighpass;
  double lastWet;

  int32_t fpdL;
  int32_t fpdR;
  // default stuff

  float A;
  float B;
  float C;
  float D;

private:
  juce::AudioProcessorValueTreeState mState;
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Capacitor2Processor)
};
}  // namespace audio_plugin
