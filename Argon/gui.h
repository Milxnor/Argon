#pragma once

// TODO: Update ImGUI

#include <Windows.h>
#include <dxgi.h>
#include <d3d11.h>

#include <Windows.h>
#include <dxgi.h>
#include <d3d11.h>

#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_win32.h>
#include <ImGui/imgui_impl_dx11.h>
#include <Kiero/kiero.h>
#include <filesystem>

#include "util.h"
#include "helper.h"
#include "commands.h"

static bool bHeadVisible = true;
static bool bBodyVisible = true;

HRESULT(WINAPI* PresentOriginal)(IDXGISwapChain* SwapChain, uint32_t Interval, uint32_t Flags);

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace fs = std::filesystem;

HWND wnd = NULL;
WNDPROC oWndProc;
ID3D11Device* pDevice = NULL;
ID3D11DeviceContext* pContext = NULL;
ID3D11RenderTargetView* mainRenderTargetView;

static bool bHasInit = false;
static bool bShow = false;

LRESULT __stdcall WndProc(const HWND hWnd, uint32_t message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_KEYUP:
		if (wParam == VK_F8 || (bShow && wParam == VK_ESCAPE))
		{
			bShow = !bShow;
			ImGui::GetIO().MouseDrawCursor = bShow;
		}
		break;
	case WM_SIZE:
		if (pDevice && wParam != SIZE_MINIMIZED)
		{
			
		}
		break;
	case WM_QUIT:
		if (bShow)
			ExitProcess(0);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
	}

	if (bShow)
	{
		ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam);
		return TRUE;
	}

	return CallWindowProc(oWndProc, hWnd, message, wParam, lParam);
}

HRESULT WINAPI HookPresent(IDXGISwapChain* SwapChain, uint32_t Interval, uint32_t Flags)
{
	if (!bHasInit)
	{
		auto stat = SUCCEEDED(SwapChain->GetDevice(__uuidof(ID3D11Device), (PVOID*)&pDevice));
		if (stat)
		{
			pDevice->GetImmediateContext(&pContext);
			DXGI_SWAP_CHAIN_DESC sd;
			SwapChain->GetDesc(&sd);
			wnd = sd.OutputWindow;
			ID3D11Texture2D* pBackBuffer;
			SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
			pDevice->CreateRenderTargetView(pBackBuffer, NULL, &mainRenderTargetView);
			pBackBuffer->Release();
			oWndProc = (WNDPROC)SetWindowLongPtr(wnd, GWLP_WNDPROC, (LONG_PTR)WndProc);
			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO();
			io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;

			ImGui_ImplWin32_Init(wnd);
			ImGui_ImplDX11_Init(pDevice, pContext);

			bHasInit = true;
		}

		else return PresentOriginal(SwapChain, Interval, Flags);
	}

	if (bShow) // TODO: Acutaly learn ImGUI and rewrite
	{
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();

		ImGui::NewFrame();
		ImGui::SetNextWindowBgAlpha(0.8f);
		ImGui::SetNextWindowSize(ImVec2(560, 345));

		ImGui::Begin(_("Argon"), 0, ImGuiWindowFlags_NoCollapse);

		// initialize variables used by the user using the gui

		static int Tab = 1;
		static float currentFOV = 80;
		static float FOV = 80;
		static char WID[60] = {};
		static bool bConsoleIsOpen = false;
		static char headPath[MAX_PATH] = "";
		static char bodyPath[MAX_PATH] = "";

		if (ImGui::BeginTabBar("")) { // Figure out what tab they are on // Creative Bar??
			if (ImGui::BeginTabItem(_("Player")))
			{
				Tab = 1;
				ImGui::EndTabItem();
			}
			
			if (ImGui::BeginTabItem(_("Fun")))
			{
				Tab = 2;
				ImGui::EndTabItem();
			}
			
#if DEVELOPMENT == 1
			if (ImGui::BeginTabItem(_("Developer")))
			{
				Tab = 3;
				ImGui::EndTabItem();
			}
#endif

			if (ImGui::BeginTabItem(_("Skin")))
			{
				Tab = 4;
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem(_("Other")))
			{
				Tab = 5;
				ImGui::EndTabItem();
			}
			
			if (ImGui::BeginTabItem(_("Actors")))
			{
				Tab = 6;
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem(_("Console")))
			{
				Tab = 7;
				ImGui::EndTabItem();
			}
			
			if (ImGui::BeginTabItem(_("Credits")))
			{
				Tab = 8;
				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}

		switch (Tab) // now that we know what tab we can now display what that tab has
		{
		case 1:
			ImGui::SliderFloat(_("FOV"), &FOV, 0, 300);

			if (currentFOV != FOV)
			{
				currentFOV = FOV;
				static auto fn = Globals::GetPC()->Function(_("FOV"));

				struct {
					float fov;
				} params{};

				params.fov = FOV;

				Globals::GetPC()->ProcessEvent(fn, &params);
			}

			if (ImGui::Button(_("Be Authority")))
			{
				Helper::ChangeRoles(Globals::GetPC(), ENetRole::ROLE_Authority);
				Helper::ChangeRoles(Globals::GetPawn(), ENetRole::ROLE_Authority);
			}

			break;
		case 2:
			break;
#if DEVELOPMENT == 1
		case 3:
			if (ImGui::Button(_("Dump Objects")))
				CreateThread(0, 0, DumpObjects, 0, 0, 0);

			if (ImGui::Button(_("Change Head")))
			{
				std::cout << _("Pawn: ") << Globals::GetPawn(true)->GetFullName() << '\n';

				auto head = FindObject(_("CustomCharacterPart /Game/Athena/Heroes/Meshes/Heads/Dev_TestAsset_Head_M_XL.Dev_TestAsset_Head_M_XL"));

				if (!head)
					std::cout << _("Could not find head!\n");

				else
				{
					UObject*** CharacterParts = Globals::GetPawn()->Member<UObject**>(_("CharacterParts"));
					
					if (!CharacterParts)
						std::cout << _("Could not find CharacterParts!\n");
					
					else if (!*CharacterParts) // characterparts = nullptr for some reason
						std::cout << _("Could not find CharacterPartsArray!\n");

					else if (!**CharacterParts)
						std::cout << _("Could not find first CharacterPart!\n");

					else
					{
						Logger::Log(_("Original HeadPart: ") + (*CharacterParts)[1]->GetFullName());

						(*CharacterParts)[1] = head;

						static auto fn = Globals::GetPawn()->Function(_("ApplyCosmeticLoadout"));

						if (fn)
							Globals::GetPawn()->ProcessEvent(fn, nullptr);

						else
							std::cout << _("Unable to find ApplyCosmeticLoadout!\n");
					}
				}
			}

			if (ImGui::Button(_("Die (doesn't work)")))
			{
				// void ServerSuicide(bool bSuppressResurrectionChip, int32_t MatchPlacement); // Function FortniteGame.FortPlayerPawnAthena.ServerSuicide

				if (Globals::GetPawn(true))
				{
					static auto fn = Globals::GetPawn(true)->Function(_("ServerSuicide"));

					if (fn)
					{
						struct {
							bool bSuppressResurrectionChip;
							int32_t MatchPlacement;
						} params{};

						params.bSuppressResurrectionChip = false;
						params.MatchPlacement = 1;

						Globals::GetPawn()->ProcessEvent(fn, &params);
					}

					else
						std::cout << _("Unable to find ServerSuicide function\n");
				}
				else
					std::cout << _("Unable to find Pawn!\n");
			}
		
			break;
#endif
		case 4:
			ImGui::InputText(_("Head Png Texture Path"), headPath, IM_ARRAYSIZE(headPath));
			ImGui::InputText(_("Body Png Texture Path"), bodyPath, IM_ARRAYSIZE(bodyPath));

			if (ImGui::Button(_("Apply Head Texture")))
			{
				auto Pawn = Globals::GetPawn(true);

				if (Pawn)
				{
					std::string HeadPath = headPath;
					if (!HeadPath.empty())
					{
						if (fs::exists(HeadPath))
						{
							if (HeadPath.contains(_(".png")))
							{
								FString h = _(L"C:\\Users\\Stowe\\Downloads\\fortnite.png");
								Helper::SetPartTextureFromPng(EFortCustomPartType::Head, h);
							}
							else
								std::cout << _("Path is not a png file!\n");
						}
						else
							std::cout << _("Path does not exist!\n");
					}
					else
						std::cout << _("Path is empty!\n");
				}
			}

			if (ImGui::Button(_("Apply Body Texture")))
			{
				auto Pawn = Globals::GetPawn(true);

				if (Pawn)
				{
					std::string BodyPath = bodyPath;
					if (!BodyPath.empty())
					{
						if (fs::exists(BodyPath))
						{
							if (BodyPath.contains(_(".png")))
							{
								FString b = _(L"C:\\Users\\Stowe\\Downloads\\fortnite.png");
								Helper::SetPartTextureFromPng(EFortCustomPartType::Body, b);
							}
							else
								std::cout << _("Path is not a png file!\n");
						}
						else
							std::cout << _("Path does not exist!\n");
					}
					else
						std::cout << _("Path is empty!\n");
				}
			}

			if (ImGui::Button(_("Set Head Visibility")))
			{
				bHeadVisible = !bHeadVisible;

				if (!Helper::SetCharacterPartVisibility(EFortCustomPartType::Head, bHeadVisible))
					std::cout << _("Failed to set head visibility!\n");

				if (bBodyVisible)
					std::cout << _("Head is now visible\n");
				else
					std::cout << _("Head is now invisible\n");
			}

			if (ImGui::Button(_("Set Body Visibility")))
			{
				bBodyVisible = !bBodyVisible;
				
				if (!Helper::SetCharacterPartVisibility(EFortCustomPartType::Body, bBodyVisible))
					std::cout << _("Failed to set body visibility!\n");
				
				if (bBodyVisible)
					std::cout << _("Body is now visible\n");
				else
					std::cout << _("Body is now invisible\n");
			}
			
			if (ImGui::Button(_("Change PID")))
			{
				Globals::GetPC(true);

				if (Globals::GetPC())
				{
					auto PCLoadout = Globals::GetPC()->Member<FFortAthenaLoadout>(_("CosmeticLoadoutPC"));
					static auto PID = FindObject(_("AthenaPickaxeItemDefinition /Game/Athena/Items/Cosmetics/Pickaxes/Pickaxe_ID_629_MechanicalEngineerSummerFemale.Pickaxe_ID_629_MechanicalEngineerSummerFemale"));

					if (PCLoadout)
					{
						if (PID)
						{
							Logger::Log(_("PC PID: ") + (*(UObject**)((uintptr_t)PCLoadout + 0x40))->GetFullName());
							*(UObject**)((uintptr_t)PCLoadout + 0x40) = PID;

							Globals::GetPawn(true);

							if (Globals::GetPawn())
							{
								auto PawnBaseLoadout = Globals::GetPawn()->Member<FFortAthenaLoadout>(_("BaseCosmeticLoadout"));
								auto PawnAppliedLoadout = Globals::GetPawn()->Member<FFortAthenaLoadout>(_("AppliedCosmeticLoadout"));
								auto PawnCosmeticLoadout = Globals::GetPawn()->Member<FFortAthenaLoadout>(_("CosmeticLoadout"));

								if (PawnBaseLoadout)
								{
									Logger::Log(_("BaseLoadout PID: ") + (*(UObject**)((uintptr_t)PawnBaseLoadout + 0x40))->GetFullName());
									*(UObject**)((uintptr_t)PawnBaseLoadout + 0x40) = PID;
								}

								if (PawnAppliedLoadout)
								{
									Logger::Log(_("AppliedLoadout PID: ") + (*(UObject**)((uintptr_t)PawnAppliedLoadout + 0x40))->GetFullName());
									*(UObject**)((uintptr_t)PawnAppliedLoadout + 0x40) = PID;
								}

								if (PawnCosmeticLoadout)
								{
									Logger::Log(_("PawnCosmeticLoadout PID: ") + (*(UObject**)((uintptr_t)PawnCosmeticLoadout + 0x40))->GetFullName());
									*(UObject**)((uintptr_t)PawnCosmeticLoadout + 0x40) = PID;
								}

								static auto fn = Globals::GetPawn()->Function(_("ApplyCosmeticLoadout"));

								if (fn)
									Globals::GetPawn()->ProcessEvent(fn, nullptr);

								else
									std::cout << _("Unable to find ApplyCosmeticLoadout!\n");

								static auto fn2 = Globals::GetPawn()->Function(_("OnRep_BaseCosmeticLoadout"));

								if (fn2)
								{
									Globals::GetPawn()->ProcessEvent(fn2, nullptr);
									Logger::Log(_("Called OnRep_BaseCosmeticLoadout\n"));
								}
								
								else
									std::cout << _("Unable to find OnRep_BaseCosmeticLoadout!\n");
							}
							else
								std::cout << _("No Pawn was found!\n");
						}
						else
							std::cout << _("Failed to find PID!\n");
					}
					else
						std::cout << _("Unable to find PlayerController's loadout!\n");
				}

				std::cout << "PID(s) changed!\n";
			}

			if (ImGui::Button(_("Request Refresh Loadout (DONT USE IN LOBBY)"))) // TODO: Add a check to see if they are in lobby, if they are don't show them this.
			{
				Globals::GetPC(true);
				
				if (Globals::GetPC())
				{
					static auto fn = Globals::GetPC()->Function(_("ServerRequestLoadoutRefresh"));
					bool bForceRefresh = true;

					if (fn)
					{
						Globals::GetPC()->ProcessEvent(fn, &bForceRefresh);
						Logger::Log("Requested Refresh!");
					}
					else
						std::cout << _("Unable to find ServerRequestLoadoutRefresh!\n");
				}
			}


			if (ImGui::Button(_("Apply Custom Parts")))
			{				
				if (Globals::GetPawn(true))
				{
					static auto HeadPart = FindObject(_("CustomCharacterPart /Game/Characters/CharacterParts/Male/Medium/Heads/CP_Athena_Head_M_AshtonMilo.CP_Athena_Head_M_AshtonMilo"));
					static auto BodyPart = FindObject(_("CustomCharacterPart /Game/Athena/Heroes/Meshes/Bodies/CP_Athena_Body_M_AshtonMilo.CP_Athena_Body_M_AshtonMilo"));
					
					Logger::Log(_("ApplyCharacterCosmetics: ") + Helper::Kismet::ApplyCharacterCosmetics(Globals::GetPawn(), BodyPart, HeadPart));
				}
			}

			if (ImGui::Button(_("Update CharacterParts")))
			{
				if (Globals::GetPawn(true))
				{
					static auto fn = Helper::Kismet::GetFortKismet()->Function(_("UpdatePlayerCustomCharacterPartsVisualization"));

					struct {
						UObject* PlayerState;
					} params;

					params.PlayerState = *Globals::GetPawn()->Member<UObject*>(_("PlayerState"));

					Helper::Kismet::GetFortKismet()->ProcessEvent(fn, &params);

					Logger::Log(_("Updated CharacterParts!"));
				}
			}
			
			if (ImGui::Button(_("Set Local Parts")))
			{
				if (Globals::GetPawn(true))
				{
					auto PlayerState = Globals::GetPawn()->Member<UObject*>(_("PlayerState"));

					if (PlayerState && *PlayerState)
					{
						// 	struct UCustomCharacterPart* LocalCharacterParts[0x7]; // 0x598(0x38)
						UObject*** LocalCharacterParts = (*PlayerState)->Member<UObject**>(_("LocalCharacterParts"));

						if (LocalCharacterParts)
						{
							if (*LocalCharacterParts) // the acutal array
							{
								if (**LocalCharacterParts) // the first of the array
								{
									std::cout << "t\n";
									std::cout << "First part: " << (**LocalCharacterParts)->GetFullName() << '\n';
								}
								else
									std::cout << _("LocalCharacterParts array is empty!");
							}

							else
								std::cout << _("LocalCharacterParts is null!");

						}

						else
							std::cout << _("Unable to find LocalCharacterParts!");
					}

					else
						std::cout << _("Could not find PlayerState");
				}
			}
			break;
		case 5:
			if (ImGui::Button(_("Log ProcessEvent")))
			{
				bLogProcessEvent = !bLogProcessEvent;

				if (bLogProcessEvent)
					Logger::Log(_("Started Logging ProcessEvent!"));

				else
					Logger::Log(_("Stopped logging ProcessEvent!"));
			}

			if (ImGui::Button(_("Give Creative Item")))
			{
				Helper::Creative::GiveItem(FindObject(_("FortWeaponRangedItemDefinition /Game/Athena/Items/Weapons/WaffleTruck/WID_WaffleTruck_Sniper_StormScout.WID_WaffleTruck_Sniper_StormScout")));
				Logger::Log(_("Gave Storm Scout!"));
			}
			
			if (ImGui::Button(_("Give Saved Item")))
			{
				Helper::Creative::GiveItem(entryToCopy);
				Logger::Log(_("Gave Saved Item!"));
			}

			if (ImGui::Button(_("Print Roles")))
			{
				if (Globals::GetPawn(true))
				{
					Logger::Log(std::format(_("Pawn => LocalRole: {} RemoteRole: {}"), std::to_string((*Globals::GetPawn()->Member<TEnumAsByte<ENetRole>>(_("Role"))).Get()),
						std::to_string((*Globals::GetPawn()->Member<TEnumAsByte<ENetRole>>(_("RemoteRole"))).Get())));
				}

				if (Globals::GetPC())
				{
					Logger::Log(std::format(_("PC => LocalRole: {} RemoteRole: {}"), std::to_string((*Globals::GetPC()->Member<TEnumAsByte<ENetRole>>(_("Role"))).Get()),
						std::to_string((*Globals::GetPC()->Member<TEnumAsByte<ENetRole>>(_("RemoteRole"))).Get())));
				}
			}

			break;
		case 6:
			if (ImGui::Button(_("Test spawn building actor")))
			{
				Helper::CreateBuildingActor(FindObject(_("Class /Script/FortniteGame.BuildingFoundationSlab")), 1);
				std::cout << _("Done!\n");
			}
			break;
		case 7:
			ShowConsole(&bConsoleIsOpen);
			break;
		case 8:
		{
			static ImVec4 Color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
			ImGui::TextColored(Color, _("Credits:\n\nMilxnor - Everything"));
			break;
		}
		}

		ImGui::End();
		ImGui::Render();

		pContext->OMSetRenderTargets(1, &mainRenderTargetView, NULL);
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	}
	return PresentOriginal(SwapChain, Interval, Flags);
}

DWORD WINAPI GuiHook(LPVOID)
{
	bool bHooked = false;
	while (!bHooked)
	{
		auto status = kiero::init(kiero::RenderType::D3D11); // Don't do auto because it tries DX9 since that's the first direct thingy it finds
		if (status == kiero::Status::Success)
		{
			kiero::bind(8, (PVOID*)&PresentOriginal, HookPresent);
			bHooked = true;
		}

		Sleep(100);
	}

	Logger::Log(_("Initialized GUI!"));

	return 0;
}