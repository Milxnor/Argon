#include <Windows.h>
#include <iostream>

#include "detours.h"
#include "structs.h"
#include "util.h"

DWORD WINAPI Input(LPVOID)
{
    while (1)
    {

        Sleep(1000 / 30);
    }
}

DWORD WINAPI Main(LPVOID)
{
    AllocConsole();

    FILE* file;
    freopen_s(&file, _("CONOUT$"), _("w"), stdout);

    auto InitializationStatus = MH_Initialize();

    if (InitializationStatus != MH_OK)
    {
        MessageBoxA(0, _("MinHook failed to initialize.\n"), _("Argon"), MB_ICONERROR);
        FreeLibraryAndExitThread(GetModuleHandleW(0), 0);
    }

    auto Ascii = _(R"(
  /$$$$$$                                         
 /$$__  $$                                        
| $$  \ $$  /$$$$$$   /$$$$$$   /$$$$$$  /$$$$$$$ 
| $$$$$$$$ /$$__  $$ /$$__  $$ /$$__  $$| $$__  $$
| $$__  $$| $$  \__/| $$  \ $$| $$  \ $$| $$  \ $$
| $$  | $$| $$      | $$  | $$| $$  | $$| $$  | $$
| $$  | $$| $$      |  $$$$$$$|  $$$$$$/| $$  | $$
|__/  |__/|__/       \____  $$ \______/ |__/  |__/
                     /$$  \ $$                    
                    |  $$$$$$/                    
                     \______/                     
)");

    Logger::log.Write("\n\n\n", false);

    std::cout << Ascii << "\n\n";

    if (!Setup(ProcessEventDetour))
    {
		MessageBoxA(0, _("Failed to setup.\n"), _("Argon"), MB_ICONERROR);
		FreeLibraryAndExitThread(GetModuleHandleW(0), 0);
    }

    Logger::Log(_("Setting up Argon v0.1. Made by Milxnor#3531."));

    auto cURLEasyAddr = FindPattern(_("89 54 24 10 4C 89 44 24 18 4C 89 4C 24 20 48 83 EC 28 48 85 C9"));
    CHECK_PATTERN(cURLEasyAddr, _("curl_easy_setopt"));
    curl_easy_setopt = decltype(curl_easy_setopt)(cURLEasyAddr);

    MH_CreateHook((PVOID)cURLEasyAddr, curl_easy_setoptDetour, (PVOID*)&curl_easy_setopt);
    MH_EnableHook((PVOID)cURLEasyAddr);
	
    CreateThread(0, 0, Input, 0, 0, 0);

    Logger::Log(_("Hooked and found every pattern successfully!"));
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dllReason, LPVOID lpReserved)
{
    switch (dllReason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        CreateThread(0, 0, Main, 0, 0, 0);
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
