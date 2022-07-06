#pragma once

#include "structs.h"

#define DEVELOPMENT 1

static double ArgonVersion = 0.1;
static bool bIsPlainHybrid = false; // Purely redirecting.

namespace Globals
{
	static bool bIsReady = false;
	static bool bIsInGame = false;

	UObject* World;
	UObject* PC;
	UObject* Engine;
	UObject* Pawn;

	static auto GetEngine(bool bReset = false)
	{
		if (bReset || !Engine)
			Engine = FindObject(_("FortEngine_"));

		return Engine;
	}

	static auto GetPC(bool bReset = false)
	{
		if (bReset || !PC)
		{
			GetEngine();
			PC = *((*(*Engine->Member<UObject*>(_("GameInstance")))->Member<TArray<UObject*>>(_("LocalPlayers"))).At(0))->Member<UObject*>(_("PlayerController"));
		}

		return PC;
	}

	static auto GetWorld(bool bReset = false)
	{
		if (bReset || !World)
		{
			Globals::GetEngine(true);
			
			auto GameViewport = *Globals::Engine->Member<UObject*>(_("GameViewport"));

			while (!GameViewport)
			{
				GameViewport = *Globals::Engine->Member<UObject*>(_("GameViewport"));
				Sleep(1000 / 30);
			}

			World = *GameViewport->Member<UObject*>(_("World")); // we could also find the world by name but that depends on the map
		}
		
		return World;
	}

	static auto GetPawn(bool bReset = false)
	{
		if (bReset || !Pawn)
		{
			Globals::GetPC(true);

			if (Globals::GetPC())
				Pawn = *PC->Member<UObject*>(_("Pawn"));
		}

		return Pawn;
	}

}