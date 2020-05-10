#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include "setting_off.h"
#include <d3dx9.h>
#include <xmmintrin.h>

#define PI 3.1415926535f

using namespace counterpose::vernets;
using namespace counterpose::signatuars;
using namespace std;

uintptr_t module_base_addr;
uintptr_t engine;
DWORD client_state;
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

// Used to read memory, function to simplify.
template<typename T> 
T read_memory_glow(SIZE_T address) {
	T buffer;
	ReadProcessMemory(process_handle, (LPCVOID)address, &buffer, sizeof(T), NULL);
	return buffer;
}

// This is used to write processes to memory, just to simplifiy
template<typename T> 
void write_memory_glow(SIZE_T address, T buffer) {
	WriteProcessMemory(process_handle, (LPVOID)address, &buffer, sizeof(buffer), NULL);
}

template <class dataType>
void write_memory_aim(dataType value, DWORD addy)
{
	WriteProcessMemory(process_handle, (PVOID)addy, &value, sizeof(dataType), 0);
}

struct Matrix3x4_t{
	float Matrix[3][4];
};

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
	return read_memory_glow<uintptr_t>(module_base_addr + dwLocalPlayer);
}

int main() {
	// Find the window named CSGO, then from that find the proces.
	window_handle = FindWindowA(NULL, "Counter-Strike: Global Offensive");
	GetWindowThreadProcessId(window_handle, &process_identification);

	// This is the DLL that has to be injected into.
	module_base_addr = GetModuleBaseAddress("client_panorama.dll");
	engine = GetModuleBaseAddress("engine.dll");
	process_handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, process_identification);
	client_state = read_memory_glow<DWORD>(engine + dwClientState);

	int flash_disable = 0;
	int ray_enabled = 0;

	// If the End key is hit the script will close.
	while (!GetKeyState(VK_END)){

		// If the Delete key is hit this will toggle the rays.
		if (GetKeyState(VK_DELETE) & 1){
			if(ray_enabled == 0){ cout << "[SUCESS] Rays are now enabled." << endl; }
			ray_enabled = 1;
			
			// Main script that starts the ray.
			uintptr_t dwGlowManager = read_memory_glow<uintptr_t>(module_base_addr + dwGlowObjectManager);
			int LocalTeam = read_memory_glow<int>(getLocalPlayer() + m_iTeamNum);

			for (int i = 1; i < 32; i++) {
				uintptr_t dwEntity = read_memory_glow<uintptr_t>(module_base_addr + dwEntityList + i * 0x10);
				int iGlowIndx = read_memory_glow<int>(dwEntity + m_iGlowIndex);
				int EnmHealth = read_memory_glow<int>(dwEntity + m_iHealth); if (EnmHealth < 1 || EnmHealth > 100) continue;
				int Dormant = read_memory_glow<int>(dwEntity + m_bDormant); if (Dormant) continue;

				int EntityTeam = read_memory_glow<int>(dwEntity + m_iTeamNum);

				if (LocalTeam == EntityTeam){
					write_memory_glow<glowStructLocal>(dwGlowManager + (iGlowIndx * 0x38) + 0x4, glowLocal);
				}
				else if (LocalTeam != EntityTeam){
					write_memory_glow<glowStructEnemy>(dwGlowManager + (iGlowIndx * 0x38) + 0x4, glowEnm);
				}
			}
		} 
		if (!GetKeyState(VK_DELETE) & 1) {
			if(ray_enabled == 1){ cout << "[SUCESS] Rays are now disabled." << endl; }
			ray_enabled = 0;
		}

		if (GetKeyState(VK_HOME) & 1){
			if(flash_disable == 0){ cout << "[SUCESS] Flashs are now disabled." << endl; }
			flash_disable = 1;

			int flash_duration;
			flash_duration = read_memory_glow<int>(getLocalPlayer() + m_flFlashDuration);
			if(flash_duration > 0){
				write_memory_glow<int>(getLocalPlayer() + m_flFlashDuration, 0);
			}
		}
		if (!GetKeyState(VK_HOME) & 1) {
			if(flash_disable == 1){ cout << "[SUCESS] Flashes are enabled again." << endl; }
			flash_disable = 0;
		}
	}
}