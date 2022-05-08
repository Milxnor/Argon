#include <iostream>
#include <xorstr.hpp>
#include <string>

#include <Windows.h>
#include <filesystem>
#include <fstream>
#include <json.hpp>

#pragma comment(lib, "urlmon.lib")

namespace fs = std::filesystem;
using namespace nlohmann;

const static auto LauncherVersion = "0.1";

std::wstring widen(const std::string& s)
{
	std::vector<wchar_t> buf(MultiByteToWideChar(CP_ACP, 0, s.c_str(), s.size() + 1, 0, 0));
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), s.size() + 1, &buf[0], buf.size());
	return std::wstring(&buf[0]);
}

struct ProcessParams
{
	std::string exeName;
	std::string exeArguments = " ";
	const char* dllName = "";
	DWORD creationFlags = CREATE_NEW_CONSOLE;
	bool exeIsFullPath = false;
	bool dllIsFullPath = false;
};

bool NewProcess(const ProcessParams& params)
{
	// TODO: Add path checking to see if it exists/is a dll/is a exe.

	// Setting wpath and path to the full path

	std::string path = params.exeName;
	std::string args = params.exeArguments;
	const char* dllPath = params.dllName;
	DWORD creationFlags = params.creationFlags;
	bool fullDirectory = params.exeIsFullPath;

	if (!path.contains(_(".exe"))) path = path + _(".exe"); // TODO: Test path.append(".exe");

	std::wstring wpath;
	fs::path fspath = fs::current_path() / path;
	if (!fullDirectory)
	{
		// path = fspath.string(); // not sure if we need to update this as we may be able to just pass fspath.string() directly into wpath.
		wpath = widen(fspath.string());
		// std::cout << fspath.string() << std::endl;
	}

	// Make sure arguments isn't nothing.

	std::wstring wargs = widen(args);
	if (wargs.size() != 0)
	{
		if (wargs[0] != L' ') wargs.insert(0, L" ");
	}

	// memory stuff

	wchar_t* wcp = new wchar_t[wargs.size() + 1];
	const wchar_t* temp = wargs.c_str();
	wcscpy_s(wcp, wargs.size() + 1, temp);

	STARTUPINFOA si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	// std::cout << _("Starting ") << params.exeName << '\n';

	// std::wcout << wpath.c_str() << '\n';

	if (!CreateProcessA(fspath.string().c_str(),// const_cast<LPCWSTR>(wpath.c_str()),
		NULL,// fspath.string(),
		NULL,
		NULL,
		FALSE,
		creationFlags,
		NULL,
		NULL,
		&si,
		&pi)
		)
	{
		std::cout << _("Couldn't create process ") + fspath.filename().string() + _(" and the error code is ") << GetLastError() << std::endl;
	}
	else
	{

		if (dllPath != "")
		{
			std::string fullDllPath = (fs::current_path() / dllPath).string();
			if (fs::exists(fullDllPath)) std::cout << _("Injecting dll not implemented.");// std::cout << InjectDLL(pi.dwProcessId, fullDllPath, params.exeName.c_str()) << std::endl;
			else std::cout << _("Couldn't locate dll to inject!");
		}
		WaitForSingleObject(pi.hProcess, INFINITE);

		delete[]wcp;
		wcp = 0;
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);

		// Getting whole path and parsing it to only the filename.

		char path[MAX_PATH] = ""; // __FILE__ could also be a solution, but I'm pretty sure its on compile time.
		std::string strAppName;

		GetModuleFileNameA(0, path, MAX_PATH);

		// Extract name
		strAppName = path;
		strAppName = strAppName.substr(strAppName.rfind("\\") + 1);

		// Sleep(1000);
		// exit(0);
	}
}

bool isFakeAnticheat(const std::string& Path)
{
	auto fileSize = std::filesystem::file_size(Path);
	
	if (fileSize > 100000)
		return false;
	
	return true;
}

std::string GetFortnitePath()
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

		auto listOfGames = j[_("InstallationList")];

		for (auto game : listOfGames)
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
		std::cout << "Could not download file! Error: " << GetLastError() << '\n';
		return "";
	}

	std::ifstream input_file(temp);

	if (!input_file.is_open()) return "";

	auto data = std::string((std::istreambuf_iterator<char>(input_file)), std::istreambuf_iterator<char>());

	input_file.close();

	fs::remove(temp);

	return data;
}

int main(){
	auto newestLauncherVersion = ReadFromPastebin(_("https://pastebin.com/raw/mHpSTbRp"));
	
	if (newestLauncherVersion == LauncherVersion)
		std::cout << _("You are up to date!\n");
	else
	{
		std::cout << _("Please download the new launcher in the discord https://discord.gg/JqJDDBFUWn.\n");
		std::cin.get();
	}
	
	auto fnPath = GetFortnitePath();

	if (!fnPath.contains(_("\\")))
	{
		std::cout << fnPath << '\n';
		std::cin.get();
	}

	fnPath += _(R"(\FortniteGame\Binaries\Win64\)");
	
    std::string options = _(R"(
[1] - Launch
)");

	auto ogAnticheats = fnPath + _("Original Anticheats\\");

	if (fs::exists(ogAnticheats))
		options += _("[2] - Revert to old Fortnite\n");

	std::cout << options << "\n";
	
	std::string optStr;
	std::cout << _("Enter option: ");
	std::getline(std::cin, optStr);
	
	int opt = 0;
	opt = std::stoi(optStr);
	
	auto invalidOption = []() {
		std::cout << _("Invalid option!\n");
		std::cin.get();
	};

	static auto bePath = fnPath + _("FortniteClient-Win64-Shipping_BE.exe");
	static auto eacPath = fnPath + _("FortniteClient-Win64-Shipping_EAC.exe");
	static auto dllPath = fnPath + _("Argon.dll");
	
	switch (opt)
	{
	case 1:
	{	
		ProcessParams fnShipping;
		fnShipping.exeName = fnPath + _("FortniteClient-Win64-Shipping.exe");
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

		if (fs::exists(dllPath));
			fs::remove(dllPath);
		
		DownloadFile(dllPath, ReadFromPastebin(_("https://pastebin.com/raw/3F8QhnQs")));

		NewProcess(fnShipping);
		
		std::cout << "\nLaunched Fortnite!\nArgon Discord: https://discord.gg/JqJDDBFUWn.\n";
	
		break;
	}
	case 2:
		if (!options.contains(_("2")))
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