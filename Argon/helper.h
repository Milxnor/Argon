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
            static auto GameViewport = Globals::GetEngine()->Member<UObject*>(_("GameViewport"));
			
            while (!*GameViewport)
            {
                GameViewport = Globals::GetEngine()->Member<UObject*>(_("GameViewport"));
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
            static auto fn = GSC->Function(_("SpawnObject"));

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

            auto GameMode = *Globals::GetWorld()->Member<UObject*>(_("AuthorityGameMode"));

            while (!GameMode)
            {
                GameMode = *Globals::GetWorld()->Member<UObject*>(_("AuthorityGameMode"));
            }

            GameMode->ProcessEvent(GameMode->Function(_("Say")), &Str);
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
			
            auto statics = FindObject(_("GameplayStatics /Script/Engine.Default__GameplayStatics"));
            CheatManager = Globals::GetPC()->Member<UObject*>(_("CheatManager"));
            auto CheatManagerClass = FindObject(_("Class /Script/Engine.CheatManager"));

            struct
            {
                UObject* ObjectClass;
                UObject* Outer;
                UObject* ReturnValue;
            } params{};

            params.ObjectClass = CheatManagerClass;
            params.Outer = Globals::PC;

            statics->ProcessEvent(statics->Function(_("SpawnObject")), &params);

            *CheatManager = params.ReturnValue;

            return 0;
        }
    }
}