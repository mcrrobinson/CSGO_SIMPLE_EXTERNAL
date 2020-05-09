#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include "csgo.h"

using namespace counterpose::vernets;
using namespace counterpose::signatuars;

uintptr_t module_base_addr;
DWORD process_identification;
HWND window_handle;
HANDLE process_handle;

uintptr_t GetModuleBaseAddress(const char* modName) {
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, process_identification);
	if (hSnap != INVALID_HANDLE_VALUE) {
		MODULEENTRY32 modEntry;
		modEntry.dwSize = sizeof(modEntry);
		if (Module32First(hSnap, &modEntry)) {
			do {
				if (!strcmp(modEntry.szModule, modName)) {
					CloseHandle(hSnap);
					return (uintptr_t)modEntry.modBaseAddr;
				}
			} while (Module32Next(hSnap, &modEntry));
		}
	}
}

template<typename T> T RPM(SIZE_T address) {
	T buffer;
	ReadProcessMemory(process_handle, (LPCVOID)address, &buffer, sizeof(T), NULL);
	return buffer;
}

template<typename T> void WPM(SIZE_T address, T buffer) {
	WriteProcessMemory(process_handle, (LPVOID)address, &buffer, sizeof(buffer), NULL);
}

struct glowStructEnemy {
	float red = 1.f;
	float green = 0.f;
	float blue = 0.f;
	float alpha = 1.f;
	uint8_t padding[8];
	float unknown = 1.f;
	uint8_t padding2[4];
	BYTE renderOccluded = true;
	BYTE renderUnoccluded = false;
	BYTE fullBloom = false;
}glowEnm;

struct glowStructLocal {
	float red = 0.f;
	float green = 1.f;
	float blue = 0.f;
	float alpha = 1.f;
	uint8_t padding[8];
	float unknown = 1.f;
	uint8_t padding2[4];
	BYTE renderOccluded = true;
	BYTE renderUnoccluded = false;
	BYTE fullBloom = false;
}glowLocal;

uintptr_t getLocalPlayer() {
	return RPM<uintptr_t>(module_base_addr + dwLocalPlayer);
}

int main() {
	// Find the window named CSGO, then from that find the proces.
	window_handle = FindWindowA(NULL, "Counter-Strike: Global Offensive");
	GetWindowThreadProcessId(window_handle, &process_identification);

	// This is the DLL that has to be injected into.
	module_base_addr = GetModuleBaseAddress("client_panorama.dll");
	process_handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, process_identification);


	// If the End key is hit the script will close.
	while (!GetAsyncKeyState(VK_END)){

		// If the Delete key is hit this will toggle the wallstuff.
		if (GetKeyState(VK_DELETE) & 1){
			
			uintptr_t dwGlowManager = RPM<uintptr_t>(module_base_addr + dwGlowObjectManager);
			int LocalTeam = RPM<int>(getLocalPlayer() + m_iTeamNum);

			for (int i = 1; i < 32; i++) {
				uintptr_t dwEntity = RPM<uintptr_t>(module_base_addr + dwEntityList + i * 0x10);
				int iGlowIndx = RPM<int>(dwEntity + m_iGlowIndex);
				int EnmHealth = RPM<int>(dwEntity + m_iHealth); if (EnmHealth < 1 || EnmHealth > 100) continue;
				int Dormant = RPM<int>(dwEntity + m_bDormant); if (Dormant) continue;

				int EntityTeam = RPM<int>(dwEntity + m_iTeamNum);

				if (LocalTeam == EntityTeam)
				{
					WPM<glowStructLocal>(dwGlowManager + (iGlowIndx * 0x38) + 0x4, glowLocal);
				}
				else if (LocalTeam != EntityTeam)
				{
					WPM<glowStructEnemy>(dwGlowManager + (iGlowIndx * 0x38) + 0x4, glowEnm);
				}
			}
		}
	}
}