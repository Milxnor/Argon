#pragma once

#include <atlconv.h>

#include "helper.h"
#include "util.h"
#include "gui.h"

static bool bHasSetup = false;

DWORD WINAPI Startup(LPVOID)
{
	if (bHasSetup)
		return 0;

	CreateThread(0, 0, Helper::Console::Setup, 0, 0, 0);
	// CreateThread(0, 0, Helper::CheatManager::Setup, 0, 0, 0);

	Sleep(1500);

	Helper::Console::Say(_(L"Welcome to Argon.\n\nKeybinds:\nF9 - Dump Objects\nF8 - Opens GUI.\nF4 - Check CheatManager Status\nF3 - Make CheatManager\ncheatscript Help - CheatScript Commands\n\nDiscord Invite: https://discord.gg/JqJDDBFUWn."));

	bHasSetup = true;

	return 0;
}

void* ProcessEventDetour(UObject* Object, UObject* Function, void* Params)
{	
	if (Object && Function)
	{
		auto FunctionName = Function->GetFullName();

		if (bLogProcessEvent)
		{
			if (!FunctionName.contains(_("EvaluateGraphExposedInputs")) &&
				!FunctionName.contains(_("Tick")) &&
				!FunctionName.contains(_("OnSubmixEnvelope")) &&
				!FunctionName.contains(_("OnSubmixSpectralAnalysis")) &&
				!FunctionName.contains(_("OnMouse")) &&
				!FunctionName.contains(_("Pulse")) &&
				!FunctionName.contains(_("BlueprintUpdateAnimation")) &&
				!FunctionName.contains(_("BlueprintPostEvaluateAnimation")) &&
				!FunctionName.contains(_("BlueprintModifyCamera")) &&
				!FunctionName.contains(_("BlueprintModifyPostProcess")) &&
				!FunctionName.contains(_("Loop Animation Curve")) &&
				!FunctionName.contains(_("UpdateTime")) &&
				!FunctionName.contains(_("GetMutatorByClass")) &&
				!FunctionName.contains(_("UpdatePreviousPositionAndVelocity")) &&
				!FunctionName.contains(_("IsCachedIsProjectileWeapon")) &&
				!FunctionName.contains(_("LockOn")) &&
				!FunctionName.contains(_("GetAbilityTargetingLevel")) &&
				!FunctionName.contains(_("ReadyToEndMatch")) &&
				!FunctionName.contains(_("ReceiveDrawHUD")) &&
				!FunctionName.contains(_("OnUpdateDirectionalLightForTimeOfDay")) &&
				!FunctionName.contains(_("GetSubtitleVisibility")) &&
				!FunctionName.contains(_("GetValue")) &&
				!FunctionName.contains(_("InputAxisKeyEvent")) &&
				!FunctionName.contains(_("ServerTouchActiveTime")) &&
				!FunctionName.contains(_("SM_IceCube_Blueprint_C")) &&
				!FunctionName.contains(_("OnHovered")) &&
				!FunctionName.contains(_("OnCurrentTextStyleChanged")) &&
				!FunctionName.contains(_("OnButtonHovered")) &&
				!FunctionName.contains(_("ExecuteUbergraph_ThreatPostProcessManagerAndParticleBlueprint")) &&
				!FunctionName.contains(_("BlueprintThreadSafeUpdateAnimation")) && 
				!FunctionName.contains(_("SetContentColorAndOpacity")) &&
				!FunctionName.contains(_("GetReticleColorOverride")) &&
				!FunctionName.contains(_("OnVisibilitySetEvent")))
			{
				Logger::log.WriteToFile(_("ProcessEvent_log.txt"), FunctionName + ' ' + Object->GetFullName());
			}
		}
		
		if (FunctionName.contains(_("UAC")) || FunctionName.contains(_("SetFullscreenMode")))
			return nullptr;

		else if (FunctionName.contains(_("ServerLoadingScreenDropped")))
		{
			CreateThread(0, 0, Startup, 0, 0, 0);
			Globals::GetWorld(true);
		}
		
		else if (FunctionName.contains(_("ServerGiveCreativeItem")))
		{
			struct params {
				FFortItemEntry CreativeItem;
				FGuid ItemToRemoveGuid;
			};

			auto GiveItemParams = (params*)Params;

			if (GiveItemParams)
			{
				Logger::Log(_("ServerGiveCreativeItem called with item: ") + GiveItemParams->CreativeItem.ItemDefinition->GetFullName());

				entryToCopy = GiveItemParams->CreativeItem;
			}
		}

		else if (FunctionName.contains(_("ValidateSpawnItems"))) // No idea what this does
		{
			std::cout << _("Attempted to validate!\n");
			return nullptr;
		}

		else if (FunctionName.contains(_("CanEquip"))) // No idea what this does
		{
			std::cout << _("CanEquip called!\n");
			bool bCanEquip = true;
			return &bCanEquip;
		}

		else if (FunctionName.contains(_("CanAddToChest"))) // No idea what this does
		{
			std::cout << _("CanAddToChest called!\n");
			bool bCanAdd = true;
			return &bCanAdd;
		}

		else if (FunctionName.contains(_("ServerAddToCachedPurchased"))) // This is uh, the acutal definition of the item in the chest when u add? Idk but whenever u spawn chest it's this wid
		{
			struct params {
				UObject* ItemDefinition;
				int32_t Count;
			};

			auto AddToCachedParams = (params*)Params;
		}

		else if (FunctionName.contains(_("ServerSpawnActorWithTransform")))
		{
			struct params {
				UObject* Actor;
				FTransform Transform;
				bool bAllowOverlap;
				bool bAllowGravity;
				bool bIgnoreStructuralIssues;
				bool bForPreviewing;
			};

			auto SpawnParams = (params*)Params;
		}

		else if (FunctionName.contains(_("CheatScript")))
		{
			auto ScriptName = (*(FString*)Params).ToString();
			std::transform(ScriptName.begin(), ScriptName.end(), ScriptName.begin(), ::tolower);

			if (!ScriptName.empty())
			{
				static auto notImplemented = []() {
					Helper::Console::Say(_(L"This cheatscript hasn't been implemented yet. It will be in a future Argon Update."));
				};

				std::vector<std::string> Arguments;

				while (ScriptName.find(" ") != -1)
				{
					Arguments.push_back(ScriptName.substr(0, ScriptName.find(' ')));
					ScriptName.erase(0, ScriptName.find(' ') + 1);
				}
				
				auto NumArgs = Arguments.size() - 1;

				if (NumArgs == 0)
				{
					if (ScriptName == _("beauthority"))
					{
						Helper::ChangeRoles(Globals::GetPC(), ENetRole::ROLE_Authority);
						Helper::ChangeRoles(Globals::GetPawn(), ENetRole::ROLE_Authority);
					}

					else if (ScriptName == _("fnver") || ScriptName == _("fnversion") || ScriptName == _("fortniteversion") || ScriptName == _("fortnitever"))
					{
						Helper::Console::Say(std::wstring(FN_Version.begin(), FN_Version.end()).c_str());
					}

					else if (ScriptName == _("argonver") || ScriptName == _("argonversion"))
					{
						Helper::Console::Say(TrimString(std::to_wstring(ArgonVersion)).c_str());
					}

					else if (ScriptName == _("getroles"))
					{
						notImplemented();
					}

					else if (ScriptName == _("spawnpickup"))
					{
						notImplemented();
					}

					else if (ScriptName == _("help"))
					{
						std::wstring helpStr = _(L"BeAuthority - Set's your Role and RemoteRole for your Pawn and PlayerController to Authority\n");
						helpStr += _(L"FortniteVer - Prints Fortnite Version\n");
						helpStr += _(L"ArgonVer - Prints Argon Version.\n");
						helpStr += _(L"GetRoles - Prints Role and RemoteRole for your Pawn and PlayerController (COMING SOON).\n");
						helpStr += _(L"SpawnPickup (WID) - Spawns a pickup at your location (COMING SOON).\n");

						helpStr += _(L"Help - Displays this.\n");
						Helper::Console::Say(helpStr.c_str());
					}

					else if (ScriptName == _("testspawnscar"))
					{
						if (Globals::GetPawn(true))
						{
							static auto ScarDef = FindObject(_("FortWeaponRangedItemDefinition /Game/Athena/Items/Weapons/WID_Assault_AutoHigh_Athena_SR_Ore_T03.WID_Assault_AutoHigh_Athena_SR_Ore_T03"));
							Helper::SpawnPickup(ScarDef, 1, EFortPickupSourceTypeFlag::Other, EFortPickupSpawnSource::Unset, Globals::GetPawn(true));
						}

						else
							std::cout << _("Failed to get Pawn!\n");
					}

					else
						Helper::Console::Say(_(L"Unrecognized command."));
				}

				else if (NumArgs == 1)
				{
				}
			}
		}

		else if (FunctionName.contains(_("LoadingScreen"))) // this is cursed but it works // this gets called after "we are connecting to anotehr server"
		{
			// i would just call begindestroy and finishdestroy but they aren't ufunctions
			// Helper::CheatManager::Destroy();

		}
		
		else if (FunctionName.contains(_("GetLoadoutForPlayer"))) // idk this never gets called
		{
			Object->ProcessEvent(Function, Params);

			struct params {
				void* MemberUniqueId;
				FFortAthenaLoadout ReturnValue;
			};

			auto LoadoutParams = (params*)Params;

			static auto PID = FindObject(_("AthenaPickaxeItemDefinition /Game/Athena/Items/Cosmetics/Pickaxes/Pickaxe_ID_629_MechanicalEngineerSummerFemale.Pickaxe_ID_629_MechanicalEngineerSummerFemale"));

			auto OldPickName = (*(UObject**)((uintptr_t)&LoadoutParams->ReturnValue + 0x40))->GetFullName();
			*(UObject**)((uintptr_t)&LoadoutParams->ReturnValue + 0x40) = PID;

			Logger::Log(std::format(_("Changed Pickaxe from {} to {}"), OldPickName, PID->GetFullName()));

			return (void*)&LoadoutParams->ReturnValue; // real how processevent works
		}

		else if (FunctionName.contains(_("ServerEmote")))
		{
			struct params {
				FName AssetName;
			};
			
			auto EmoteParams = (params*)Params;

			std::cout << _("Emote: ") << EmoteParams->AssetName.ToString() << '\n';
		}

		else if (FunctionName.contains(_("ServerCreateBuildingActor")))
		{
			struct params {
				FCreateBuildingActorData data;
			};
			auto SCBA = (params*)Params;

			auto data = SCBA->data;
			
			// std::cout << _("Building: ") << data.BuildingClassData.BuildingClass->GetFullName() << '\n';
		}
	}

	return ProcessEventO(Object, Function, Params);
}

#define CURLOPT(na,t,nu) na = t + nu

#define CURLOPTTYPE_LONG          0
#define CURLOPTTYPE_OBJECTPOINT   10000
#define CURLOPTTYPE_FUNCTIONPOINT 20000
#define CURLOPTTYPE_OFF_T         30000
#define CURLOPTTYPE_BLOB          40000
#define CURLOPTTYPE_STRINGPOINT CURLOPTTYPE_OBJECTPOINT

typedef enum {
	CURLOPT(CURLOPT_URL, CURLOPTTYPE_STRINGPOINT, 2),
	CURLOPT(CURLOPT_SSL_VERIFYPEER, CURLOPTTYPE_LONG, 64),
	CURLOPT(CURLOPT_SSL_VERIFYHOST, CURLOPTTYPE_LONG, 81),
	CURLOPT(CURLOPT_NOPROXY, CURLOPTTYPE_STRINGPOINT, 177),
	CURLOPT(CURLOPT_PINNEDPUBLICKEY, CURLOPTTYPE_STRINGPOINT, 230),
} CURLoption;

typedef enum {
	CURLE_OK = 0,
} CURLcode;

typedef void CURL;

CURLcode(*curl_easy_setopt)(CURL* curl, CURLoption option, ...);

std::vector<std::string> URLs =
{
	_("/socialban/api/public/v1"), _("/affiliate/api/public/affiliates/slug"), _("/content/api/pages/fortnite-game"),
	_("/fortnite/api/game/v2/profile") //, _("/fortnite/api/v2/versioncheck/"), _("/fortnite/api/cloudstorage/system")
};

#define HOST _("http://localhost:3551") // _("https://lawinserver.milxnor.repl.co")
CURLcode curl_easy_setoptDetour(CURL* curl, CURLoption option, char* url)
{
	switch (option)
	{
	case CURLOPT_URL:
	{
		std::regex EG(_("(.*).ol.epicgames.com"));

		for (auto& URL : URLs)
		{
			if (std::regex_search(url, std::regex(URL))) {
				url = const_cast<char*>(std::regex_replace(url, EG, HOST).c_str());
				break;
			}
		}
		
		if (std::string(url).find(playlistToReplace) != std::string::npos) // && playlistToReplace != playlistToReplaceWith)
		{
			url = const_cast<char*>(std::regex_replace(url, std::regex(playlistToReplace), playlistToReplaceWith).c_str());
			std::cout << _("Changed Playlist!\n");
		}

		if (bIsS13)
		{
			if (std::string(url).find(_("WIN")) != std::string::npos)
			{
				url = const_cast<char*>(std::regex_replace(url, std::regex(_("WIN")), _("IOS")).c_str());
				std::cout << _("Changed Platform!\n");
			}
		}

		break;
	}
	case CURLOPT_SSL_VERIFYPEER:
		return curl_easy_setopt(curl, option, 0);
		break;
	case CURLOPT_SSL_VERIFYHOST:
		return curl_easy_setopt(curl, option, 0);
		break;
	case CURLOPT_NOPROXY:
		return curl_easy_setopt(curl, option, "");
		break;
	case CURLOPT_PINNEDPUBLICKEY:
		return CURLcode::CURLE_OK;
		break;
	default:
		break;
	}

	return curl_easy_setopt(curl, option, url);
}

void (__fastcall* RequestExitWithStatusOriginal)(char a1, uint8_t a2, int a3);

void __fastcall RequestExitWithStatusDetour(char a1, uint8_t a2, int a3)
{
	Logger::log.Write(_("RequestExit called!"));
	return;
}