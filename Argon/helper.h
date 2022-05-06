#include <format>

#include "globals.h"

DWORD WINAPI DumpObjects(LPVOID)
{
    std::ofstream obj(_("Objects.txt"));

    obj << _("[ARGON] Fortnite Version: ") << FN_Version << "\n\n";

    for (int32_t i = 0; i < (ObjObjects ? ObjObjects->Num() : OldObjects->Num()); i++)
    {
        auto Object = ObjObjects ? ObjObjects->GetObjectById(i) : OldObjects->GetObjectById(i);

        if (!Object) continue;

        obj << std::format(_("[{}] {}\n"), Object->InternalIndex, Object->GetFullName());
    }

    obj.close();

    Logger::Log(_("Dumped Objects!"));

    return 0;
}

namespace Helper
{
    namespace GameplayStatics
    {
        static UObject* Class = nullptr;
        static UObject* SpawnObjectFunction = nullptr;

        auto GetClass()
        {
            if (!Class)
                Class = FindObject(_("GameplayStatics /Script/Engine.Default__GameplayStatics"));

            return Class;
        }

        auto GetSpawnObject()
        {
			if (!SpawnObjectFunction)
                SpawnObjectFunction = GetClass()->Function(_("SpawnObject"));

            return SpawnObjectFunction;
        }
    }

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

            static auto fn = GameplayStatics::GetSpawnObject();

            GameplayStatics::GetClass()->ProcessEvent(fn, &params);

            *ViewportConsole = params.ReturnValue;

            Logger::DebugLog(_("Console created!"));

            return 0;
        }

        static void Say(FString Str)
        {
            while (!Globals::GetWorld(true))
            {
                Sleep(1000 / 30);
            }

            auto GameMode = *Globals::GetWorld()->Member<UObject*>(_("AuthorityGameMode"));

            while (!GameMode)
            {
                GameMode = *Globals::GetWorld()->Member<UObject*>(_("AuthorityGameMode"));
                Sleep(1000 / 30);
            }
            
            static auto say = GameMode->Function(_("Say"));

            GameMode->ProcessEvent(say, &Str);
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
			
            CheatManager = Globals::GetPC()->Member<UObject*>(_("CheatManager"));
            static auto CheatManagerClass = FindObject(_("Class /Script/Engine.CheatManager"));

            struct
            {
                UObject* ObjectClass;
                UObject* Outer;
                UObject* ReturnValue;
            } params{};

            params.ObjectClass = CheatManagerClass;
            params.Outer = Globals::PC;

            GameplayStatics::GetClass()->ProcessEvent(GameplayStatics::GetSpawnObject(), &params);

            *CheatManager = params.ReturnValue;

            return 0;
        }
    }
}