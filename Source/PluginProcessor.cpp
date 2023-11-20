#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace audio_plugin {
	Capacitor2Processor::Capacitor2Processor()
		: AudioProcessor(
			BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
			.withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
			.withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
		),
		mState(*this, nullptr) {

		highpass = new juce::AudioParameterFloat("highpass", "Highpass", 0.0f,
			1000.0f, 0.0f);
		addParameter(highpass);

		lowpass = new juce::AudioParameterFloat("lowpass", "Lowpass", 1000.0f,
			20000.0f, 20000.0f);
		addParameter(lowpass);

		nonlin = new juce::AudioParameterFloat("nonlin", "Nonlin", 0.0f, 1.0f, 0.0f);
		addParameter(nonlin);

		mix = new juce::AudioParameterFloat("mix", "Mix", 0.0f, 1.0f, 1.0f);
		addParameter(mix);

		iirHighpassAL = 0.0;
		iirHighpassBL = 0.0;
		iirHighpassCL = 0.0;
		iirHighpassDL = 0.0;
		iirHighpassEL = 0.0;
		iirHighpassFL = 0.0;
		iirLowpassAL = 0.0;
		iirLowpassBL = 0.0;
		iirLowpassCL = 0.0;
		iirLowpassDL = 0.0;
		iirLowpassEL = 0.0;
		iirLowpassFL = 0.0;

		iirHighpassAR = 0.0;
		iirHighpassBR = 0.0;
		iirHighpassCR = 0.0;
		iirHighpassDR = 0.0;
		iirHighpassER = 0.0;
		iirHighpassFR = 0.0;
		iirLowpassAR = 0.0;
		iirLowpassBR = 0.0;
		iirLowpassCR = 0.0;
		iirLowpassDR = 0.0;
		iirLowpassER = 0.0;
		iirLowpassFR = 0.0;
		count = 0;
		lowpassChase = 0.0;
		highpassChase = 0.0;
		wetChase = 0.0;
		lowpassBaseAmount = 1.0;
		highpassBaseAmount = 0.0;
		lastLowpass = 1000.0;
		lastHighpass = 1000.0;
		lastWet = 1000.0;

		fpdL = 1.0;
		while (fpdL < 16386)
			fpdL = static_cast<uint32_t>(rand()) << 16 | static_cast<uint32_t>(rand());
		fpdR = 1.0;
		while (fpdR < 16386)
			fpdR = static_cast<uint32_t>(rand()) << 16 | static_cast<uint32_t>(rand());
	}

	Capacitor2Processor::~Capacitor2Processor() {}

	const juce::String Capacitor2Processor::getName() const {
		return JucePlugin_Name;
	}

	bool Capacitor2Processor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
		return true;
#else
		return false;
#endif
	}

	bool Capacitor2Processor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
		return true;
#else
		return false;
#endif
	}

	bool Capacitor2Processor::isMidiEffect() const {
#if JucePlugin_IsMidiEffect
		return true;
#else
		return false;
#endif
	}

	double Capacitor2Processor::getTailLengthSeconds() const {
		return 0.0;
	}

	int Capacitor2Processor::getNumPrograms() {
		return 1;  // NB: some hosts don't cope very well if you tell them there are 0
		// programs, so this should be at least 1, even if you're not
		// really implementing programs.
	}

	int Capacitor2Processor::getNumParameters() {
		return 4;  // ToDo can we get the number of entries in the enum
		// Capacitor2::Params?
	}

	int Capacitor2Processor::getCurrentProgram() {
		return 0;
	}

	void Capacitor2Processor::setCurrentProgram(int index) {
		juce::ignoreUnused(index);
	}

	const juce::String Capacitor2Processor::getProgramName(int index) {
		juce::ignoreUnused(index);
		return {};
	}

	void Capacitor2Processor::changeProgramName(int index,
		const juce::String& newName) {
		juce::ignoreUnused(index, newName);
	}

	void Capacitor2Processor::prepareToPlay(double sampleRate,
		int samplesPerBlock) {
		// Use this method as the place to do any pre-playback
		// initialisation that you need..
		juce::ignoreUnused(sampleRate, samplesPerBlock);
	}

	void Capacitor2Processor::releaseResources() {
		// When playback stops, you can use this as an opportunity to free up any
		// spare memory, etc.
	}

	bool Capacitor2Processor::isBusesLayoutSupported(
		const BusesLayout& layouts) const {
#if JucePlugin_IsMidiEffect
		juce::ignoreUnused(layouts);
		return true;
#else
		// This is the place where you check if the layout is supported.
		// In this template code we only support mono or stereo.
		// Some plugin hosts, such as certain GarageBand versions, will only
		// load plugins that support stereo bus layouts.
		if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
			layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
			return false;

		// This checks if the input layout matches the output layout
#if !JucePlugin_IsSynth
		if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
			return false;
#endif

		return true;
#endif
	}

	bool Capacitor2Processor::hasEditor() const {
		return true;  // (change this to false if you choose to not supply an editor)
	}

	juce::AudioProcessorEditor* Capacitor2Processor::createEditor() {
		return new Capacitor2Editor(*this);
	}

	// =============================================================================
	// section param handling

	void Capacitor2Processor::getStateInformation(juce::MemoryBlock& destData) {
		juce::MemoryOutputStream(destData, true).writeFloat(*highpass);
		juce::MemoryOutputStream(destData, true).writeFloat(*lowpass);
		juce::MemoryOutputStream(destData, true).writeFloat(*nonlin);
		juce::MemoryOutputStream(destData, true).writeFloat(*mix);
	}

	void Capacitor2Processor::setStateInformation(const void* data,
		int sizeInBytes) {
		*highpass =
			juce::MemoryInputStream(data, static_cast<size_t>(sizeInBytes), false)
			.readFloat();
		*lowpass =
			juce::MemoryInputStream(data, static_cast<size_t>(sizeInBytes), false)
			.readFloat();
		*nonlin =
			juce::MemoryInputStream(data, static_cast<size_t>(sizeInBytes), false)
			.readFloat();
		*mix = juce::MemoryInputStream(data, static_cast<size_t>(sizeInBytes), false)
			.readFloat();
	}

	// process section

	void Capacitor2Processor::processBlock(juce::AudioBuffer<float>& buffer,
		juce::MidiBuffer& midiMessages) {
		juce::ignoreUnused(midiMessages);

		juce::ScopedNoDenormals noDenormals;
		size_t totalNumInputChannels = getTotalNumInputChannels();
		size_t totalNumOutputChannels = getTotalNumOutputChannels();

		// In case we have more outputs than inputs, this code clears any output
		// channels that didn't contain input data, (because these aren't
		// guaranteed to be empty - they may contain garbage).
		// This is here to avoid people getting screaming feedback
		// when they first compile a plugin, but obviously you don't need to keep
		// this code if your algorithm always overwrites all the output channels.
		for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
			buffer.clear(i, 0, buffer.getNumSamples());

		std::vector<const float*> inputPointers(totalNumInputChannels);
		std::vector<float*> outputPointers(totalNumOutputChannels);

		for (size_t i = 0; i < totalNumInputChannels; ++i) {
			inputPointers[i] = buffer.getReadPointer(i);
		}
		for (size_t i = 0; i < totalNumOutputChannels; ++i) {
			outputPointers[i] = buffer.getWritePointer(i);
		}
		const float** inputs = inputPointers.data();
		float** outputs = outputPointers.data();
		processDoubleReplacing(inputs, outputs, buffer.getNumSamples());
	}

	double cutoffToParameter(double cutoffInHz, double exponent) {
		const double minHz = 5.0;
		const double maxHz = 20000.0;
		const double minParam = 0.00125;
		const double maxParam = 1.0;

		if (cutoffInHz < minHz) return 0;
		if (cutoffInHz > maxHz) return 1;
		double normalizedCutoff = (cutoffInHz - minHz) / (maxHz - minHz);
		double param = pow(normalizedCutoff, exponent);
		return (maxParam - minParam) * param + minParam;
	}

	void Capacitor2Processor::processDoubleReplacing(const float** inputs,
		float** outputs,
		int32_t sampleFrames) {
		const float* in1 = inputs[0];
		const float* in2 = inputs[1];
		float* out1 = outputs[0];
		float* out2 = outputs[1];

		lowpassChase = pow(cutoffToParameter(lowpass->get(), 0.2), 2);
		highpassChase = pow(cutoffToParameter(highpass->get(), 0.6), 2);
		double nonLin = 1.0 + ((1.0 - nonlin->get()) * 6.0);
		double nonLinTrim = 1.5 / cbrt(nonLin);
		wetChase = mix->get();
		// should not scale with sample rate, because values reaching 1 are important
		// to its ability to bypass when set to max
		double lowpassSpeed = 300 / (fabs(lastLowpass - lowpassChase) + 1.0);
		double highpassSpeed = 300 / (fabs(lastHighpass - highpassChase) + 1.0);
		double wetSpeed = 300 / (fabs(lastWet - wetChase) + 1.0);
		lastLowpass = lowpassChase;
		lastHighpass = highpassChase;
		lastWet = wetChase;

		while (--sampleFrames >= 0) {
			double inputSampleL = *in1;
			double inputSampleR = *in2;
			if (fabs(inputSampleL) < 1.18e-23)
				inputSampleL = fpdL * 1.18e-17;
			if (fabs(inputSampleR) < 1.18e-23)
				inputSampleR = fpdR * 1.18e-17;
			double drySampleL = inputSampleL;
			double drySampleR = inputSampleR;

			double dielectricScaleL = fabs(2.0 - ((inputSampleL + nonLin) / nonLin));
			double dielectricScaleR = fabs(2.0 - ((inputSampleR + nonLin) / nonLin));

			lowpassBaseAmount = (((lowpassBaseAmount * lowpassSpeed) + lowpassChase) /
				(lowpassSpeed + 1.0));
			// positive voltage will mean lower capacitance when capacitor is barium
			// titanate on the lowpass, higher pressure means positive swings/smaller
			// cap/larger value for lowpassAmount
			double lowpassAmountL = lowpassBaseAmount * dielectricScaleL;
			double invLowpassL = 1.0 - lowpassAmountL;
			double lowpassAmountR = lowpassBaseAmount * dielectricScaleR;
			double invLowpassR = 1.0 - lowpassAmountR;

			highpassBaseAmount =
				(((highpassBaseAmount * highpassSpeed) + highpassChase) /
					(highpassSpeed + 1.0));
			// positive voltage will mean lower capacitance when capacitor is barium
			// titanate on the highpass, higher pressure means positive swings/smaller
			// cap/larger value for highpassAmount
			double highpassAmountL = highpassBaseAmount * dielectricScaleL;
			double invHighpassL = 1.0 - highpassAmountL;
			double highpassAmountR = highpassBaseAmount * dielectricScaleR;
			double invHighpassR = 1.0 - highpassAmountR;

			wet = (((wet * wetSpeed) + wetChase) / (wetSpeed + 1.0));

			count++;
			if (count > 5)
				count = 0;
			switch (count) {
			case 0:
				iirHighpassAL =
					(iirHighpassAL * invHighpassL) + (inputSampleL * highpassAmountL);
				inputSampleL -= iirHighpassAL;
				iirLowpassAL =
					(iirLowpassAL * invLowpassL) + (inputSampleL * lowpassAmountL);
				inputSampleL = iirLowpassAL;
				iirHighpassBL =
					(iirHighpassBL * invHighpassL) + (inputSampleL * highpassAmountL);
				inputSampleL -= iirHighpassBL;
				iirLowpassBL =
					(iirLowpassBL * invLowpassL) + (inputSampleL * lowpassAmountL);
				inputSampleL = iirLowpassBL;
				iirHighpassDL =
					(iirHighpassDL * invHighpassL) + (inputSampleL * highpassAmountL);
				inputSampleL -= iirHighpassDL;
				iirLowpassDL =
					(iirLowpassDL * invLowpassL) + (inputSampleL * lowpassAmountL);
				inputSampleL = iirLowpassDL;
				iirHighpassAR =
					(iirHighpassAR * invHighpassR) + (inputSampleR * highpassAmountR);
				inputSampleR -= iirHighpassAR;
				iirLowpassAR =
					(iirLowpassAR * invLowpassR) + (inputSampleR * lowpassAmountR);
				inputSampleR = iirLowpassAR;
				iirHighpassBR =
					(iirHighpassBR * invHighpassR) + (inputSampleR * highpassAmountR);
				inputSampleR -= iirHighpassBR;
				iirLowpassBR =
					(iirLowpassBR * invLowpassR) + (inputSampleR * lowpassAmountR);
				inputSampleR = iirLowpassBR;
				iirHighpassDR =
					(iirHighpassDR * invHighpassR) + (inputSampleR * highpassAmountR);
				inputSampleR -= iirHighpassDR;
				iirLowpassDR =
					(iirLowpassDR * invLowpassR) + (inputSampleR * lowpassAmountR);
				inputSampleR = iirLowpassDR;
				break;
			case 1:
				iirHighpassAL =
					(iirHighpassAL * invHighpassL) + (inputSampleL * highpassAmountL);
				inputSampleL -= iirHighpassAL;
				iirLowpassAL =
					(iirLowpassAL * invLowpassL) + (inputSampleL * lowpassAmountL);
				inputSampleL = iirLowpassAL;
				iirHighpassCL =
					(iirHighpassCL * invHighpassL) + (inputSampleL * highpassAmountL);
				inputSampleL -= iirHighpassCL;
				iirLowpassCL =
					(iirLowpassCL * invLowpassL) + (inputSampleL * lowpassAmountL);
				inputSampleL = iirLowpassCL;
				iirHighpassEL =
					(iirHighpassEL * invHighpassL) + (inputSampleL * highpassAmountL);
				inputSampleL -= iirHighpassEL;
				iirLowpassEL =
					(iirLowpassEL * invLowpassL) + (inputSampleL * lowpassAmountL);
				inputSampleL = iirLowpassEL;
				iirHighpassAR =
					(iirHighpassAR * invHighpassR) + (inputSampleR * highpassAmountR);
				inputSampleR -= iirHighpassAR;
				iirLowpassAR =
					(iirLowpassAR * invLowpassR) + (inputSampleR * lowpassAmountR);
				inputSampleR = iirLowpassAR;
				iirHighpassCR =
					(iirHighpassCR * invHighpassR) + (inputSampleR * highpassAmountR);
				inputSampleR -= iirHighpassCR;
				iirLowpassCR =
					(iirLowpassCR * invLowpassR) + (inputSampleR * lowpassAmountR);
				inputSampleR = iirLowpassCR;
				iirHighpassER =
					(iirHighpassER * invHighpassR) + (inputSampleR * highpassAmountR);
				inputSampleR -= iirHighpassER;
				iirLowpassER =
					(iirLowpassER * invLowpassR) + (inputSampleR * lowpassAmountR);
				inputSampleR = iirLowpassER;
				break;
			case 2:
				iirHighpassAL =
					(iirHighpassAL * invHighpassL) + (inputSampleL * highpassAmountL);
				inputSampleL -= iirHighpassAL;
				iirLowpassAL =
					(iirLowpassAL * invLowpassL) + (inputSampleL * lowpassAmountL);
				inputSampleL = iirLowpassAL;
				iirHighpassBL =
					(iirHighpassBL * invHighpassL) + (inputSampleL * highpassAmountL);
				inputSampleL -= iirHighpassBL;
				iirLowpassBL =
					(iirLowpassBL * invLowpassL) + (inputSampleL * lowpassAmountL);
				inputSampleL = iirLowpassBL;
				iirHighpassFL =
					(iirHighpassFL * invHighpassL) + (inputSampleL * highpassAmountL);
				inputSampleL -= iirHighpassFL;
				iirLowpassFL =
					(iirLowpassFL * invLowpassL) + (inputSampleL * lowpassAmountL);
				inputSampleL = iirLowpassFL;
				iirHighpassAR =
					(iirHighpassAR * invHighpassR) + (inputSampleR * highpassAmountR);
				inputSampleR -= iirHighpassAR;
				iirLowpassAR =
					(iirLowpassAR * invLowpassR) + (inputSampleR * lowpassAmountR);
				inputSampleR = iirLowpassAR;
				iirHighpassBR =
					(iirHighpassBR * invHighpassR) + (inputSampleR * highpassAmountR);
				inputSampleR -= iirHighpassBR;
				iirLowpassBR =
					(iirLowpassBR * invLowpassR) + (inputSampleR * lowpassAmountR);
				inputSampleR = iirLowpassBR;
				iirHighpassFR =
					(iirHighpassFR * invHighpassR) + (inputSampleR * highpassAmountR);
				inputSampleR -= iirHighpassFR;
				iirLowpassFR =
					(iirLowpassFR * invLowpassR) + (inputSampleR * lowpassAmountR);
				inputSampleR = iirLowpassFR;
				break;
			case 3:
				iirHighpassAL =
					(iirHighpassAL * invHighpassL) + (inputSampleL * highpassAmountL);
				inputSampleL -= iirHighpassAL;
				iirLowpassAL =
					(iirLowpassAL * invLowpassL) + (inputSampleL * lowpassAmountL);
				inputSampleL = iirLowpassAL;
				iirHighpassCL =
					(iirHighpassCL * invHighpassL) + (inputSampleL * highpassAmountL);
				inputSampleL -= iirHighpassCL;
				iirLowpassCL =
					(iirLowpassCL * invLowpassL) + (inputSampleL * lowpassAmountL);
				inputSampleL = iirLowpassCL;
				iirHighpassDL =
					(iirHighpassDL * invHighpassL) + (inputSampleL * highpassAmountL);
				inputSampleL -= iirHighpassDL;
				iirLowpassDL =
					(iirLowpassDL * invLowpassL) + (inputSampleL * lowpassAmountL);
				inputSampleL = iirLowpassDL;
				iirHighpassAR =
					(iirHighpassAR * invHighpassR) + (inputSampleR * highpassAmountR);
				inputSampleR -= iirHighpassAR;
				iirLowpassAR =
					(iirLowpassAR * invLowpassR) + (inputSampleR * lowpassAmountR);
				inputSampleR = iirLowpassAR;
				iirHighpassCR =
					(iirHighpassCR * invHighpassR) + (inputSampleR * highpassAmountR);
				inputSampleR -= iirHighpassCR;
				iirLowpassCR =
					(iirLowpassCR * invLowpassR) + (inputSampleR * lowpassAmountR);
				inputSampleR = iirLowpassCR;
				iirHighpassDR =
					(iirHighpassDR * invHighpassR) + (inputSampleR * highpassAmountR);
				inputSampleR -= iirHighpassDR;
				iirLowpassDR =
					(iirLowpassDR * invLowpassR) + (inputSampleR * lowpassAmountR);
				inputSampleR = iirLowpassDR;
				break;
			case 4:
				iirHighpassAL =
					(iirHighpassAL * invHighpassL) + (inputSampleL * highpassAmountL);
				inputSampleL -= iirHighpassAL;
				iirLowpassAL =
					(iirLowpassAL * invLowpassL) + (inputSampleL * lowpassAmountL);
				inputSampleL = iirLowpassAL;
				iirHighpassBL =
					(iirHighpassBL * invHighpassL) + (inputSampleL * highpassAmountL);
				inputSampleL -= iirHighpassBL;
				iirLowpassBL =
					(iirLowpassBL * invLowpassL) + (inputSampleL * lowpassAmountL);
				inputSampleL = iirLowpassBL;
				iirHighpassEL =
					(iirHighpassEL * invHighpassL) + (inputSampleL * highpassAmountL);
				inputSampleL -= iirHighpassEL;
				iirLowpassEL =
					(iirLowpassEL * invLowpassL) + (inputSampleL * lowpassAmountL);
				inputSampleL = iirLowpassEL;
				iirHighpassAR =
					(iirHighpassAR * invHighpassR) + (inputSampleR * highpassAmountR);
				inputSampleR -= iirHighpassAR;
				iirLowpassAR =
					(iirLowpassAR * invLowpassR) + (inputSampleR * lowpassAmountR);
				inputSampleR = iirLowpassAR;
				iirHighpassBR =
					(iirHighpassBR * invHighpassR) + (inputSampleR * highpassAmountR);
				inputSampleR -= iirHighpassBR;
				iirLowpassBR =
					(iirLowpassBR * invLowpassR) + (inputSampleR * lowpassAmountR);
				inputSampleR = iirLowpassBR;
				iirHighpassER =
					(iirHighpassER * invHighpassR) + (inputSampleR * highpassAmountR);
				inputSampleR -= iirHighpassER;
				iirLowpassER =
					(iirLowpassER * invLowpassR) + (inputSampleR * lowpassAmountR);
				inputSampleR = iirLowpassER;
				break;
			case 5:
				iirHighpassAL =
					(iirHighpassAL * invHighpassL) + (inputSampleL * highpassAmountL);
				inputSampleL -= iirHighpassAL;
				iirLowpassAL =
					(iirLowpassAL * invLowpassL) + (inputSampleL * lowpassAmountL);
				inputSampleL = iirLowpassAL;
				iirHighpassCL =
					(iirHighpassCL * invHighpassL) + (inputSampleL * highpassAmountL);
				inputSampleL -= iirHighpassCL;
				iirLowpassCL =
					(iirLowpassCL * invLowpassL) + (inputSampleL * lowpassAmountL);
				inputSampleL = iirLowpassCL;
				iirHighpassFL =
					(iirHighpassFL * invHighpassL) + (inputSampleL * highpassAmountL);
				inputSampleL -= iirHighpassFL;
				iirLowpassFL =
					(iirLowpassFL * invLowpassL) + (inputSampleL * lowpassAmountL);
				inputSampleL = iirLowpassFL;
				iirHighpassAR =
					(iirHighpassAR * invHighpassR) + (inputSampleR * highpassAmountR);
				inputSampleR -= iirHighpassAR;
				iirLowpassAR =
					(iirLowpassAR * invLowpassR) + (inputSampleR * lowpassAmountR);
				inputSampleR = iirLowpassAR;
				iirHighpassCR =
					(iirHighpassCR * invHighpassR) + (inputSampleR * highpassAmountR);
				inputSampleR -= iirHighpassCR;
				iirLowpassCR =
					(iirLowpassCR * invLowpassR) + (inputSampleR * lowpassAmountR);
				inputSampleR = iirLowpassCR;
				iirHighpassFR =
					(iirHighpassFR * invHighpassR) + (inputSampleR * highpassAmountR);
				inputSampleR -= iirHighpassFR;
				iirLowpassFR =
					(iirLowpassFR * invLowpassR) + (inputSampleR * lowpassAmountR);
				inputSampleR = iirLowpassFR;
				break;
			}
			// Highpass Filter chunk. This is three poles of IIR highpass, with a
			// 'gearbox' that progressively steepens the filter after minimizing
			// artifacts.

			inputSampleL =
				(drySampleL * (1.0 - wet)) + (inputSampleL * nonLinTrim * wet);
			inputSampleR =
				(drySampleR * (1.0 - wet)) + (inputSampleR * nonLinTrim * wet);

			// begin 32 bit stereo floating point dither
			int expon;
			frexpf((float)inputSampleL, &expon);
			fpdL ^= fpdL << 13;
			fpdL ^= fpdL >> 17;
			fpdL ^= fpdL << 5;
			inputSampleL +=
				((double(fpdL) - uint32_t(0x7fffffff)) * 5.5e-36l * pow(2, expon + 62));
			frexpf((float)inputSampleR, &expon);
			fpdR ^= fpdR << 13;
			fpdR ^= fpdR >> 17;
			fpdR ^= fpdR << 5;
			inputSampleR +=
				((double(fpdR) - uint32_t(0x7fffffff)) * 5.5e-36l * pow(2, expon + 62));
			// end 32 bit stereo floating point dither

			*out1 = inputSampleL;
			*out2 = inputSampleR;

			in1++;
			in2++;
			out1++;
			out2++;
		}
	}

	// namespace audio_plugin
}  // namespace audio_plugin

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
	return new audio_plugin::Capacitor2Processor();
}
