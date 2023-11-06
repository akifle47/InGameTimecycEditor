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

                                case 0x01000700:
                                    gameVersion = 1070;
                                    return true;
                                break;

                                case 0x01000800:
                                    gameVersion = 1080;
                                    return true;
                                break;
                                
                                case 0x0102003b:
                                    gameVersion = 1200;
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

            case 1070:
                d3d9Device = (IDirect3DDevice9*)(baseAddress + 0x148AB48);

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

            case 1200:
                d3d9Device = (IDirect3DDevice9*)(baseAddress + 0x13ED8D8);

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


    static void Float4ToU32(const float *inColor, uint32_t &outColor)
    {
        outColor = 0;
        outColor |= ((uint32_t)(inColor[0] * 255.0f + 0.5f)) << 16;
        outColor |= ((uint32_t)(inColor[1] * 255.0f + 0.5f)) << 8;
        outColor |= ((uint32_t)(inColor[2] * 255.0f + 0.5f)) << 0;
        outColor |= ((uint32_t)(inColor[3] * 255.0f + 0.5f)) << 24;
    }
    static void U32ToFloat4(const uint32_t inColor, float *outColor)
    {
        outColor[0] = ((inColor >> 16) & 0xFF) / 255.0f;
        outColor[1] = ((inColor >> 8)  & 0xFF) / 255.0f;
        outColor[2] = ((inColor >> 0)  & 0xFF) / 255.0f;
        outColor[3] = ((inColor >> 24) & 0xFF) / 255.0f;
    }

};
