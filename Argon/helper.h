#pragma once

#include <format>

#include "util.h"
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

    namespace Kismet
    {
        static UObject* Class = nullptr;

        auto DestroyActor(UObject* Actor)
        {
            if (!Actor) return;

            static auto fn = Actor->Function(_("K2_DestroyActor"));
			
            if (!fn)
            {
                Logger::Log(_("[WARNING] Could not find K2_DestroyActor for ") + Actor->GetFullName());
                return;
            }
			
            Actor->ProcessEvent(fn, nullptr);
        }
    }

    namespace Easy
    {
        UObject* SpawnObject(UObject* ObjectClass, UObject* Outer)
        {
            if (!ObjectClass || !Outer)
                return nullptr;
			
            struct {
                UObject* ObjectClass;
                UObject* Outer;
                UObject* ReturnValue;
            } params{};

            params.ObjectClass = ObjectClass;
            params.Outer = Outer;

            static auto fn = GameplayStatics::GetSpawnObject();

            GameplayStatics::GetClass()->ProcessEvent(fn, &params);

            return params.ReturnValue;
        }
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
			
            auto ConsoleObject = Easy::SpawnObject(ConsoleClass, *GameViewport);

            if (ConsoleObject)
                *ViewportConsole = ConsoleObject;
            else
                Logger::Log(_("[WARNING] SpawnObject failed to create console!"));
			
            return 0;
        }

        static void Say(FString&& Str)
        {
            // while (!Globals::GetWorld(true))
            // {
                // Sleep(1000 / 30);
            // }

            auto GameMode = *Globals::GetWorld(true)->Member<UObject*>(_("AuthorityGameMode"));

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

        static bool IsSetup()
        {
            return CheatManager ? *CheatManager != nullptr : false; // The game will crash if you try to dereference a nullptr
        }

        static DWORD WINAPI Setup(LPVOID)
        {
            if (IsSetup())
                return 0;

            while (!Globals::GetPC(true))
            {
                Sleep(1000 / 30);
            }
			
            CheatManager = Globals::GetPC()->Member<UObject*>(_("CheatManager"));
            static auto CheatManagerClass = FindObject(_("Class /Script/Engine.CheatManager"));

            auto CheatManagerObject = Easy::SpawnObject(CheatManagerClass, Globals::GetPC());

			if (CheatManagerObject)
                *CheatManager = CheatManagerObject;
            else
                Logger::Log(_("[WARNING] SpawnObject failed to create CheatManager!"));

            return 0;
        }

        static DWORD WINAPI SetupFort(LPVOID) // Create FortCheatManager
        {
            if (IsSetup())
                return 0;

            while (!Globals::GetPC(true))
            {
                Sleep(1000 / 30);
            }

            CheatManager = Globals::GetPC()->Member<UObject*>(_("CheatManager"));
            static auto CheatManagerClass = FindObject(_("Class /Script/FortniteGame.FortCheatManager"));

            auto CheatManagerObject = Easy::SpawnObject(CheatManagerClass, Globals::GetPC());

            if (CheatManagerObject)
                *CheatManager = CheatManagerObject;
            else
                Logger::Log(_("[WARNING] SpawnObject failed to create FortCheatManager!"));

            return 0;
        }

        static void Destroy()
        {
            if (!IsSetup()) return;

            Logger::Log(_("Destroying CheatManager!"));

            (*CheatManager)->ObjectFlags = EObjectFlags::RF_NoFlags;
			*CheatManager = nullptr;
            CheatManager = nullptr;
        }
    }

    namespace Creative
    {
        static bool IsInCreative()
        {
            return Globals::GetWorld()->GetFullName() == _("World /Game/Creative/Maps/Creative_NoApollo_Terrain.Creative_NoApollo_Terrain");
        }
    }

    auto ChangeRole(UObject* Actor, ENetRole Role, bool bLocal = true)
    {
        if (!Actor || Role == ENetRole::ROLE_None || Role == ENetRole::ROLE_MAX)
            return ENetRole::ROLE_None;

        if (bLocal)
            *Actor->Member<TEnumAsByte<ENetRole>>(_("Role")) = Role;
		
        else
            *Actor->Member<TEnumAsByte<ENetRole>>(_("RemoteRole")) = Role;

        return Role;
    }
    
    auto ChangeRoles(UObject* Actor, ENetRole Role)
    {
        if (!Actor || Role == ENetRole::ROLE_None || Role == ENetRole::ROLE_MAX)
            return;

        ChangeRole(Actor, Role, false);
        ChangeRole(Actor, Role);
    }
}