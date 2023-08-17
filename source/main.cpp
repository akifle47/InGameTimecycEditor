#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include "TimecycEditor.h"
#include "Log.h"

std::ofstream Log::mLogFile;

typedef HRESULT(__stdcall D3D9DeviceEndSceneT)(IDirect3DDevice9*);
typedef HRESULT(__stdcall D3D9DeviceResetT)(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);
typedef HRESULT(__stdcall DInput8DeviceGetDeviceStateT)(IDirectInputDevice8*, DWORD, LPVOID);
typedef HRESULT(__stdcall DInput8DeviceAcquireT)(IDirectInputDevice8*);

void *gD3D9Device_vtbl[119] = {};
D3D9DeviceEndSceneT *D3D9DeviceEndSceneO = nullptr;
D3D9DeviceResetT    *D3D9DeviceResetO = nullptr;

void *gDinpu8Device_vtbl[32] = {};
DInput8DeviceGetDeviceStateT *DInput8DeviceGetDeviceStateO = nullptr;
DInput8DeviceAcquireT *DInput8DeviceAcquireO = nullptr;

WNDPROC WndProcO = nullptr;

TimecycEditor gTimecycEditor;

HANDLE gMainThreadHandle = nullptr;

//Function Hooks

LRESULT CALLBACK WndProcH(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(gTimecycEditor.OnWndProc(hWnd, uMsg, wParam, lParam))
		return true;
	
	return CallWindowProc(WndProcO, hWnd, uMsg, wParam, lParam);
}

HRESULT __stdcall D3D9DeviceResetH(IDirect3DDevice9 *This, D3DPRESENT_PARAMETERS *pPresentationParameters)
{
	gTimecycEditor.OnBeforeD3D9DeviceReset(This);

	HRESULT hr = D3D9DeviceResetO(This, pPresentationParameters);

	gTimecycEditor.OnAfterD3D9DeviceReset();

	return hr;
}

HRESULT APIENTRY D3D9DeviceEndSceneH(LPDIRECT3DDEVICE9 This)
{
	gTimecycEditor.OnBeforeD3D9DeviceEndScene(This);

	return D3D9DeviceEndSceneO(This);
}

//maybe not the best way to do this but it works, its simple and doesn't cause any issues with the game
HRESULT __stdcall DInput8DeviceGetDeviceStateH(IDirectInputDevice8 *This, DWORD cbData, LPVOID lpvData)
{
	HRESULT hr = DInput8DeviceGetDeviceStateO(This, cbData, lpvData);
	
	if(gTimecycEditor.mDisableMouseControl)
	{
		if(cbData == sizeof(DIMOUSESTATE) || cbData == sizeof(DIMOUSESTATE2))
		{
			This->Unacquire();
		}
	}

	return hr;
}

HRESULT __stdcall DInput8DeviceAcquireH(IDirectInputDevice8 *This)
{
	if(gTimecycEditor.mDisableMouseControl)
	{
		return DI_OK;
	}

	return DInput8DeviceAcquireO(This);
}

//Functions

bool Initialize()
{
	std::stringstream logStream;
	MH_STATUS mhStatus;
	uint8_t *baseAddress = (uint8_t*)GetModuleHandle(NULL);

	gTimecycEditor.Initialize(baseAddress);

	if(!*gD3D9Device_vtbl)
	{
		if(!Utils::GetD3D9DeviceVTable(baseAddress, gD3D9Device_vtbl))
		{
			return false;
		}
	}
	
	if(!*gDinpu8Device_vtbl)
	{
		if(!Utils::GetDInput8DeviceVTable(gDinpu8Device_vtbl))
		{
			return false;
		}
	}

	if(*gD3D9Device_vtbl)
	{
		if(!D3D9DeviceEndSceneO)
		{
			mhStatus = MH_CreateHook(gD3D9Device_vtbl[42], &D3D9DeviceEndSceneH, (void**)&D3D9DeviceEndSceneO);
			if(mhStatus != MH_OK)
			{
				logStream << "IDirect3DDevice9::EndScene hook could not be created - " << MH_StatusToString(mhStatus);
				Log::Error(logStream.str());

				return false;
			}

			Log::Info("Created IDirect3DDevice9::EndScene hook");
		}

		if(!D3D9DeviceResetO)
		{
			mhStatus = MH_CreateHook(gD3D9Device_vtbl[16], &D3D9DeviceResetH, (void**)&D3D9DeviceResetO);
			if(mhStatus != MH_OK)
			{
				logStream << "IDirect3DDevice9::Reset hook could not be created - " << MH_StatusToString(mhStatus);
				Log::Error(logStream.str());

				return false;
			}

			Log::Info("Created IDirect3DDevice9::Reset hook");
		}
	}

	if(*gDinpu8Device_vtbl)
	{
		if(!DInput8DeviceGetDeviceStateO)
		{
			mhStatus = MH_CreateHook(gDinpu8Device_vtbl[9], &DInput8DeviceGetDeviceStateH, (void**)&DInput8DeviceGetDeviceStateO);
			if(mhStatus != MH_OK)
			{
				logStream << "IDirectInputDevice8::GetDeviceState hook could not be created - " << MH_StatusToString(mhStatus);
				Log::Error(logStream.str());

				return false;
			}

			Log::Info("Created IDirectInputDevice8::GetDeviceState hook");
		}

		if(!DInput8DeviceAcquireO)
		{
			mhStatus = MH_CreateHook(gDinpu8Device_vtbl[7], &DInput8DeviceAcquireH, (void**)&DInput8DeviceAcquireO);
			if(mhStatus != MH_OK)
			{
				logStream << "IDirectInputDevice8::Acquire hook could not be created - " << MH_StatusToString(mhStatus);
				Log::Error(logStream.str());

				return false;
			}

			Log::Info("Created IDirectInputDevice8::Acquire hook");
		}
	}

	if(!WndProcO)
	{
		WndProcO = (WNDPROC)SetWindowLongPtr(FindWindow(L"grcWindow", L"GTAIV"), GWL_WNDPROC, (LONG_PTR)WndProcH);
		
		if(!WndProcO)
		{ 
			return false;
		}
	}
	
	mhStatus = MH_EnableHook(MH_ALL_HOOKS);
	if(mhStatus != MH_OK)
	{
		logStream << "Failed to enable hooks - " << MH_StatusToString(mhStatus);
		Log::Error(logStream.str());

		return false;
	}

	return true;
}

void MainLoop()
{
	static bool initialized = false;

	while(!initialized)
	{
		Sleep(300);

		initialized = Initialize();
	}

	DWORD exitCode;
	GetExitCodeThread(gMainThreadHandle, &exitCode);
	TerminateThread(gMainThreadHandle, exitCode);
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if(fdwReason == DLL_PROCESS_ATTACH)
	{
		if(!Log::Initialize())
		{
			return false;
		}

		std::stringstream logStream;
		int32_t gameVersion = 0;

		if(!Utils::GetGameVersion(gameVersion))
		{
			logStream << "In Game Timecyc Editor only supports patch 4, 7 and 8 - " << std::to_string(gameVersion);
			Log::Error(logStream.str());

			return false;
		}
		
		logStream << "Game Version: " << gameVersion;
		Log::Info(logStream.str());
		logStream.clear();

		MH_STATUS mhStatus = MH_Initialize();
		if(mhStatus != MH_OK)
		{
			logStream << "MinHook could not be initialized - " << MH_StatusToString(mhStatus);
			Log::Error(logStream.str());

			return false;
		}

		Log::Info("MinHook initialized");

		gMainThreadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)MainLoop, NULL, 0, NULL);
	}

	return true;
}