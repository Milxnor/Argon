#include "globals.h"

namespace Helper
{
    namespace Easy
    {

		    }

    namespace Console
    {
        static UObject** ViewportConsole = nullptr;

        DWORD WINAPI Setup(LPVOID)
        {
            Globals::Engine = FindObject(_("FortEngine_"));

            while (!Globals::Engine)
            {
                Globals::Engine = FindObject(_("FortEngine_"));
                Sleep(1000 / 30);
            }

            static auto ConsoleClass = FindObject(_("Class /Script/Engine.Console"));
            static auto GameViewport = Globals::Engine->Member<UObject*>(_("GameViewport"));
			
            while (!*GameViewport)
            {
                GameViewport = Globals::Engine->Member<UObject*>(_("GameViewport"));
                Sleep(1000 / 30);
            }
			
            ViewportConsole = (*GameViewport)->Member<UObject*>(_("ViewportConsole"));

            struct {
                UObject* ObjectClass;
                UObject* Outer;
                UObject* ReturnValue;
            } params{};

            params.ObjectClass = ConsoleClass;
            params.Outer = *GameViewport;

            static auto GSC = FindObject(_("GameplayStatics /Script/Engine.Default__GameplayStatics"));
            static auto fn = FindObject(_("Function /Script/Engine.GameplayStatics.SpawnObject")); // GSC->Function(_("SpawnObject"));

            GSC->ProcessEvent(fn, &params);

            *ViewportConsole = params.ReturnValue;

            Logger::DebugLog(_("Console created!"));

            return 0;
        }

        static void Say(FString Str)
        {
            while (Globals::GetWorld(true))
            {
                Sleep(1000 / 30);
            }

            static auto fn = FindObject(_("Function /Script/Engine.GameMode.Say"), true);

            auto GameMode = *Globals::World->Member<UObject*>(_("AuthorityGameMode"));

            while (!GameMode)
            {
                GameMode = *Globals::World->Member<UObject*>(_("AuthorityGameMode"));
            }

            GameMode->ProcessEvent(fn, &Str);
        }
    }

    namespace CheatManager
    {
        static UObject** CheatManager = nullptr;

        static DWORD WINAPI Setup(LPVOID)
        {
            while (!Globals::GetPC(true))
            {
                Sleep(1000 / 30);
            }
			
            auto fn = FindObject(_("Function /Script/Engine.GameplayStatics.SpawnObject"));
            auto statics = FindObject(_("GameplayStatics /Script/Engine.Default__GameplayStatics"));
            CheatManager = Globals::PC->Member<UObject*>(_("CheatManager"));
            auto CheatManagerClass = FindObject(_("Class /Script/Engine.CheatManager"));

            struct
            {
                UObject* ObjectClass;
                UObject* Outer;
                UObject* ReturnValue;
            } params{};

            params.ObjectClass = CheatManagerClass;
            params.Outer = Globals::PC;

            statics->ProcessEvent(fn, &params);

            *CheatManager = params.ReturnValue;

            return 0;
        }
    }
}