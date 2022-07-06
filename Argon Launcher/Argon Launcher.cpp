#include <iostream>
#include <xorstr.hpp>
#include <string>

#include <Windows.h>
#include <filesystem>
#include <fstream>
#include <json.hpp>
#include "installs.h"

#include "fnauth.h"
#include "util.h"

#pragma comment(lib, "urlmon.lib")

using namespace nlohmann;

const static std::string LauncherVersion = _("0.2");

void LaunchPrivateServer()
{
	ordered_json j = json::parse(std::ifstream((Installs::ArgonAppDataPath / _("install.json")).generic_string()));

	std::string options = _(R"(
Installations are for S1-S15 private server versions.

[1] Change Launch Settings
[2] Add a new version
[3] Edit a version
[4] Remove a version
[5] Edit a version
[6] Launch a version
[7] List versions
)");

	std::cout << std::format(_("\nWelcome back {}!"), j[_("username")].get<std::string>()) << '\n' << options << "\n";

	std::string optStr;
	std::cout << _("Enter option: ");
	std::getline(std::cin, optStr);

	int opt = 0;
	try
	{
		opt = std::stoi(optStr);
	}
	catch (std::exception& e) {}

	auto invalidOption = []() {
		std::cout << _("Invalid option!\n");
		std::cin.get();
	};

	auto notImplemented = []() {
		std::cout << _("Not Implemented!\n");
		std::cin.get();
	};

	auto& listOfInstalls = j[_("InstallationList")];

	switch (opt)
	{
	case 1:
	{
		options = _(R"(
[1] Change Username
[2] Edit Launch Arguments (coming soon)
)");
		std::cout << options << "\n";

		std::cout << _("Enter option: ");
		std::getline(std::cin, optStr);

		auto opt2 = 0;
		try
		{
			opt2 = std::stoi(optStr);
		}
		catch (std::exception& e) {}

		switch (opt2)
		{
		case 1:
		{
			std::string name;
			std::cout << _("Enter Username: ");
			std::getline(std::cin, name);

			j["username"] = name;
			std::cout << _("Changed Name!");
			break;
		}
		case 2:
			notImplemented();
			break;
		default:
			invalidOption();
			break;
		}
		break;
	}
	case 2:
		notImplemented();
		break;
	case 3:
		notImplemented();
		break;
	case 4:
		notImplemented();
		break;
	case 5:
		notImplemented();
		break;
	case 6:
		notImplemented();
		break;
	case 7:
		for (auto& game : listOfInstalls)
		{
			std::cout << std::format(_("\n{}:\n\nName: {}\nPath: {}\n"), game["version"].get<std::string>(), game["name"].get<std::string>(), game["path"].get<std::string>() /* << (game["message"].get<std::string>().contains("NONE") ? "" : "\n" + game["message"].get<std::string>()) */);
		}
		break;
	default:
		invalidOption();
		break;
	}

	LaunchPrivateServer();
}

std::string RequestAuthorizationCode()
{
	std::cout << _("Please get your authorization code from here from the process about to start...");
	Sleep(750);
	ShellExecuteA(0, 0, std::format("https://www.epicgames.com/id/api/redirect?clientId={}&responseType=code", FortniteAuth::clientId).c_str(), 0, 0, SW_SHOW);

	std::cout << _("\n\nPlease enter your authorization code: ");
	std::string code;
	std::getline(std::cin, code);
	return code;
}

std::string FindFortnitePath()
{
	char* buf{};
	size_t size = MAX_PATH;
	_dupenv_s(&buf, &size, _("PROGRAMDATA"));

	if (buf)
	{
		auto InstalledDat = fs::path(std::string(buf) + _(R"(\Epic\UnrealEngineLauncher\LauncherInstalled.dat)"));
		free(buf);

		// if (!fs::exists(InstalledDat))
			// return "Could not find InstalledDat!";
		
		std::ifstream data(InstalledDat);
		json j = json::parse(data);
		data.close();

		auto& listOfGames = j[_("InstallationList")];

		for (auto& game : listOfGames)
		{
			if (game[_("AppName")] == _("Fortnite"))
				return game[_("InstallLocation")];
		}

		return _("Could not find Fortnite Path!");
	}

	return _("Could not find ProgramData!");
}

auto DownloadFile(const std::string& savePath, const std::string& Url)
{
	return URLDownloadToFileA(NULL, Url.c_str(), savePath.c_str(), 0, NULL);
}

std::string ReadFromPastebin(const std::string& Url) // terrible
{
	std::string argonPath = _("C:\\Argon");
	
	if (!fs::exists(argonPath))
		fs::create_directory(argonPath);

	static auto temp = argonPath + _("\\temp.txt");
	
	if (fs::exists(temp))
		fs::remove(temp);
	
	DownloadFile(temp, Url);
	
	if (!fs::exists(temp))
	{
		std::cout << _("Could not download file! Error: ") << GetLastError() << '\n';
		return "";
	}

	std::ifstream input_file(temp);

	if (!input_file.is_open()) return "";

	auto data = std::string((std::istreambuf_iterator<char>(input_file)), std::istreambuf_iterator<char>());

	input_file.close();

	//fs::remove(temp);

	return data;
}

int main(){
	auto newestLauncherVersion = ReadFromPastebin(_("https://pastebin.com/raw/mHpSTbRp"));
	
	if (newestLauncherVersion == LauncherVersion)
		std::cout << _("You are up to date!\n");
	
	else
	{
		std::cout << _("Please download the new launcher in the discord https://discord.gg/JqJDDBFUWn.\n");
		// std::cin.get();
	}
	
	auto fnPath = FindFortnitePath();

	if (!fnPath.contains(_("\\")))
	{
		std::cout << fnPath << '\n';
		std::cin.get();
	}

	fnPath += _(R"(\FortniteGame\Binaries\Win64\)");
	
    std::string options = _(R"(
[1] - Launch Hybrid
[2] - Launch S13
[3] - Installations
)");

	// Add modifying launch arguments

	auto ogAnticheats = fnPath + _("Original Anticheats\\");

	if (fs::exists(ogAnticheats))
		options += _("[4] - Revert to old Fortnite\n");

	std::cout << options << "\n";
	
	std::string optStr;
	std::cout << _("Enter option: ");
	std::getline(std::cin, optStr);
	
	int opt = 0;
	try
	{
		opt = std::stoi(optStr);
	}
	catch (std::exception& e) {}
	
	auto invalidOption = []() {
		std::cout << _("Invalid option!\n");
		std::cin.get();
	};

	auto notImplemented = []() {
		std::cout << _("Not Implemented!\n");
		std::cin.get();
	};

	static auto bePath = fnPath + _("FortniteClient-Win64-Shipping_BE.exe");
	static auto eacPath = fnPath + _("FortniteClient-Win64-Shipping_EAC.exe");
	static auto dllPath = fnPath + _("Argon.dll");
	
	Installs::SetupStream();

	switch (opt)
	{
	case 1:
	{
		ProcessParams fnShipping;
		fnShipping.exeName = (fs::path(fnPath) / _("FortniteClient-Win64-Shipping.exe")).generic_string();
		fnShipping.exeIsFullPath = true;

		if (!fs::exists(ogAnticheats))
		{
			fs::create_directory(ogAnticheats);

			if (!isFakeAnticheat(bePath))
				fs::rename(bePath, ogAnticheats + _("FortniteClient-Win64-Shipping_BE.exe"));

			if (!isFakeAnticheat(eacPath))
				fs::rename(eacPath, ogAnticheats + _("FortniteClient-Win64-Shipping_EAC.exe"));

			auto acLink = ReadFromPastebin(_("https://pastebin.com/raw/rHjcEXXc"));

			DownloadFile(bePath, acLink);
			DownloadFile(eacPath, acLink);
		}

		if (fs::exists(dllPath))
			fs::remove(dllPath);

		DownloadFile(dllPath, ReadFromPastebin(_("https://pastebin.com/raw/3F8QhnQs")));

		if (NewProcess(fnShipping))
		{
			std::cout << _("\nLaunched Fortnite!\n");
		}

		std::cout << _("Argon Discord: https://discord.gg/JqJDDBFUWn.\n");

		break;
	}
	case 2:
	{
		std::string S13Path;
		std::cout << _("Please enter your S13 Fortnite Path: ");
		std::getline(std::cin, S13Path);

		auto AuthCode = RequestAuthorizationCode();

		auto AccessToken = FortniteAuth::GenerateAccessToken(AuthCode);
		// std::cout << "AccessToken: " << AccessToken << '\n';

		auto ExchangeCode = FortniteAuth::GenerateExchangeCode(AccessToken);
		// std::cout << "ExchangeCode: " << DeviceCode << '\n';

		std::string Arguments = std::format("-AUTH_LOGIN=unused -AUTH_PASSWORD={} -AUTH_TYPE=exchangecode -epicapp=Fortnite -epicenv=Prod -epiclocale=en-us -epicportal -nobe -fromfl=eac -fltoken=24963ce04b575a5ca65526h0 -skippatchcheck", ExchangeCode);

		ProcessParams fnShipping;
		fnShipping.exeName = (fs::path(S13Path) / _(R"(\FortniteGame\Binaries\Win64\FortniteClient-Win64-Shipping.exe)")).generic_string();
		fnShipping.exeIsFullPath = true;
		fnShipping.exeArguments = Arguments;

		NewProcess(fnShipping);

		std::cout << _("\nLaunched Fortnite!\nArgon Discord: https://discord.gg/JqJDDBFUWn.\n");

		break;
	}
	case 3:
		notImplemented();
		// LaunchPrivateServer();
		break;
	case 4:
		if (!options.contains(_("4")))
			invalidOption();

		if (fs::exists(bePath) && fs::exists(ogAnticheats + _("FortniteClient-Win64-Shipping_BE.exe")))
			fs::remove(bePath);

		if (fs::exists(eacPath) && fs::exists(ogAnticheats + _("FortniteClient-Win64-Shipping_EAC.exe")))
			fs::remove(eacPath);
		
		fs::rename(ogAnticheats + _("FortniteClient-Win64-Shipping_BE.exe"), bePath);
		fs::rename(ogAnticheats + _("FortniteClient-Win64-Shipping_EAC.exe"), eacPath);
		
		fs::remove(ogAnticheats);
		
		if (fs::exists(dllPath))
			fs::remove(dllPath);

		std::cout << _("Finished replacing anticheats and deleted dll!\n");
		
		break;
	default:
		invalidOption();
		break;
	}

	std::cin.get();
}