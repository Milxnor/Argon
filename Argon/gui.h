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

#include "util.h"
#include "helper.h"

HRESULT(WINAPI* PresentOriginal)(IDXGISwapChain* SwapChain, uint32_t Interval, uint32_t Flags);

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

HWND wnd = NULL;
WNDPROC oWndProc;
ID3D11Device* pDevice = NULL;
ID3D11DeviceContext* pContext = NULL;
ID3D11RenderTargetView* mainRenderTargetView;

static bool bHasInit = false;
static bool bShow = false;

LRESULT __stdcall WndProc(const HWND hWnd, uint32_t uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_KEYUP && (wParam == VK_F8 || (bShow && wParam == VK_ESCAPE)))
	{
		bShow = !bShow;
		ImGui::GetIO().MouseDrawCursor = bShow;
	}
	
	else if (uMsg == WM_QUIT && bShow)
		ExitProcess(0);

	if (bShow)
	{
		ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
		return TRUE;
	}

	return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
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

		if (ImGui::BeginTabBar("")) { // Figure out what tab they are on
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
			
			if (ImGui::BeginTabItem(_("Credits")))
			{
				Tab = 4;
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
				std::cout << "Pawn: " << Globals::GetPawn(true)->GetFullName() << '\n';

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

			if (ImGui::Button(_("Set Head Visibility")))
			{
				Globals::GetPawn(true);

				auto fn = Globals::GetPawn()->Function(_("SetCharacterPartVisibility"));

				struct {
					EFortCustomPartType InPartType;
					bool bNewVisibility;
					bool bPropagateToChildren;
					bool ReturnValue;
				} params;

				params.InPartType = EFortCustomPartType::Head;
				params.bNewVisibility = false;
				params.bPropagateToChildren = true;
				
				Globals::GetPawn()->ProcessEvent(fn, &params);
			}
		
			break;
#endif
		case 4:
		{
			ImVec4 Color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
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