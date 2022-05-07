#include <Windows.h>
#include <iostream>

#include "detours.h"
#include "util.h"
#include "helper.h"
#include "gui.h"

DWORD WINAPI Input(LPVOID)
{
    while (1)
    {
        if (GetAsyncKeyState(VK_F9) & 1)
            CreateThread(0, 0, DumpObjects, 0, 0, 0);

        else if (GetAsyncKeyState(VK_F3) & 1)
            CreateThread(0, 0, Helper::CheatManager::Setup, 0, 0, 0);

        else if (GetAsyncKeyState(VK_F5) & 1)
            CreateThread(0, 0, Helper::CheatManager::SetupFort, 0, 0, 0);

        else if (GetAsyncKeyState(VK_F6) & 1)
            Helper::CheatManager::Destroy();

        else if (GetAsyncKeyState(VK_F4) & 1)
        {
            if (Helper::CheatManager::IsSetup())
                Logger::Log(_("CheatManager is setup."));
            else
                Logger::Log(_("CheatManager is not setup."));
        }

        Sleep(1000 / 30);
    }
}

DWORD WINAPI Startup(LPVOID)
{
    CreateThread(0, 0, Helper::Console::Setup, 0, 0, 0);
    // CreateThread(0, 0, Helper::CheatManager::Setup, 0, 0, 0);

    Sleep(1500);

    Helper::Console::Say(_(L"Welcome to Argon.\n\nKeybinds:\nF9 - Dump Objects\nF8 - Opens GUI.\nF4 - Check CheatManager Status\nF3 - Make CheatManager\ncheatscript Help - CheatScript Commands\n\nDiscord Invite: https://discord.gg/JqJDDBFUWn."));

    return 0;
}

DWORD WINAPI Main(LPVOID)
{
    AllocConsole();

    FILE* file;
    freopen_s(&file, _("CONOUT$"), _("w"), stdout);

    auto InitializationStatus = MH_Initialize();

    if (InitializationStatus != MH_OK)
    {
        MessageBoxA(0, _("MinHook failed to initialize."), _("Argon"), MB_ICONERROR);
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

    Logger::Log(std::format(_("Setting up Argon v{}. Made by Milxnor#3531."), ArgonVersion));

    if (!Setup(ProcessEventDetour))
    {
		MessageBoxA(0, _("Failed to setup."), _("Argon"), MB_ICONERROR);
		FreeLibraryAndExitThread(GetModuleHandleW(0), 0);
    }
    
#if DEVELOPMENT == 0
    if (!FindObject(_("FortEngine_")))
    {
        MessageBoxA(0, _("Unfortunately, Argon does not work when injected on startup. Please inject in lobby."), _("Argon"), MB_ICONINFORMATION);
        FreeLibraryAndExitThread(GetModuleHandleW(0), 0);
    }
#endif

    auto cURLEasyAddr = FindPattern(_("89 54 24 10 4C 89 44 24 18 4C 89 4C 24 20 48 83 EC 28 48 85 C9"));
	
    if (!cURLEasyAddr)
        std::cout << _("[WARNING] Failed to find curl_easy_setopt.\n");

    auto RequestExitWithStatusAddr = FindPattern(_("48 8B C4 48 89 58 18 88 50 10 88 48 08"));
    CHECK_PATTERN(RequestExitWithStatusAddr, _("RequestExitWithStatus"));

    if (cURLEasyAddr)
    {
        MH_CreateHook((PVOID)cURLEasyAddr, curl_easy_setoptDetour, (PVOID*)&curl_easy_setopt);
        MH_EnableHook((PVOID)cURLEasyAddr);
    }
	
    MH_CreateHook((LPVOID)RequestExitWithStatusAddr, RequestExitWithStatusDetour, (LPVOID*)&RequestExitWithStatusOriginal);
    MH_EnableHook((LPVOID)RequestExitWithStatusAddr);
	
    CreateThread(0, 0, Input, 0, 0, 0);
    CreateThread(0, 0, Startup, 0, 0, 0);
    CreateThread(0, 0, GuiHook, 0, 0, 0);

    Logger::Log(_("Hooked and found every pattern successfully!"));

    return 0;
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
