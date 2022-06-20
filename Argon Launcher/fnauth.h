#pragma once

#include <string>
#include "json.hpp"
#include <cpr/cpr.h>

#pragma comment(lib, "ws2_32")
#pragma comment(lib, "Crypt32.lib")

using namespace nlohmann;

namespace FortniteAuth
{
	// clientId:secret encoded in Base64
	static const std::string WindowsAuth = "ZWM2ODRiOGM2ODdmNDc5ZmFkZWEzY2IyYWQ4M2Y1YzY6ZTFmMzFjMjExZjI4NDEzMTg2MjYyZDM3YTEzZmM4NGQ=";
	static const std::string IOSAuth = "MzQ0NmNkNzI2OTRjNGE0NDg1ZDgxYjc3YWRiYjIxNDE6OTIwOWQ0YTVlMjVhNDU3ZmI5YjA3NDg5ZDMxM2I0MWEK";
	static const std::string clientId = "ec684b8c687f479fadea3cb2ad83f5c6";

	std::string GenerateAccessToken(const std::string& AuthCode)
	{
		std::string TokenUrl = "https://account-public-service-prod.ol.epicgames.com/account/api/oauth/token";
		/* json j;
		j["grant_type"] = "authorization_code";
		j["code"] = AuthCode; */

		static const auto Url = cpr::Url{ "https://account-public-service-prod.ol.epicgames.com/account/api/oauth/token" };

		cpr::AsyncResponse ar = cpr::PostAsync(Url,
			cpr::Payload{
				{ "grant_type", "authorization_code" }, 
				{ "code", AuthCode },
				// { "token_type", "eg1" }
			},
			cpr::Header{ 
				{"Authorization", "basic " + WindowsAuth},
				// {"Content-Type", "application/x-www-form-urlencoded"}
			} 
		);
		ar.wait();
		cpr::Response r = ar.get();

		const auto Response = json::parse(r.text);
		// std::cout << "Response: " << Response << '\n';

		std::string DisplayName = Response["displayName"].get<std::string>();
		std::cout << "Launching as " << DisplayName << '\n';

		std::string AccessToken = Response["access_token"].get<std::string>();
		// std::cout << "Access Token: " << AccessToken << '\n';

		return AccessToken;
	}

	std::string GenerateExchangeCode(const std::string& BearerToken)
	{
		static const auto Url = cpr::Url{ "https://account-public-service-prod.ol.epicgames.com/account/api/oauth/exchange" };

		cpr::AsyncResponse ar = cpr::GetAsync(Url,
			cpr::Payload{

			},
			cpr::Header{
				{"Authorization", "bearer " + BearerToken},
				// {"Content-Type", "application/x-www-form-urlencoded"}
			}
			);
		ar.wait();
		cpr::Response r = ar.get();
		
		return json::parse(r.text)["code"];//r.text;
		/*	x = requests.get(url, data = myobj, headers = {"Authorization": f"Bearer {bearerToken}"})
			tokentext = x.text
			texas = json.loads(tokentext)
			token = texas["code"]
			return token */

	}
}