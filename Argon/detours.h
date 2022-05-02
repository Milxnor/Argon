#pragma once

#include "structs.h"
#include "util.h"

void* ProcessEventDetour(UObject* Object, UObject* Function, void* Params)
{
	if (Object && Function)
	{
		auto FunctionName = Function->GetFullName();
		
		if (FunctionName.contains(_("UAC")))
			return nullptr;
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