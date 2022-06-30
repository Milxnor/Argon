#pragma once

#include <fstream>
#include "util.h"

enum class Error : uint8_t // IDK OK
{
	SUCCESS = 0,
	FAILED_TO_CREATE_DIR = 1,
	STREAM_NOT_OPEN = 0
};

namespace Installs
{
	/*
	
	install.json:

	{
		"username": "Milxnor",
		"launch_arguments": "",
		"InstallationList": [
			{
				"name": "Butterfly Event",
				"path": "C:/6.21",
				"message": "This version you can play butterfly event on!",
				"version": "6.21"
			},
			{
				"name": "5.00",
				"path": "C:/5.00",
				"message": "NONE",
				"version": "5.00"
			}
		]
	}

	*/

	static std::ofstream InstallJsonStream;
	static fs::path ArgonAppDataPath;

	bool IsOpen()
	{
		return InstallJsonStream.is_open();
	}

	Error SetupStream()
	{
		ArgonAppDataPath = (fs::path(GetEnv(_("LOCALAPPDATA"))) / _("Argon"));

		if (!fs::exists(ArgonAppDataPath))
		{
			auto a = fs::create_directory(ArgonAppDataPath);
			// auto b = fs::file(ArgonAppDataPath / _("install.json"));

			if (!a) // || !b)
				return Error::FAILED_TO_CREATE_DIR;
		}

		InstallJsonStream.open(ArgonAppDataPath / _("install.json"), std::ios::app);
		return IsOpen() ? Error::SUCCESS : Error::STREAM_NOT_OPEN;
	}

	std::ofstream& GetStream()
	{
		return InstallJsonStream;
	}
}