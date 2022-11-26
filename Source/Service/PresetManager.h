#pragma once

#include <JuceHeader.h>
#include "./../SimpleSynth.h"
#include "./../StepSequencerData.h"



using namespace juce;

namespace Service
{
	class PresetManager : ValueTree::Listener
	{
	public:
		static const File defaultDirectory;
        static const File defaultAudioDirectory;

		static const String extension;
		static const String presetNameProperty;

		PresetManager(AudioProcessorValueTreeState&);

		void savePreset(const String& presetName);
		void deletePreset(const String& presetName);
		void loadPreset(const String& presetName);
		int loadNextPreset();
		int loadPreviousPreset();
		StringArray getAllPresets() const;
		String getCurrentPreset() const;
                
	private:
		void valueTreeRedirected(ValueTree& treeWhichHasBeenChanged) override;

		AudioProcessorValueTreeState& valueTreeState;
		Value currentPreset;

	};
}
