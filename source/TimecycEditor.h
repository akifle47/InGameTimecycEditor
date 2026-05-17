#pragma once

#define WIN32_LEAN_AND_MEAN

#include "TimeCycle.h"
#include "imgui_dx9_backend/imgui_impl_dx9_shader.h"

#include <tuple>
#include <string>
#include <array>
#include <Windows.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

class TimecycEditor
{
public:
    void Initialize();

    bool OnWndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    void Update();

    void OnBeforeD3D9DeviceReset(IDirect3DDevice9 *d3d9Device);
    void OnAfterD3D9DeviceReset();

    void OnBeforeD3D9DeviceEndScene(IDirect3DDevice9 *d3d9Device);

    bool DisablePlayerControls() { return mDisableMouseControl; };

private:
    void InitializeImGui(IDirect3DDevice9 *d3d9Device);

    void SaveSettings();
    void LoadSettings();

    void ReSetFloatColors();
    void ReCreateFont();

    void DrawMainWindow();
    void DrawSaveWindow();
    void DrawLoadWindow();
    void DrawSettingsWindow();
    void DrawSetParamOnAllHoursAndWeathersWindow();
    void DrawLicensesWindow();

    int32_t TimecycTimeIndexToGameTime(const int32_t timeIndex);
    int32_t GameTimeToTimecycTimeIndex(const int32_t gameTime);

    int32_t (__cdecl* ForceWeather)(uint32_t) = nullptr;
    void (__cdecl *ReleaseWeather)() = nullptr;
    int32_t(__cdecl* SetTimeOneDayForward)() = nullptr;
    int32_t(__cdecl* SetTimeOneDayBack)() = nullptr;

    bool mDisableMouseControl = false;

    int32_t *mHour = nullptr;
    int32_t *mMinutes = nullptr;
    uint32_t *mTimerLength = nullptr;

    int32_t mSelectedHourIndex = 0;
    int32_t mSelectedHour = 0;
    int32_t mSelectedMinutes = 0;
    int32_t mSelectedDayOfTheWeek = 0;
    int32_t mSelectedWeather = 0;

    bool mIsImGuiInitialized = false;
    bool mLockTimeAndWeather = false;
    bool mOnlyTimecycTimes = true;

    const uint8_t mSettingsFileMajorVersion = 1;
    const uint8_t mSettingsFileMinorVersion = 2;

    bool mShowWindow = false;
    bool mShowSaveWindow = false;
    bool mShowLoadWindow = false;
    bool mShowSettingsWindow = false;
    bool mShowSetParamOnAllHoursAndWeathersWindow = false;
    bool mShowLicensesWindow = false;

    ImGuiKey mOpenWindowKey = ImGuiKey_F9;
    ImVec2 mWindowPos = ImVec2(5.0f, 5.0f);
    ImVec2 mWindowSize = ImVec2(440.0f, 650.0f);
    float mFontScale = 1.0f;
    ImGuiKey mToggleCameraControlKey = ImGuiKey_None;
    
    enum TIMECYC_PARAM_TYPE
    {
        TIMECYCPARAMTYPE_COLOR_U32 = 1,
        TIMECYCPARAMTYPE_COLOR_FLOAT3 = 2,
        TIMECYCPARAMTYPE_FLOAT = 3,
        TIMECYCPARAMTYPE_INT = 4,
    };

    std::array<std::tuple<std::string, uint32_t, TIMECYC_PARAM_TYPE>, 80> mTimecycParamNameOffsetAndType;
};