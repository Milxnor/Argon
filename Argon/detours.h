#pragma once

#include "helper.h"
#include "util.h"

static bool bLogProcessEvent = false;

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
				!FunctionName.contains(_("ExecuteUbergraph_ThreatPostProcessManagerAndParticleBlueprint")))
			{
				Logger::Log(FunctionName + ' ' + Object->GetFullName());
			}
		}
		
		if (FunctionName.contains(_("UAC")))
			return nullptr;

		else if (FunctionName.contains(_("ServerLoadingScreenDropped")))
			Globals::GetWorld(true);

		else if (FunctionName.contains(_("ServerGiveCreativeItem")))
		{
			struct params {
				UObject* CreativeItem;
				FGuid ItemToRemoveGuid;
			};

			auto GiveItemParams = (params*)Params;
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
					static FString* NotImplemented = new FString(TrimString(std::wstring(_(L"This cheatscript hasn't been implemented yet. It will be in a future Argon Update."))).c_str());
					Helper::Console::Say(*NotImplemented);
					std::cout << NotImplemented->ToString() << '\n';
				};
				// std::vector<std::string> args = ScriptName.split(' ');

				if (ScriptName == _("beauthority"))
				{
					Helper::ChangeRoles(Globals::GetPC(), ENetRole::ROLE_Authority);
					Helper::ChangeRoles(Globals::GetPawn(), ENetRole::ROLE_Authority);
				}

				else if (ScriptName == _("fnver") || ScriptName == _("fnversion") || ScriptName == _("fortniteversion") || ScriptName == _("fortnitever"))
				{
					FString FnVer = std::wstring(FN_Version.begin(), FN_Version.end()).c_str();
					Helper::Console::Say(FnVer);
				}

				else if (ScriptName == _("argonver") || ScriptName == _("argonversion"))
				{
					static FString* ArgonVer = new FString(TrimString(std::to_wstring(ArgonVersion)).c_str());

					Helper::Console::Say(*ArgonVer);
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
					Helper::Console::Say(helpStr.c_str());
				}
			}
		}

		else if (FunctionName.contains(_("LoadingScreen"))) // this is cursed but it works // this gets called after "we are connecting to anotehr server"
		{
			// i would just call begindestroy and finishdestroy but they aren't ufunctions
			Helper::CheatManager::Destroy();
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
	_("/fortnite/api/game/v2/profile"), _("/fortnite/api/v2/versioncheck/"), _("/fortnite/api/cloudstorage/system")
};

#define HOST _("http://localhost:3551")

CURLcode curl_easy_setoptDetour(CURL* curl, CURLoption option, char* url)
{
	switch (option)
	{
	case CURLOPT_URL:
	{
		std::regex EG(_("(.*).ol.epicgames.com"));

		for (auto URL : URLs)
		{
			if (std::regex_search(url, std::regex(URL))) {
				url = const_cast<char*>(std::regex_replace(url, EG, HOST).c_str());
				break;
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