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

        static TArray<UObject*> GetAllActorsOfClass(UObject* Class)
        {
            static UObject* GetAllActorsOfClass = FindObject("Function /Script/Engine.GameplayStatics.GetAllActorsOfClass");

            struct
            {
                UObject* World;
                UObject* Class;
                TArray<UObject*> ReturnValue;
            } Params;

            Params.World = Globals::GetWorld(true);
            Params.Class = Class;

            ProcessEventO(GetClass(), GetAllActorsOfClass, &Params);

            return Params.ReturnValue;
        }
    }

    namespace Kismet
    {
        static UObject* Class = nullptr;
        static UObject* FortKismetClass = nullptr;

        auto GetFortKismet()
        {
			if (!FortKismetClass)
                FortKismetClass = FindObject(_("FortKismetLibrary / Script / FortniteGame.Default__FortKismetLibrary"));
            
            return FortKismetClass;
        }

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

        bool ApplyCharacterCosmetics(UObject* Pawn, UObject* Body, UObject* Head)
        {
			// ApplyCharacterCosmetics(struct UObject* WorldContextObject, struct TArray<struct UCustomCharacterPart*> CharacterParts, struct AFortPlayerState* PlayerState, bool& bSuccess);

            struct {
                UObject* World;
				TArray<UObject*> Parts;
                UObject* PlayerState;
                bool* bSuccess;
            } params{};

            static auto Hero = FindObject(_("FortHero /Engine/Transient.FortHero_"));

            if (!Hero)
            {
                Logger::Log(_("[WARNING] Could not find FortHero!"));
                return false;
            }
			
            auto CharacterParts = Hero->Member<TArray<UObject*>>(_("CharacterParts"));
			
            if (!CharacterParts)
            {
				Logger::Log(_("Unable to find CharacterParts!"));
                return false;
            }
			
            CharacterParts->At(0) = Body;
            CharacterParts->At(1) = Head;

			params.World = Globals::GetWorld(true);
            params.Parts = *CharacterParts;
            params.PlayerState = *Pawn->Member<UObject*>(_("PlayerState"));

            bool Success = false;
            params.bSuccess = &Success;

			auto fn = GetFortKismet()->Function(_("ApplyCharacterCosmetics"));

            if (fn)
                GetFortKismet()->ProcessEvent(fn, &params);
            else
                std::cout << _("Unable to find ApplyCharacterCosmetics!\n");
			
            return Success;
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

        UObject* SpawnActor(UObject* Class, FVector Location, FRotator Rotation = FRotator())
        {
            FQuat SpawnQuat{};
            SpawnQuat.W = 0;
            SpawnQuat.X = Rotation.Pitch;
            SpawnQuat.Y = Rotation.Roll;
            SpawnQuat.Z = Rotation.Yaw;

            FTransform SpawnTrans{};
            SpawnTrans.Scale3D = FVector(1, 1, 1);
            SpawnTrans.Translation = Location;
            SpawnTrans.Rotation = SpawnQuat;

            return SpawnActorO(Globals::GetWorld(true), Class, &SpawnTrans, FActorSpawnParameters());
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

            if (CheatManager && *CheatManager)
            {
                (*CheatManager)->ObjectFlags = EObjectFlags::RF_NoFlags;
                *CheatManager = nullptr;
            }
			
            CheatManager = nullptr;
        }
    }

    namespace Creative
    {
        static bool IsInCreative()
        {
            return Globals::GetWorld()->GetFullName() == _("World /Game/Creative/Maps/Creative_NoApollo_Terrain.Creative_NoApollo_Terrain");
        }

        static void GiveItem(FFortItemEntry entry)
        {
            struct {
                FFortItemEntry CreativeItem;
                FGuid ItemToRemoveGuid;
            } params{};

            params.CreativeItem = entry;
            params.ItemToRemoveGuid = FGuid();

            static auto fn = Globals::GetPC(true)->Function(_("ServerGiveCreativeItem"));
            Globals::GetPC()->ProcessEvent(fn, &params);
        }

        static void GiveItem(UObject* ItemDef)
        {
            struct {
                FFortItemEntry CreativeItem;
                FGuid ItemToRemoveGuid;
            } params{};

            FFortItemEntry newEntry = {};
            newEntry.bIsReplicatedCopy = true;
            newEntry.Durability = 1;
            newEntry.Count = 1;
            newEntry.Count = 1;
            newEntry.LoadedAmmo = 999;
            newEntry.Level = 1;
            newEntry.ItemDefinition = ItemDef;

            params.CreativeItem = newEntry;
            params.ItemToRemoveGuid = FGuid();

            static auto fn = Globals::GetPC(true)->Function(_("ServerGiveCreativeItem"));
            Globals::GetPC()->ProcessEvent(fn, &params);
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

    void ChangePart()
    {
		
    }

    bool SetCharacterPartVisibility(EFortCustomPartType Part, bool bVisible = false)
    {
        if (!Globals::GetPawn(true))
            return false;

        static auto fn = Globals::GetPawn()->Function(_("SetCharacterPartVisibility"));

        struct {
            EFortCustomPartType InPartType;
            bool bNewVisibility;
            bool bPropagateToChildren;
            bool ReturnValue;
        } params{};

        params.InPartType = Part;
        params.bNewVisibility = bVisible;
        params.bPropagateToChildren = true;

        Globals::GetPawn()->ProcessEvent(fn, &params);

        return params.ReturnValue;
    }

    FVector GetActorLocation(UObject* Actor)
    {
        static auto fn = Actor->Function(_("K2_GetActorLocation"));

        if (!fn)
            return FVector();

        FVector Ret;
        Actor->ProcessEvent(fn, &Ret);

        return Ret;
    }

    void SpawnPickup(UObject* ItemDef, int Count, EFortPickupSourceTypeFlag Flags, EFortPickupSpawnSource Src, UObject* Pawn)
    {
        if (!ItemDef)
            return;

        auto summonLoc = GetActorLocation(Pawn);
        auto Pickup = Easy::SpawnActor(FindObject(_("Class /Script/FortniteGame.FortPickupAthena")), summonLoc);

        auto PickupEntry = Pickup->Member<FFortItemEntry>(_("PrimaryPickupItemEntry"));

        if (!PickupEntry)
        {
            std::cout << _("Unable to find ItemEntry!\n");
            return;
        }

        *(UObject**)((uintptr_t)PickupEntry + 0x18) = ItemDef;
        *reinterpret_cast<int*>((uintptr_t)PickupEntry + 0x0c) = Count;

        static auto PrimaryPickupItemEntryFn = Pickup->Function(_("OnRep_PrimaryPickupItemEntry"));
        static auto TossPickupFn = Pickup->Function(_("TossPickup"));

        struct
        {
            FVector FinalLocation;
            UObject* Pawn;
            int32_t OverrideMaxStackCount;
            bool bToss;
            bool bShouldCombinePickupsWhenTossCompletes;
            EFortPickupSourceTypeFlag InPickupSourceTypeFlags;
            EFortPickupSpawnSource InPickupSpawnSource;
        } params{};
        params.FinalLocation = GetActorLocation(Pawn);
        params.bToss = true;
        params.bShouldCombinePickupsWhenTossCompletes = true;
        params.Pawn = Pawn;
        params.OverrideMaxStackCount = 999;
        params.InPickupSourceTypeFlags = Flags;
        params.InPickupSpawnSource = Src;

        if (Pickup && PrimaryPickupItemEntryFn)
            Pickup->ProcessEvent(PrimaryPickupItemEntryFn, nullptr);
		
        if (Pickup && TossPickupFn)
            Pickup->ProcessEvent(TossPickupFn, &params);
    }

    void CreateBuildingActor(UObject* BuildingClass, char UpgradeLevel)
    {
        FCreateBuildingActorData data{};
        FBuildingClassData classData{};
        classData.BuildingClass = BuildingClass;
        data.BuildingClassData = classData;
        data.BuildRot = FRotator();
        classData.UpgradeLevel = UpgradeLevel;
        data.BuildLoc = GetActorLocation(Globals::GetPawn(true));
        
		static auto fn = Globals::GetPC()->Function(_("ServerCreateBuildingActor"));

        Globals::GetPC()->ProcessEvent(fn, &data);
    }
	
    static auto PngToTexture2D(FString& Path)
    {
		static auto renderingLibrary = FindObject(_("KismetRenderingLibrary /Script/Engine.Default__KismetRenderingLibrary"));

		static auto fn = renderingLibrary->Function(_("ImportFileAsTexture2D"));

        struct {
            UObject* World;
            FString FileName;
            UObject* ReturnValue;
        } params;

        params.World = Globals::GetWorld(true);
        params.FileName = Path;
        
        renderingLibrary->ProcessEvent(fn, &params);

        return params.ReturnValue;
    }

    static auto StringToName(FString Str)
    {
        struct {
            FString String;
            FName ReturnValue;
        } params;

        params.String = Str;
		
        static auto renderingLibrary = FindObject(_("KismetStringLibrary /Script/Engine.Default__KismetStringLibrary"));

        static auto fn = renderingLibrary->Function(_("Conv_StringToName"));
		
		renderingLibrary->ProcessEvent(fn, &params);

        return params.ReturnValue;
    }

    static void SetPartTextureFromPng(EFortCustomPartType PartType, FString& Path)
    {
        struct {
            FName ParameterName;
            UObject* Value; // Texture
        } params{};

        params.ParameterName = StringToName(_(L"Diffuse"));
		params.Value = PngToTexture2D(Path);

        for (auto i = 0; i < 6; i++)
        {
            auto MaterialInstanceDynamic = FindObject(
                _("MaterialInstanceDynamic /Game/Athena/Artemis/Maps/Artemis_Terrain.Artemis_Terrain.PersistentLevel.PlayerPawn_Athena_C_"), false, false, i);

            if (!MaterialInstanceDynamic)
            {
                MaterialInstanceDynamic = FindObject(
                    _("MaterialInstanceDynamic /Game/Maps/Frontend.Frontend.PersistentLevel.PlayerPawn_Athena_C_"), false, false, i);

                if (!MaterialInstanceDynamic)
                    MaterialInstanceDynamic = FindObject(
                        _("MaterialInstanceDynamic /Game/Creative/Maps/Creative_NoApollo_Terrain.Creative_NoApollo_Terrain.PersistentLevel.PlayerPawn_Athena_C_"), false, false, i);
            }

            if (MaterialInstanceDynamic)
            {
				static auto fn = MaterialInstanceDynamic->Function(_("SetTextureParameterValue"));

                std::string toFind;
				
                switch (PartType)
                {
                case EFortCustomPartType::Head:
                    toFind = _("CharacterPartSkelMesh_Head");
                    break;
                case EFortCustomPartType::Body:
                    toFind = _("CharacterPartSkelMesh_Body");
                    break;
                default:
                    toFind = _("Undefined");
                    break;
                }

                if (toFind != _("Undefined") && MaterialInstanceDynamic->GetFullName().contains(toFind))
                {
                    MaterialInstanceDynamic->ProcessEvent(fn, &params);
                    return;
                }
            }
            else
            {
                std::cout << _("Unable to find MaterialInstanceDynamic!\n");
                break;
            }
        }
    }

    UObject* GetCurrentWeapon()
    {
        return *Globals::GetPawn(true)->Member<UObject*>(_("CurrentWeapon"));
    }

    namespace Conversion
    {
        static FText FStringToFText(FString Str)
        {
            static auto TextLibrary = FindObject(_("Class /Script/Engine.KismetTextLibrary"));
            static auto fn = FindObject(_("Function /Script/Engine.KismetTextLibrary.Conv_StringToText"));

            if (!TextLibrary || !fn || !Str.Data.Data)
                return FText();

            struct {
                FString inString;
                FText ReturnValue;
            } params;

            params.inString = Str;
		
			TextLibrary->ProcessEvent(fn, &params); // crash here

			return params.ReturnValue;
        }
    }

    namespace Widget
    {
        static UObject* CreateUserWidget()
        {
            static auto UserWidgetClass = FindObject(_("Class /Script/UMG.UserWidget"));

            auto Widget = Easy::SpawnObject(UserWidgetClass, Globals::GetPC(true));

            std::cout << "Widget:" << Widget << '\n';

            return Widget;
        }

        namespace UserWidget
        {
            static void AddToViewport(UObject* UserWidget)
            {
                static auto fn = FindObject(_("Function /Script/UMG.UserWidget.AddToViewport"));

                struct {
                    int32_t ZOrder;
                } params{};

                params.ZOrder = 10;
				
                UserWidget->ProcessEvent(fn, &params);
            }
        }
    }
}