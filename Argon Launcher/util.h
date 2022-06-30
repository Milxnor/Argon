#pragma once

#include <string>
#include <stringapiset.h>
#include <string>
#include <Windows.h>
#include <filesystem>

namespace fs = std::filesystem;

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
	const int numArgs = 0;
};

std::wstring get_utf16(const std::string& str, int codepage)
{
    if (str.empty()) return std::wstring();
    int sz = MultiByteToWideChar(codepage, 0, &str[0], (int)str.size(), 0, 0);
    std::wstring res(sz, 0);
    MultiByteToWideChar(codepage, 0, &str[0], (int)str.size(), &res[0], sz);
    return res;
}

bool NewProcess(const ProcessParams& params)
{
	// TODO: Add path checking to see if it exists/is a dll/is a exe.

	// Setting wpath and path to the full path

	std::string path = params.exeName;
	std::string args = params.exeArguments;
	const char* dllPath = params.dllName;
	DWORD creationFlags = params.creationFlags;
	bool fullDirectory = params.exeIsFullPath;

	if (!path.contains(_(".exe"))) path += _(".exe");

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

	auto ProcName = (params.numArgs > 0) ? nullptr : fspath.string().c_str();

	if (!ProcName)
		args = std::format(_("\"{}\" {}"), fspath.string(), args);

	std::cout << std::format(_("Starting {} with args {}\n"), fspath.string(), args);

	// std::wcout << wpath.c_str() << '\n';

	if (!CreateProcessA(ProcName,// const_cast<LPCWSTR>(wpath.c_str()),
		(char*)args.c_str(),// fspath.string(),
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
		// 2 = file not found
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

std::string GetEnv(const std::string& Env)
{
	char* buf{};
	size_t size = MAX_PATH;
	_dupenv_s(&buf, &size, Env.c_str());

	std::string Path = buf;

	if (buf)
		free(buf);

	return Path;
}