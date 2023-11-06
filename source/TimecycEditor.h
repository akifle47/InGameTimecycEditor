#pragma once
#include <vector>
#include <fstream>
#include <sstream>
#include <tuple>
#include <unordered_map>
#include <array>

#include <imgui.h>
#include <imgui_internal.h>
#include <backends\imgui_impl_win32.h>
#include "imgui_dx9_backend/imgui_impl_dx9_shader.h"

#include "Utils.h"
#include "TimeCycle.h"
#include "EmbeddedFont.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

class TimecycEditor
{
public:
	void Initialize(const uint8_t *baseAddress);

	bool OnWndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void Update();

	void OnBeforeD3D9DeviceReset(IDirect3DDevice9 *d3d9Device);
	void OnAfterD3D9DeviceReset();

	void OnBeforeD3D9DeviceEndScene(IDirect3DDevice9 *d3d9Device);

	bool mDisableMouseControl = false;

private:
	void InitializeImGui(IDirect3DDevice9 *d3d9Device);
	void InitializeColors();

	void SaveSettings();
	void LoadSettings();

	void DrawMainWindow();
	void DrawSaveWindow();
	void DrawLoadWindow();
	void DrawSettingsWindow();
	void DrawSetParamForAllHoursAndWeathersWindow();

	int32_t TimecycTimeIndexToGameTime(const int32_t timeIndex);
	int32_t GameTimeToTimecycTimeIndex(const int32_t gameTime);

	typedef int32_t(FORCE_WEATHER_NOWT)(uint32_t weatherIndex);
	typedef void(RELEASE_WEATHERT)();
	typedef int32_t(SET_TIME_ONE_DAY_FORWARDT)();
	typedef int32_t(SET_TIME_ONE_DAY_BACKT)();

	FORCE_WEATHER_NOWT *FORCE_WEATHER_NOW = nullptr;
	RELEASE_WEATHERT *RELEASE_WEATHER = nullptr;
	SET_TIME_ONE_DAY_FORWARDT *SET_TIME_ONE_DAY_FORWARD = nullptr;
	SET_TIME_ONE_DAY_BACKT *SET_TIME_ONE_DAY_BACK = nullptr;

	//Copy of timecyc parameters in types that are supported by ImGui
	float mAmbient0ColorFloat4[NUM_HOURS][NUM_WEATHERS][4];
	float mAmbient1ColorFloat4[NUM_HOURS][NUM_WEATHERS][4];
	float mDirLightColorFloat4[NUM_HOURS][NUM_WEATHERS][4];
	float mSkyBottomColorFogDensityFloat4[NUM_HOURS][NUM_WEATHERS][4];
	float mSunCoreFloat4[NUM_HOURS][NUM_WEATHERS][4];
	float mLowCloudsColorFloat4[NUM_HOURS][NUM_WEATHERS][4];
	float mBottomCloudsColorFloat4[NUM_HOURS][NUM_WEATHERS][4];
	float mWaterFloat4[NUM_HOURS][NUM_WEATHERS][4];
	float mColorCorrectionFloat4[NUM_HOURS][NUM_WEATHERS][4];
	float mColorAddFloat4[NUM_HOURS][NUM_WEATHERS][4];
	float mSkyTopColorFloat4[NUM_HOURS][NUM_WEATHERS][4];

	Timecycle *mTimeCycle = nullptr;

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
	const uint8_t mSettingsFileMinorVersion = 1;

	bool mShowWindow = false;
	bool mShowSaveWindow = false;
	bool mShowLoadWindow = false;
	bool mShowSettingsWindow = false;
	bool mShowSetParamForAllHoursAndWeathersWindow = false;

	ImGuiKey mOpenWindowKey = ImGuiKey_F9;
	ImVec2 mWindowPos = ImVec2(5.0f, 5.0f);
	ImVec2 mWindowSize = ImVec2(440.0f, 650.0f);
	float mItemInnerSpacing = 4.0f;
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