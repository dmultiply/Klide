#include "PresetManager.h"

namespace Service
{
	const File PresetManager::defaultDirectory{ File::getSpecialLocation(
		File::SpecialLocationType::commonDocumentsDirectory)
			.getChildFile(ProjectInfo::companyName)
			.getChildFile(ProjectInfo::projectName)
	};
    const File PresetManager::defaultAudioDirectory{ File::getSpecialLocation(                                                                         File::SpecialLocationType::commonDocumentsDirectory)
        .getChildFile(ProjectInfo::companyName)
        .getChildFile(ProjectInfo::projectName)
        .getChildFile("Audio")
    };
    
	const String PresetManager::extension{ "preset" };
	const String PresetManager::presetNameProperty{ "presetName" };

	PresetManager::PresetManager(AudioProcessorValueTreeState& apvts) :
		valueTreeState(apvts)
	{
		// Create a default Preset Directory, if it doesn't exist
		if (!defaultDirectory.exists())
		{
			const auto result = defaultDirectory.createDirectory();
			if (result.failed())
			{
				DBG("Could not create preset directory: " + result.getErrorMessage());
				jassertfalse;
			}
		}
        
        // Create a default Audio Files Directory, if it doesn't exist
        if (!defaultAudioDirectory.exists())
        {
        const auto result = defaultAudioDirectory.createDirectory();
            if (result.failed())
            {
                DBG("Could not create audio directory: " + result.getErrorMessage());
                jassertfalse;
            }
        }

		valueTreeState.state.addListener(this);
		currentPreset.referTo(valueTreeState.state.getPropertyAsValue(presetNameProperty, nullptr));
	}

	void PresetManager::savePreset(const String& presetName)
	{
		if (presetName.isEmpty())
			return;

		currentPreset.setValue(presetName);
		const auto xml = valueTreeState.copyState().createXml();
		const auto presetFile = defaultDirectory.getChildFile(presetName + "." + extension);
		if (!xml->writeTo(presetFile))
		{
			DBG("Could not create preset file: " + presetFile.getFullPathName());
			jassertfalse;
		}
        
        // === Copy audio file to defautAudioDirectory
        //0
        //TODO : retrieve the number of row from the tree or else
        for(int row=0;row<4;row++)
        {
            Value audioPath = valueTreeState.state.getPropertyAsValue("AUDIO_FILEPATH"+String(row), nullptr, true);
            std::string filename = audioPath.toString().toStdString();
            if(filename != "") {
                File file(filename);
                
                //Get extension
                int position = filename.find_last_of(".");
                String result = filename.substr(position+1);
                
                //TODO : add check that it's possible to copy and all
                file.copyFileTo(defaultAudioDirectory.getChildFile(presetName+ "_" +std::to_string(row) + "." + result));
            }
        }
        
	}

	void PresetManager::deletePreset(const String& presetName)
	{
		if (presetName.isEmpty())
			return;

		const auto presetFile = defaultDirectory.getChildFile(presetName + "." + extension);
		if (!presetFile.existsAsFile())
		{
			DBG("Preset file " + presetFile.getFullPathName() + " does not exist");
			jassertfalse;
			return;
		}
		if (!presetFile.deleteFile())
		{
			DBG("Preset file " + presetFile.getFullPathName() + " could not be deleted");
			jassertfalse;
			return;
		}
		currentPreset.setValue("");
	}

	void PresetManager::loadPreset(const String& presetName)
	{
		if (presetName.isEmpty())
			return;

		const auto presetFile = defaultDirectory.getChildFile(presetName + "." + extension);
		if (!presetFile.existsAsFile())
		{
			DBG("Preset file " + presetFile.getFullPathName() + " does not exist");
			jassertfalse;
			return;
		}
		// presetFile (XML) -> (ValueTree)
		XmlDocument xmlDocument{ presetFile }; 
		const auto valueTreeToLoad = ValueTree::fromXml(*xmlDocument.getDocumentElement());
            
        

        
		valueTreeState.replaceState(valueTreeToLoad);
		currentPreset.setValue(presetName);

	}

	int PresetManager::loadNextPreset()
	{
		const auto allPresets = getAllPresets();
		if (allPresets.isEmpty())
			return -1;
		const auto currentIndex = allPresets.indexOf(currentPreset.toString());
		const auto nextIndex = currentIndex + 1 > (allPresets.size() - 1) ? 0 : currentIndex + 1;
		loadPreset(allPresets.getReference(nextIndex));
		return nextIndex;
	}

	int PresetManager::loadPreviousPreset()
	{
		const auto allPresets = getAllPresets();
		if (allPresets.isEmpty())
			return -1;
		const auto currentIndex = allPresets.indexOf(currentPreset.toString());
		const auto previousIndex = currentIndex - 1 < 0 ? allPresets.size() - 1 : currentIndex - 1;
		loadPreset(allPresets.getReference(previousIndex));
		return previousIndex;
	}

	StringArray PresetManager::getAllPresets() const
	{
		StringArray presets;
		const auto fileArray = defaultDirectory.findChildFiles(
			File::TypesOfFileToFind::findFiles, false, "*." + extension);
		for (const auto& file : fileArray)
		{
			presets.add(file.getFileNameWithoutExtension());
		}
		return presets;
	}

	String PresetManager::getCurrentPreset() const
	{
		return currentPreset.toString();
	}

	void PresetManager::valueTreeRedirected(ValueTree& treeWhichHasBeenChanged)
	{
		currentPreset.referTo(treeWhichHasBeenChanged.getPropertyAsValue(presetNameProperty, nullptr));
	}

    
}
