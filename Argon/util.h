#pragma once

#include <Windows.h>
#include <vector>
#include <iostream>
#include <string>
#include <chrono>
#include <fstream>
#include <format>

#include <xorstr.hpp>

using namespace std::chrono;

static void Exit(const std::string& message = "", UINT type = MB_ICONERROR, DWORD ExitCode = 0)
{
	if (message != "") MessageBoxA(0, message.c_str(), _("Argon"), type);
	FreeLibraryAndExitThread(GetModuleHandleW(0), ExitCode);
}

#define CHECK_PATTERN(sig, name) \
	if(!sig) \
		Exit(std::format("Could not find {}!", name));  \
	else \
		Logger::log.Write(std::string("Found ") + name); \

namespace Time
{
	static uint64_t GetEpochTime() { return (uint64_t)duration_cast<seconds>(system_clock::now().time_since_epoch()).count(); }
	static uint64_t GetEpochTimeInMilliseconds() { return (uint64_t)duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count(); }

	static const tm* GetCurrentLocalTime()
	{
		time_t now = time(0);
		tm Result;
		auto currentTime = localtime_s(&Result, &now);

		if (currentTime != 0)
		{
			std::cout << _("Couldn't not get current local time!\n");
			return nullptr;
		}
		return &Result;
	}
	static int GetCurrentYear() { return GetCurrentLocalTime()->tm_year + 1900; }
	static int GetCurrentMonth() { return GetCurrentLocalTime()->tm_mon + 1; }
	static int GetCurrentDay() { return GetCurrentLocalTime()->tm_mday; }
	static int GetCurrentHour() { return GetCurrentLocalTime()->tm_hour; }
	static int GetCurrentMinute() { return GetCurrentLocalTime()->tm_min; }
	static int GetCurrentSecond() { return GetCurrentLocalTime()->tm_sec; }
	static std::string GetFullTime() { return std::format(_("{}-{}-{} {}:{}"), GetCurrentMonth(), GetCurrentDay(), GetCurrentYear(), GetCurrentHour(), GetCurrentMinute()); }
};

class File
{
	std::string FileName;

public:

	void Write(std::string Str, const bool bLogTime = true)
	{
		if (bLogTime)
			Str = std::format("[{}] {}", Time::GetFullTime(), Str);

		std::fstream f;

		if (!FileName.contains(".txt"))
			FileName += ".txt";

		f.open(FileName, std::ios::out | std::ios::app);
		f << Str << '\n';
		f.close();
	}

	static void WriteToFile(std::string Filename, std::string Str, const bool bLogTime = true)
	{
		if (bLogTime)
			Str = std::format("[{}] {}", Time::GetFullTime(), Str);

		std::fstream f;

		if (!Filename.contains(".txt"))
			Filename += ".txt";

		f.open(Filename, std::ios::out | std::ios::app);
		f << Str << '\n';
		f.close();
	}

	File(const std::string& FileToOpen) : FileName(FileToOpen) {};
};

namespace Logger
{
	static bool bWriteToFile = true;
	static File log(_("Argon_log.txt"));

	enum class Color
	{

	};

	static void Log(const std::string& Str, bool prefix = true)
	{
		std::cout << _("[ARGON] ") << Str << '\n';

		if (bWriteToFile)
			log.Write(Str);
	}

	static void DebugLog(const std::string& Str);

#ifdef DEV
	static void DebugLog(const std::string& Str)
	{
		std::cout << _("[ARGON DEBUG] ") << Str << '\n';

		if (bWriteToFile)
			log.Write(Str);
	}
#else
	static void DebugLog(const std::string& Str) {};
#endif

	static void ChangeColor(Color& color)
	{

	}
}
