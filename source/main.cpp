#define DIRECTINPUT_VERSION 0x0800

#include "TimecycEditor.h"
#include "Common.h"
#include "injector/injector.hpp"

#include <d3d9.h>
#include <dinput.h>

TimecycEditor gTimecycEditor;

WNDPROC WndProcO = nullptr;
LRESULT CALLBACK WndProcH(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if(gTimecycEditor.OnWndProc(hWnd, uMsg, wParam, lParam))
        return true;
    
    return CallWindowProc(WndProcO, hWnd, uMsg, wParam, lParam);
}

HRESULT(__stdcall* D3D9DeviceResetO)(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*) = nullptr;
HRESULT __stdcall D3D9DeviceResetH(IDirect3DDevice9* This, D3DPRESENT_PARAMETERS* pPresentationParameters)
{
    gTimecycEditor.OnBeforeD3D9DeviceReset(This);

    HRESULT hr = D3D9DeviceResetO(This, pPresentationParameters);

    gTimecycEditor.OnAfterD3D9DeviceReset();

    return hr;
}

HRESULT(__stdcall* D3D9DeviceEndSceneO)(IDirect3DDevice9*) = nullptr;
HRESULT __stdcall D3D9DeviceEndSceneH(LPDIRECT3DDEVICE9 This)
{
    gTimecycEditor.OnBeforeD3D9DeviceEndScene(This);

    return D3D9DeviceEndSceneO(This);
}

HRESULT(__stdcall* DInput8DeviceGetDeviceStateO)(IDirectInputDevice8*, DWORD, LPVOID) = nullptr;
HRESULT __stdcall DInput8DeviceGetDeviceStateH(IDirectInputDevice8 *This, DWORD cbData, LPVOID lpvData)
{
    HRESULT hr = DInput8DeviceGetDeviceStateO(This, cbData, lpvData);
    
    if(gTimecycEditor.DisablePlayerControls())
    {
        if(cbData == sizeof(DIMOUSESTATE) || cbData == sizeof(DIMOUSESTATE2))
        {
            This->Unacquire();
        }
        else if(cbData == 256 && ((uint8_t*)lpvData)[DIK_LCONTROL] != 0)
        {
            uint8_t* key = &((uint8_t*)lpvData)[DIK_LCONTROL];
            *key = 0;
        }
    }

    return hr;
}

HRESULT(__stdcall* DInput8DeviceAcquireO)(IDirectInputDevice8*) = nullptr;
HRESULT __stdcall DInput8DeviceAcquireH(IDirectInputDevice8 *This)
{
    if(gTimecycEditor.DisablePlayerControls())
    {
        return DI_OK;
    }

    return DInput8DeviceAcquireO(This);
}

HWND(__cdecl* grcDevice__CreateDeviceWindowO)() = nullptr;
HWND grcDevice__CreateDeviceWindowH()
{
    HWND hwnd = grcDevice__CreateDeviceWindowO();

    auto pattern = FindPattern({"C7 05 ? ? ? ? ? ? ? ? E8 ? ? ? ? 8B 0D ? ? ? ? 8B 51",  "C7 05 ? ? ? ? ? ? ? ? E8 ? ? ? ? A1 ? ? ? ? 68 ? ? ? ? ? ? 6A"});
    if(pattern.empty())
    {
        return hwnd;
    }

    uint32_t** ppvft = *(uint32_t***)pattern.get_first(6);
    if(!IsReadable(ppvft, 4) || !IsReadable(*ppvft, 43 * 4))
    {
        return hwnd;
    }

    uint32_t* d3d9_vft = *ppvft;

    // This can run twice. A second install would read our own hook back out
    // as the original and make it call itself.
    static uint32_t* installedOn = nullptr;
    if(installedOn)
    {
        return hwnd;
    }
    installedOn = d3d9_vft;

    D3D9DeviceEndSceneO = (decltype(D3D9DeviceEndSceneO))d3d9_vft[42];
    PatchDword(&d3d9_vft[42], (uint32_t)D3D9DeviceEndSceneH);

    D3D9DeviceResetO = (decltype(D3D9DeviceResetO))d3d9_vft[16];
    PatchDword(&d3d9_vft[16], (uint32_t)D3D9DeviceResetH);

    WndProcO = (WNDPROC)SetWindowLongPtr(hwnd, GWL_WNDPROC, (LONG_PTR)WndProcH);

    return hwnd;
}

// We need to update the timecyc editor here instead of "D3D9DeviceEndSceneH" because EndScene is called from the render thread, and the timecyc editor needs to be updated from the main thread.
void(__cdecl* CSystem__AddDrawList_EndRenderO)() = nullptr;
void CSystem__AddDrawList_EndRenderH()
{
    if(!gEditorDead)
        gTimecycEditor.Update();
    CSystem__AddDrawList_EndRenderO();
}

void CatastrophicError(const wchar_t* msg)
{
    MessageBox(0, msg, L"In Game TimeCycle Editor Error", MB_OK | MB_ICONERROR);

    #ifdef _DEBUG
        __debugbreak();
    #endif // DEBUG
}

static bool InitBody()
{
    gTimecycEditor.Initialize();
    if(gEditorDead)
    {
        return false;
    }

    // DirectInput hooks

    IDirectInput8W* dinput8 = nullptr;
    if(FAILED(DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8W, (void**)&dinput8, 0)))
    {
        CatastrophicError(L"Failed to create a DirectInput interface.");
        return false;
    }

    IDirectInputDevice8W* dinput8Device = nullptr;
    if(dinput8->CreateDevice(GUID_SysMouse, &dinput8Device, NULL) != DI_OK)
    {
        CatastrophicError(L"Failed to create a DirectInput device.");
        return false;
    }

    uint32_t* dinput8Device_vft = *(uint32_t**)dinput8Device;

    // Recorded so a failed init can restore them.
    DInput8DeviceGetDeviceStateO = (decltype(DInput8DeviceGetDeviceStateO))dinput8Device_vft[9];
    PatchDword(&dinput8Device_vft[9], (uint32_t)DInput8DeviceGetDeviceStateH);

    DInput8DeviceAcquireO = (decltype(DInput8DeviceAcquireO))dinput8Device_vft[7];
    PatchDword(&dinput8Device_vft[7], (uint32_t)DInput8DeviceAcquireH);

    dinput8->Release();
    dinput8Device->Release();

    // wom

    auto pattern = FindPattern({"E8 ? ? ? ? A3 ? ? ? ? A1 ? ? ? ? C6 05",  "E8 ? ? ? ? A3 ? ? ? ? C6 05 ? ? ? ? ? A1",  "E8 ? ? ? ? 8B 0D ? ? ? ? A3 ? ? ? ? C6 05"});
    if(pattern.empty()) return false;
    grcDevice__CreateDeviceWindowO = (decltype(grcDevice__CreateDeviceWindowO))PatchCall(pattern.get_first(0), grcDevice__CreateDeviceWindowH);

    // other

    pattern = FindPattern({"E8 ? ? ? ? B9 ? ? ? ? E8 ? ? ? ? B9 ? ? ? ? E8 ? ? ? ? ? ? ? 76 ? FF 15", "E8 ? ? ? ? B9 ? ? ? ? E8 ? ? ? ? B9 ? ? ? ? E8 ? ? ? ? ? ? ? 76"});
    if(pattern.empty()) return false;
    CSystem__AddDrawList_EndRenderO = (decltype(CSystem__AddDrawList_EndRenderO))PatchCall(pattern.get_first(0), CSystem__AddDrawList_EndRenderH);

    return true;
}

static DWORD InitCrashFilter(EXCEPTION_POINTERS*)
{
    return EXCEPTION_EXECUTE_HANDLER;
}

bool Initialize()
{
    // Whatever happens here the game survives: an exception is caught rather
    // than escaping into the loader, every patch is rolled back, and the
    // module stays resident but inert.
    bool ok = false;
    __try
    {
        ok = InitBody();
    }
    __except(InitCrashFilter(GetExceptionInformation()))
    {
        ok = false;
    }

    if(!ok)
    {
        gEditorDead = true;
        RollbackAllPatches();
    }
    return true;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    if(fdwReason == DLL_PROCESS_ATTACH)
    {
        return Initialize();
    }

    return true;
}