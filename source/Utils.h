#pragma once

#define DIRECTINPUT_VERSION 0x0800

#include <d3d9.h>
#include <dinput.h>
#include <MinHook.h>
#include <cstdint>

namespace Utils
{
    //returns true if its a supported version and false if not
    static bool GetGameVersion(int32_t &gameVersion)
    {
        DWORD fileInfoSize = GetFileVersionInfoSize(L"GTAIV.exe", NULL);

        if(fileInfoSize)
        {
            char *versionInfo = new char[fileInfoSize];

            if(GetFileVersionInfo(L"GTAIV.exe", NULL, fileInfoSize, versionInfo))
            {
                UINT versionSize = 0;
                LPBYTE versionData = NULL;

                if(VerQueryValue(versionInfo, L"\\", (void**)&versionData, &versionSize))
                {
                    if(versionSize)
                    {
                        VS_FIXEDFILEINFO *verInfo = (VS_FIXEDFILEINFO*)versionData;
                        if(verInfo->dwSignature == 0xFEEF04BD)
                        {
                            int32_t version = 0;
                            version |= ((verInfo->dwFileVersionMS >> 16) & 0xffff) << 24;
                            version |= ((verInfo->dwFileVersionMS >>  0) & 0xffff) << 16;
                            version |= ((verInfo->dwFileVersionLS >> 16) & 0xffff) << 8;
                            version |= ((verInfo->dwFileVersionLS >>  0) & 0xffff);

                            switch(version)
                            {
                                case 0x01000400:
                                    gameVersion = 1040;
                                    return true;
                                break;

                                case 0x01000800:
                                    gameVersion = 1080;
                                    return true;
                                break;

                                default:
                                    gameVersion = version;
                                    return false;
                                break;
                            }
                        }
                    }
                }
            }

            delete[] versionInfo;
        }

        return false;
    }

    static bool GetDInput8DeviceVTable(void **vtable)
    {
        if(!vtable)
            return false;

        IDirectInput8W *dinput8 = nullptr;
        if(DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8W, (void **)&dinput8, 0) != DI_OK)
        {
            return false;
        }

        IDirectInputDevice8W *dummyDevice = nullptr;
        if(dinput8->CreateDevice(GUID_SysMouse, &dummyDevice, NULL) != DI_OK)
        {
            return false;
        }

        memcpy(vtable, *(void***)dummyDevice, 32 * 4);

        dinput8->Release();
        dummyDevice->Release();

        return true;
    }

    //gets the RageDirect3DDevice9 vtable instead of using the dummy device method because it crashes the game with dxvk
    static bool GetD3D9DeviceVTable(const uint8_t *baseAddress, void **vtable)
    {
        IDirect3DDevice9 *d3d9Device = nullptr;
        int32_t gameVersion;
        Utils::GetGameVersion(gameVersion);

        switch(gameVersion)
        {
            case 1040:
                d3d9Device = (IDirect3DDevice9*)(baseAddress + 0x11D6E50);

                if(*(int32_t*)d3d9Device == NULL)
                {
                    return false;
                }

                memcpy(vtable, **(void***)d3d9Device, 119 * 4);

                return true;
            break;

            case 1080:
                d3d9Device = (IDirect3DDevice9*)(baseAddress + 0x1345630);

                if(*(int32_t*)d3d9Device == NULL)
                {
                    return false;
                }

                memcpy(vtable, **(void***)d3d9Device, 119 * 4);

                return true;
            break;
        }

        return false;
    }
};
