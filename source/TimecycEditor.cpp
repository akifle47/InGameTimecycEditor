#include "TimecycEditor.h"

void TimecycEditor::Initialize(const uint8_t *baseAddress)
{
	int32_t gameVersion;
	Utils::GetGameVersion(gameVersion);

	switch(gameVersion)
	{
		case 1040:
			mTimeCycle = (Timecycle*)(baseAddress + 0xCF46F0);
			mHour = (int32_t*)(baseAddress + 0xC7AD84);
			mMinutes = (int32_t*)(baseAddress + 0xC7AD80);
			mTimerLength = (uint32_t*)(baseAddress + 0xC7AD88);
			FORCE_WEATHER_NOW = (FORCE_WEATHER_NOWT*)(baseAddress + 0x446930);
			RELEASE_WEATHER = (RELEASE_WEATHERT*)(baseAddress + 0x4469A0);
			SET_TIME_ONE_DAY_FORWARD = (SET_TIME_ONE_DAY_FORWARDT*)(baseAddress + 0x5CB090);
			SET_TIME_ONE_DAY_BACK = (SET_TIME_ONE_DAY_BACKT*)(baseAddress + 0x5CB0D0);
		break;

		case 1070:
			mTimeCycle = (Timecycle*)(baseAddress + 0xDF6080);
			mHour = (int32_t*)(baseAddress + 0xDD5300);
			mMinutes = (int32_t*)(baseAddress + 0xDD52FC);
			mTimerLength = (uint32_t*)(baseAddress + 0xDD5304);
			FORCE_WEATHER_NOW = (FORCE_WEATHER_NOWT*)(baseAddress + 0x5E41D0);
			RELEASE_WEATHER = (RELEASE_WEATHERT*)(baseAddress + 0x5E4240);
			SET_TIME_ONE_DAY_FORWARD = (SET_TIME_ONE_DAY_FORWARDT*)(baseAddress + 0x765060);
			SET_TIME_ONE_DAY_BACK = (SET_TIME_ONE_DAY_BACKT*)(baseAddress + 0x7650A0);
		break;

		case 1080:
			mTimeCycle = (Timecycle*)(baseAddress + 0xFF1150);
			mHour = (int32_t*)(baseAddress + 0xD51694);
			mMinutes = (int32_t*)(baseAddress + 0xD51690);
			mTimerLength = (uint32_t*)(baseAddress + 0xD51698);
			FORCE_WEATHER_NOW = (FORCE_WEATHER_NOWT*)(baseAddress + 0x5A0910);
			RELEASE_WEATHER = (RELEASE_WEATHERT*)(baseAddress + 0x5A0980);
			SET_TIME_ONE_DAY_FORWARD = (SET_TIME_ONE_DAY_FORWARDT*)(baseAddress + 0x711E50);
			SET_TIME_ONE_DAY_BACK = (SET_TIME_ONE_DAY_BACKT*)(baseAddress + 0x711E90);
		break;

		case 1200:
			mTimeCycle = (Timecycle*)(baseAddress + 0x11E8D30);
			mHour = (int32_t*)(baseAddress + 0xE95854);
			mMinutes = (int32_t*)(baseAddress + 0xE95850);
			mTimerLength = (uint32_t*)(baseAddress + 0xE95840);
			FORCE_WEATHER_NOW = (FORCE_WEATHER_NOWT*)(baseAddress + 0x62E640);
			RELEASE_WEATHER = (RELEASE_WEATHERT*)(baseAddress + 0x62E9E0);
			SET_TIME_ONE_DAY_FORWARD = (SET_TIME_ONE_DAY_FORWARDT*)(baseAddress + 0x7D7530);
			SET_TIME_ONE_DAY_BACK = (SET_TIME_ONE_DAY_BACKT*)(baseAddress + 0x7D74F0);
		break;
	}
	
	LoadSettings();
	mTimeCycle->Load("pc/data/timecyc.dat", NULL, 0);
	InitializeColors();
}

void TimecycEditor::InitializeImGui(IDirect3DDevice9 *d3d9Device)
{
	if(!mIsImGuiInitialized)
	{
		D3DDEVICE_CREATION_PARAMETERS creationParams;
		d3d9Device->GetCreationParameters(&creationParams);

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO &io = ImGui::GetIO(); (void)io;
		
		io.IniFilename = NULL;
		
		//ImGui Style
		{
			ImGuiStyle *style = &ImGui::GetStyle();
			style->FrameRounding = 1;
			style->WindowPadding.x = 50;
			style->WindowPadding.y = 10;
			style->FramePadding.x = 1;
			style->FramePadding.y = 3;
			style->ItemSpacing.x = 10;
			style->ItemSpacing.x = 5;
			style->ScrollbarSize = 20;
			style->ScrollbarRounding = 1;
			style->GrabMinSize = 15;

			style->WindowBorderSize = 0;
			style->WindowRounding = 1;

			style->WindowTitleAlign.x = 0.5;
			style->WindowTitleAlign.y = 0.5;
			style->WindowMenuButtonPosition = 0;

			style->SeparatorTextBorderSize = 3;
		}
		//ImGui Color
		{
			ImVec4 *colors = ImGui::GetStyle().Colors;
			colors[ImGuiCol_Text] = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
			colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
			colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.98f);
			colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
			colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
			colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
			colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
			colors[ImGuiCol_FrameBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
			colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
			colors[ImGuiCol_FrameBgActive] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
			colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
			colors[ImGuiCol_TitleBgActive] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
			colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
			colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
			colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
			colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
			colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
			colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
			colors[ImGuiCol_CheckMark] = ImVec4(0.84f, 0.84f, 0.84f, 1.00f);
			colors[ImGuiCol_SliderGrab] = ImVec4(0.43f, 0.43f, 0.43f, 1.00f);
			colors[ImGuiCol_SliderGrabActive] = ImVec4(0.47f, 0.47f, 0.47f, 1.00f);
			colors[ImGuiCol_Button] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
			colors[ImGuiCol_ButtonHovered] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
			colors[ImGuiCol_ButtonActive] = ImVec4(0.45f, 0.45f, 0.45f, 1.00f);
			colors[ImGuiCol_Header] = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
			colors[ImGuiCol_HeaderHovered] = ImVec4(0.17f, 0.17f, 0.17f, 1.00f);
			colors[ImGuiCol_HeaderActive] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
			colors[ImGuiCol_Separator] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
			colors[ImGuiCol_SeparatorHovered] = ImVec4(0.19f, 0.19f, 0.19f, 1.00f);
			colors[ImGuiCol_SeparatorActive] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
			colors[ImGuiCol_ResizeGrip] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
			colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.72f, 0.72f, 0.72f, 1.00f);
			colors[ImGuiCol_ResizeGripActive] = ImVec4(0.77f, 0.77f, 0.77f, 1.00f);
			colors[ImGuiCol_Tab] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
			colors[ImGuiCol_TabHovered] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
			colors[ImGuiCol_TabActive] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
			colors[ImGuiCol_TabUnfocused] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
			colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
			colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
			colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
			colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
			colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
			colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
			colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
			colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
			colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
			colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
			colors[ImGuiCol_TextSelectedBg] = ImVec4(0.43f, 0.43f, 0.43f, 0.98f);
			colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
			colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
			colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
			colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
			colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
		}

		ImGui_ImplWin32_Init(creationParams.hFocusWindow);
		ImGui_ImplDX9_Init(d3d9Device);

		ImFontConfig conf = {};
		conf.SizePixels = 13;
		conf.OversampleH = 2;
		conf.OversampleV = 2;

		static const ImWchar ranges[] =
		{
			0x0020, 0x00FF, // Basic Latin + Latin Supplement
			0,
		};
		const ImWchar* glyph_ranges = ranges;

		io.Fonts->AddFontFromMemoryCompressedBase85TTF(gCousineRegularCompressedDataBase85, conf.SizePixels, &conf, glyph_ranges);
		io.FontGlobalScale = mFontScale;

		mIsImGuiInitialized = true;
	}
}

void TimecycEditor::InitializeColors()
{
	for(uint32_t time = 0; time < NUM_HOURS; time++)
	{
		for(uint32_t weather = 0; weather < NUM_WEATHERS; weather++)
		{
			U32ToFloat4(mTimeCycle->mParams[time][weather].mAmbient0Color, mAmbient0ColorFloat4[time][weather]);
			U32ToFloat4(mTimeCycle->mParams[time][weather].mAmbient1Color, mAmbient1ColorFloat4[time][weather]);
			U32ToFloat4(mTimeCycle->mParams[time][weather].mDirLightColor, mDirLightColorFloat4[time][weather]);
			U32ToFloat4(mTimeCycle->mParams[time][weather].mSkyBottomColorFogDensity, mSkyBottomColorFogDensityFloat4[time][weather]);
			U32ToFloat4(mTimeCycle->mParams[time][weather].mSunCore, mSunCoreFloat4[time][weather]);
			U32ToFloat4(mTimeCycle->mParams[time][weather].mLowCloudsColor, mLowCloudsColorFloat4[time][weather]);
			U32ToFloat4(mTimeCycle->mParams[time][weather].mBottomCloudsColor, mBottomCloudsColorFloat4[time][weather]);
			U32ToFloat4(mTimeCycle->mParams[time][weather].mWater, mWaterFloat4[time][weather]);
			U32ToFloat4(mTimeCycle->mParams[time][weather].mColorCorrection, mColorCorrectionFloat4[time][weather]);
			U32ToFloat4(mTimeCycle->mParams[time][weather].mColorAdd, mColorAddFloat4[time][weather]);
		}
	}
}

void TimecycEditor::SaveSettings()
{
	std::ofstream file("InGameTimecycSettings.bin", std::ios::binary);

	if(!file.good())
	{
		return;
	}
	
	file.write((char*)&mSettingsFileMajorVersion, sizeof(char));
	file.write((char*)&mSettingsFileMinorVersion, sizeof(char));
	file.write((char*)&mWindowPos.x, sizeof(float));
	file.write((char*)&mWindowPos.y, sizeof(float));
	file.write((char*)&mWindowSize.x, sizeof(float));
	file.write((char*)&mWindowSize.y, sizeof(float));
	file.write((char*)&mFontScale, sizeof(float));
	file.write((char*)&mOpenWindowKey, sizeof(ImGuiKey));
	file.write((char*)&mToggleCameraControlKey, sizeof(ImGuiKey));
	file.write((char*)&mItemInnerSpacing, sizeof(float));
}

void TimecycEditor::LoadSettings()
{
	std::ifstream file("InGameTimecycSettings.bin", std::ios::binary);

	if(!file.good())
	{
		return;
	}
	
	file.seekg(2);
	file.read((char*)&mWindowPos.x, sizeof(float));
	file.read((char*)&mWindowPos.y, sizeof(float));
	file.read((char*)&mWindowSize.x, sizeof(float));
	file.read((char*)&mWindowSize.y, sizeof(float));
	file.read((char*)&mFontScale, sizeof(float));
	file.read((char*)&mOpenWindowKey, sizeof(ImGuiKey));
	file.read((char*)&mToggleCameraControlKey, sizeof(ImGuiKey));
	file.read((char*)&mItemInnerSpacing, sizeof(float));

	if(mIsImGuiInitialized)
	{
		ImGui::GetIO().FontGlobalScale = mFontScale;
		ImGui::GetStyle().ItemInnerSpacing.x = mItemInnerSpacing;
	}

	HWND gameWindow = FindWindow(L"grcWindow", L"GTAIV");
	if(!gameWindow)
	{
		return;
	}

	RECT gameWindowRect = {};
	GetWindowRect(gameWindow, &gameWindowRect);

	int32_t gameWindowWidth = gameWindowRect.right - gameWindowRect.left;
	int32_t gameWindowHeight = gameWindowRect.bottom - gameWindowRect.top;

	if(mWindowPos.x >= gameWindowWidth || mWindowPos.x < 0.0f)
	{
		mWindowPos.x = 0.0f;
	}
	if(mWindowPos.y >= gameWindowHeight || mWindowPos.y < 0.0f)
	{
		mWindowPos.y = 0.0f;
	}
}

bool TimecycEditor::OnWndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(mIsImGuiInitialized)
	{
		if(ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
		{
			return true;
		}
	}

	return false;
}

void TimecycEditor::Update()
{
	static bool prevShowWindow = 0;
	prevShowWindow = mShowWindow;
	
	if(ImGui::IsKeyPressed(mOpenWindowKey))
	{
		mShowWindow = !mShowWindow;

		if(mShowWindow)
		{
			LoadSettings();
			ImGui::GetIO().FontGlobalScale = mFontScale;

			mDisableMouseControl = true;
			ImGui::GetIO().MouseDrawCursor = 1;
		}
		else
		{
			mDisableMouseControl = false;
			ImGui::GetIO().MouseDrawCursor = 0;
		}
	}
	
	if(mShowWindow)
	{
		if(ImGui::IsKeyPressed(mToggleCameraControlKey))
		{
			mDisableMouseControl = !mDisableMouseControl;
		}

		if(mLockTimeAndWeather)
		{
			*mHour = mSelectedHour;
			*mMinutes = mSelectedMinutes;

			FORCE_WEATHER_NOW(mSelectedWeather);
		}
	}
	
	bool windowWasJustClosed = prevShowWindow && !mShowWindow;
	if(windowWasJustClosed)
	{
		RELEASE_WEATHER();
		*mTimerLength = 2000;
	}

	bool windowWasJustOpened = !prevShowWindow && mShowWindow;
	if(windowWasJustOpened && mLockTimeAndWeather)
	{
		*mTimerLength = 30000;
	}
}

void TimecycEditor::OnBeforeD3D9DeviceReset(IDirect3DDevice9 *d3d9Device)
{
	InitializeImGui(d3d9Device);

	ImGui_ImplDX9_InvalidateDeviceObjects();
}

void TimecycEditor::OnAfterD3D9DeviceReset()
{
	ImGui_ImplDX9_CreateDeviceObjects();
}

void TimecycEditor::OnBeforeD3D9DeviceEndScene(IDirect3DDevice9 *d3d9Device)
{
	InitializeImGui(d3d9Device);

	Update();

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	if(mShowWindow)
	{
		DrawMainWindow();
		DrawSaveWindow();
		DrawLoadWindow();
		DrawSettingsWindow();
	}

	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
}

void TimecycEditor::DrawMainWindow()
{
	const char *timeOfDayNames[NUM_HOURS] = {"Midnight", "5AM", "6AM", "7AM", "9AM", "Midday", "18PM", "19PM", "20PM", "21PM", "22PM"};
	const char *timeOfDayName = timeOfDayNames[mSelectedHourIndex];
	const char *weatherNames[NUM_WEATHERS - 1] = {"EXTRASUNNY", "SUNNY", "SUNNY_WINDY", "CLOUDY", "RAIN", "DRIZZLE", "FOGGY", "LIGHTNING"};
	const char *weatherName = weatherNames[mSelectedWeather];

	ImGui::Begin("Timecyc Editor 1.2", NULL, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus);
	ImGui::SetWindowPos(mWindowPos);
	ImGui::SetWindowSize(mWindowSize);

	if(ImGui::BeginMenuBar())
	{
		if(ImGui::BeginMenu("Load"))
		{
			if(ImGui::MenuItem("timecyc.dat##Load"))
			{
				mTimeCycle->Load("pc/data/timecyc.dat", NULL, 0);
				InitializeColors();
			}
			if(ImGui::MenuItem("Load From"))
			{
				mShowLoadWindow = true;
			}

			ImGui::EndMenu();
		}

		if(ImGui::BeginMenu("Save"))
		{
			if(ImGui::MenuItem("timecyc.dat##Save"))
			{
				mTimeCycle->Save("pc/data/timecyc.dat", NULL, 0);
			}

			if(ImGui::MenuItem("Save As"))
			{
				mShowSaveWindow = true;
			}

			ImGui::EndMenu();
		}

		if(ImGui::BeginMenu("Settings"))
		{
			mShowSettingsWindow = true;

			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
	}

	ImGui::Checkbox("Lock to Selected Time, Weather and Day", &mLockTimeAndWeather);
	if(ImGui::IsItemEdited())
	{
		if(mLockTimeAndWeather)
		{
			*mTimerLength = 30000;
		}
		else
		{
			RELEASE_WEATHER();
			*mTimerLength = 2000;
		}
	}

	ImGui::Checkbox("Only Show Timecyc Times", &mOnlyTimecycTimes);
	
	ImGui::NewLine();

	if(mOnlyTimecycTimes)
	{
		mSelectedHour = TimecycTimeIndexToGameTime(mSelectedHourIndex);

		ImGui::Text("Time of Day");
		ImGui::SliderInt("##Hour", &mSelectedHourIndex, 0, NUM_HOURS - 1, timeOfDayName);
	}
	else
	{
		mSelectedHourIndex = GameTimeToTimecycTimeIndex(mSelectedHour);

		std::stringstream label;
		label << "Time of Day (Editing " << timeOfDayNames[mSelectedHourIndex] << ")";

		ImGui::Text(label.str().c_str());
		ImGui::SliderInt("##Hour", &mSelectedHour, 0, 23);
	}
	
	ImGui::SliderInt("##Minutes", &mSelectedMinutes, 0, 59);

	
	ImGui::Text("Day");

	ImGui::SameLine();
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort) && ImGui::BeginTooltip())
	{
		ImGui::TextUnformatted("Affects Cloud Noise. Don't save your game if you mess with this");
		ImGui::EndTooltip();
	}

	if(ImGui::Button("Skip One Day Forward"))
	{
		SET_TIME_ONE_DAY_FORWARD();
	}
	if(ImGui::Button("Skip One Day Back"))
	{
		SET_TIME_ONE_DAY_BACK();
	}

	ImGui::Text("Weather");
	ImGui::SliderInt("##Weather", &mSelectedWeather, 0, NUM_WEATHERS - 2, weatherName);

	ImGui::NewLine();

	if(ImGui::CollapsingHeader("Lighting"))
	{
		ImGui::SeparatorText("Ambient Light 0");
		{
			ImGui::Text("Color");
			ImGui::ColorEdit3("##Ambient Light 0 Color", mAmbient0ColorFloat4[mSelectedHourIndex][mSelectedWeather]);
			Float4ToU32(mAmbient0ColorFloat4[mSelectedHourIndex][mSelectedWeather], mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mAmbient0Color);

			ImGui::Text("Multiplier");
			ImGui::DragFloat("##Ambient Light 0 Multiplier", &mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mAmbient0Multiplier, 0.005f, 0.0f, FLT_MAX);
		}
		ImGui::SeparatorText("Ambient Light 1");
		{
			ImGui::Text("Color");
			ImGui::ColorEdit3("##Ambient 1 Color", mAmbient1ColorFloat4[mSelectedHourIndex][mSelectedWeather]);
			Float4ToU32(mAmbient1ColorFloat4[mSelectedHourIndex][mSelectedWeather], mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mAmbient1Color);

			ImGui::Text("Multiplier");
			ImGui::DragFloat("##Ambient Light 1 Light Multiplier", &mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mAmbient1Multiplier, 0.005f, 0.0f, FLT_MAX);
		}
		ImGui::SeparatorText("Directional Light");
		{
			ImGui::Text("Color");
			ImGui::ColorEdit3("##Directional Light Color", mDirLightColorFloat4[mSelectedHourIndex][mSelectedWeather]);
			Float4ToU32(mDirLightColorFloat4[mSelectedHourIndex][mSelectedWeather], mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mDirLightColor);

			ImGui::Text("Multiplier");
			ImGui::DragFloat("##Directional Light multitplier", &mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mDirLightMultiplier, 0.005f, 0.0f, FLT_MAX);

			ImGui::Text("Specular Multiplier");
			ImGui::DragFloat("##Directional Light Specular Multiplier", &mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mDirLightSpecMultiplier, 0.005f, 0.0f, FLT_MAX);
		}
		ImGui::SeparatorText("Water");
		{
			ImGui::Text("Color");
			ImGui::ColorEdit4("##Water Color", mWaterFloat4[mSelectedHourIndex][mSelectedWeather]);
			Float4ToU32(mWaterFloat4[mSelectedHourIndex][mSelectedWeather], mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mWater);

			ImGui::Text("Reflection Multiplier");
			ImGui::DragFloat("##Water Reflection Multiplier", &mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mWaterReflectionMultiplier, 0.005f, 0.0f, FLT_MAX);
		}
		ImGui::SeparatorText("");

		ImGui::Text("Rim Lighting Multiplier");
		ImGui::DragFloat("##Rim Lighting Multiplier", &mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mRimLightingMultiplier, 0.005f, 0.0f, FLT_MAX);

		ImGui::Text("Sky Light Multiplier");
		ImGui::DragFloat("##Sky Light Multiplier", &mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mSkyLightMultiplier, 0.005f, 0.0f, FLT_MAX);

		ImGui::Text("Global Reflection Multiplier");
		ImGui::DragFloat("##Global Reflection Multiplier", &mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mGlobalReflectionMultiplier, 0.005f, 0.0f, FLT_MAX);

		ImGui::Text("AO Strength");
		ImGui::DragFloat("##AO Strength", &mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mAOStrength, 0.005f, 0.0f, FLT_MAX);

		ImGui::Text("Ped AO Strength");
		ImGui::DragFloat("##Ped AO Strength", &mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mPedAOStrength, 0.005f, 0.0f, FLT_MAX);

		ImGui::Text("Corona Size");
		ImGui::DragFloat("##Corona Size", &mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mCoronaSize, 0.005f, 0.0f, FLT_MAX);

		ImGui::Text("Corona Brightness");
		ImGui::DragFloat("##Corona Brightness", &mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mCoronaBrightness, 0.005f, 0.0f, FLT_MAX);

		ImGui::Text("Distant Corona Size");
		ImGui::DragFloat("##Distant Corona Size", &mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mDistantCoronaSize, 0.05f, 0.0f, FLT_MAX);

		ImGui::Text("Distant Corona Brightness");
		ImGui::DragFloat("##Distant Corona Brightness", &mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mDistantCoronaBrightness, 0.05f, 0.0f, FLT_MAX);

		ImGui::Text("Particle Brightness");
		ImGui::DragFloat("##Particle Brightness", &mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mParticleBrightness, 0.005f, 0.0f, FLT_MAX);
	}
	
	if(ImGui::CollapsingHeader("Atmosphere"))
	{
		ImGui::Text("Temperature");
		ImGui::DragFloat("##Temperature", &mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mTemperature, 0.1f);

		if(ImGui::TreeNode("Fog"))
		{
			ImGui::Text("Start");
			ImGui::DragFloat("##Fog Start", &mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mFogStart, 0.5f);

			ImGui::Text("Density");
			ImGui::DragFloat("##Fog Density", &mSkyBottomColorFogDensityFloat4[mSelectedHourIndex][mSelectedWeather][3], 0.005f, 0.0f, 1.0f);
			Float4ToU32(mSkyBottomColorFogDensityFloat4[mSelectedHourIndex][mSelectedWeather], mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mSkyBottomColorFogDensity);

			ImGui::TreePop();
		}
		if(ImGui::TreeNode("Sky"))
		{
			ImGui::Text("Brightness");
			ImGui::DragFloat("##Sky Brightness", &mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mSkyBrightness, 0.005f, 0.0f, FLT_MAX);

			ImGui::Text("Bottom Color");
			ImGui::ColorEdit3("##Sky Bottom Color", mSkyBottomColorFogDensityFloat4[mSelectedHourIndex][mSelectedWeather]);
			Float4ToU32(mSkyBottomColorFogDensityFloat4[mSelectedHourIndex][mSelectedWeather], mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mSkyBottomColorFogDensity);

			ImGui::Text("Top Color");
			ImGui::ColorEdit3("##Sky Top Color", mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mSkyColor, ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float);

			ImGui::Text("East Horizon Color");
			ImGui::ColorEdit3("##Sky East Horizon Color", mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mSkyHorizonColor, ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float);

			ImGui::Text("West Horizon Color");
			ImGui::ColorEdit3("##Sky West Horizon Color", mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mSkyEastHorizonColor, ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float);

			ImGui::Text("Horizon Brightness");
			ImGui::DragFloat("##Sky East Horizon Brightness", &mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mSkyHorizonBrightness, 0.005f, 0.0f, FLT_MAX);

			ImGui::Text("Horizon Height Fade Out");
			ImGui::DragFloat("##Sky Horizon Height Fade Out", &mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mSkyHorizonHeight, 0.05f, 0.0f, FLT_MAX);

			ImGui::NewLine();

			ImGui::Text("Sun Color");
			ImGui::ColorEdit3("##Sun Color", mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mSunColor, ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float);
			
			ImGui::Text("Sun Size");
			ImGui::DragFloat("##Sun Size", &mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mSunSize, 0.001f, 0.0f, FLT_MAX);

			ImGui::TreePop();
		}
		if(ImGui::TreeNode("Clouds"))
		{
			ImGui::Text("Color 1");
			ImGui::ColorEdit3("##Cloud Color 1", mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mCloud1Color, ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float);

			ImGui::Text("Color 2");
			ImGui::ColorEdit3("##Cloud Color 2", mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mCloud2Color, ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float);

			ImGui::Text("Color 3");
			ImGui::ColorEdit3("##Cloud 3 Color", mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mCloud3Color, ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float);

			ImGui::Text("Brightness");
			ImGui::DragFloat("##Cloud Brightness", &mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mCloudsBrightness, 0.05f, 0.0f, FLT_MAX);

			ImGui::Text("Height Fade Out");
			ImGui::DragFloat("##Clouds Fade Out", &mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mCloudsFadeOut, 0.05f, 0.0f, FLT_MAX);

			ImGui::SeparatorText("Detail Noise");
			{
				ImGui::Text("Scale");
				ImGui::DragFloat("##Detail Noise Scale", &mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mDetailNoiseScale, 0.5f, 0.0f, FLT_MAX);

				ImGui::Text("Offset/Scrolling Speed");
				ImGui::DragFloat("##Detail Noise Offset", &mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mDetailNoiseOffset, 0.05f);

				ImGui::Text("Multiplier");
				ImGui::DragFloat("##Detail Noise Multiplier", &mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mDetailNoiseMultiplier, 0.005f, FLT_MIN, FLT_MAX);
			}

			ImGui::SeparatorText("Clouds 1");
			{
				ImGui::Text("Threshold");
				ImGui::DragFloat("##Clouds 1 Threshold", &mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mCloud1Threshold, 0.005f, 0.0f, FLT_MAX);

				ImGui::Text("Bias");
				ImGui::DragFloat("##Clouds 1 Bias", &mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mCloud1Bias, 0.005f,	0.0f, FLT_MAX);

				ImGui::Text("Detail");
				ImGui::DragFloat("##Clouds 1 Detail", &mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mCloud1Detail, 0.005f, 0.0f, FLT_MAX);

				ImGui::Text("Height");
				ImGui::DragFloat("##Clouds 1 Height", &mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mCloud1Height, 0.005f, 0.0f, FLT_MAX);
			}
			ImGui::SeparatorText("Clouds 2");
			{
				ImGui::Text("Threshold");
				ImGui::DragFloat("##Clouds 2 Threshold", &mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mCloud2Threshold, 0.005f, 0.0f, FLT_MAX);

				ImGui::Text("Bias 1");
				ImGui::DragFloat("##Clouds 2 Bias 1", &mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mCloud2Bias1, 0.005f, 0.0f, FLT_MAX);

				ImGui::Text("Bias 2");
				ImGui::DragFloat("##Clouds 2 Bias 2", &mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mCloud2Bias2, 0.005f, 0.0f, FLT_MAX);

				ImGui::Text("Scale");
				ImGui::DragFloat("##Clouds 2 Scale", &mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mCloud2Scale, 0.005f, 0.0f, FLT_MAX);

				ImGui::Text("Softness");
				ImGui::DragFloat("##Clouds 2 Softness", &mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mCloud2Offset, 0.005f, 0.0f, FLT_MAX);

				ImGui::Text("Shadow Strength");
				ImGui::DragFloat("##Clouds 2 Shadow Strength", &mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mCloud2ShadowStrength, 0.005f, 0.0f, 1.0f);
			}

			ImGui::TreePop();
		}

		if(ImGui::TreeNode("Night Sky"))
		{
			ImGui::Text("Visible Stars");
			ImGui::DragFloat("##Visible Stars", &mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mVisibleStars, 0.005f, 0.0f, FLT_MAX);

			ImGui::Text("Star Field Brightness");
			ImGui::DragFloat("##Star Field Brightness", &mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mStarsBrightness, 0.005f, 0.0f, FLT_MAX);

			ImGui::NewLine();

			ImGui::Text("Moon Color");
			ImGui::ColorEdit3("##Moon Color", mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mMoonColor, ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float);

			ImGui::Text("Moon Brightness");
			ImGui::DragFloat("##Moon Brightness", &mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mMoonBrightness, 0.001f, 0.0f, FLT_MAX);

			ImGui::Text("Moon Glow");
			ImGui::DragFloat("##Moon Glow", &mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mMoonGlow, 0.005f, 0.0f, FLT_MAX);

			ImGui::Text("Unknown");
			ImGui::DragFloat("##Moon Unknown", &mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mMoonParam3, 0.015f);

			ImGui::TreePop();
		}
	}

	if(ImGui::CollapsingHeader("Post Processing"))
	{
		ImGui::Text("Film Grain");
		ImGui::DragInt("##Film Grain", &mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mFilmGrain, 0.5f, 0, INT32_MAX);

		ImGui::Text("Luminance Min");
		ImGui::DragFloat("##Luminance Min", &mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mLumMin, 0.005f, 0.0f, mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mLumMax);

		ImGui::Text("Luminance Max");
		ImGui::DragFloat("##Luminance Max", &mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mLumMax, 0.005f, mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mLumMin, FLT_MAX);

		ImGui::Text("Luminance Delay");
		ImGui::DragFloat("##Luminance Delay", &mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mLumDelay, 0.005f, 0.0f, FLT_MAX);

		ImGui::Text("Exposure");
		ImGui::DragFloat("##Exposure", &mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mExposure, 0.001f, 0.0f, FLT_MAX);

		ImGui::Text("Color Correction");
		ImGui::ColorEdit3("##Color Correction", mColorCorrectionFloat4[mSelectedHourIndex][mSelectedWeather]);
		Float4ToU32(mColorCorrectionFloat4[mSelectedHourIndex][mSelectedWeather], mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mColorCorrection);

		ImGui::Text("Color Add");
		ImGui::ColorEdit3("##Color Add", mColorAddFloat4[mSelectedHourIndex][mSelectedWeather]);
		Float4ToU32(mColorAddFloat4[mSelectedHourIndex][mSelectedWeather], mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mColorAdd);

		ImGui::Text("Bloom Threshold");
		ImGui::DragFloat("##Bloom Threshold", &mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mBloomThreshold, 0.005f, 0.0f, FLT_MAX);

		ImGui::Text("Bloom Intensity");
		ImGui::DragFloat("##Bloom Intensity", &mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mBloomIntensity, 0.005f, 0.0f, FLT_MAX);

		ImGui::Text("Mid Gray Value");
		ImGui::DragFloat("##Mid Gray Value", &mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mMidGrayValue, 0.005f, 0.0f, FLT_MAX);

		ImGui::Text("Desaturation");
		ImGui::DragFloat("##Desaturation", &mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mDesaturation, 0.005f, 0.0f, 1.0f);

		ImGui::Text("Desaturation Far");
		ImGui::DragFloat("##Desaturation Far", &mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mDesaturationFar, 0.005f, 0.0f, 1.0f);

		ImGui::Text("Gamma");
		ImGui::DragFloat("##Gamma", &mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mGamma, 0.005f, 0.0f, FLT_MAX);

		ImGui::Text("Gamma Far");
		ImGui::DragFloat("##Gamma Far", &mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mGammaFar, 0.005f, 0.0f, FLT_MAX);

		ImGui::Text("DepthFX Near");
		ImGui::DragFloat("##DepthFX Near", &mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mDepthFxNear, 0.5f, 0.0f, mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mDepthFxFar);

		ImGui::Text("DepthFX Far");
		ImGui::DragFloat("##DepthFX Far", &mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mDepthFxFar, 0.5f, mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mDepthFxNear, FLT_MAX);

		ImGui::Text("DOF Start");
		ImGui::DragFloat("##DOF Start", &mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mDOFStart, 0.5f, 0.0f, FLT_MAX);

		ImGui::Text("Near DOF Blur");
		ImGui::DragFloat("##Near DOF Blur", &mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mNearDOFBlur, 0.005f, 0.0f, mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mFarDOFBlur);

		ImGui::Text("Far DOF Blur");
		ImGui::DragFloat("##Far DOF Blur", &mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mFarDOFBlur, 0.005f, mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mNearDOFBlur, 1.0f);
	}

	if(ImGui::CollapsingHeader("Other"))
	{
		ImGui::Text("Far Clip");
		ImGui::DragFloat("##Far Clip", &mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mFarClip, 0.5f, 0.0f, FLT_MAX);
	}

	if(ImGui::CollapsingHeader("(Probably) Unused"))
	{
		ImGui::Text("Contrast Far");
		ImGui::DragFloat("##Contrast Far", &mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mContrastFar, 0.1f);

		ImGui::Text("Contrast");
		ImGui::DragFloat("##Contrast", &mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mContrast, 0.1f);

		ImGui::Text("Low clouds Color");
		ImGui::ColorEdit3("##Low clouds Color", mLowCloudsColorFloat4[mSelectedHourIndex][mSelectedWeather]);
		Float4ToU32(mLowCloudsColorFloat4[mSelectedHourIndex][mSelectedWeather], mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mLowCloudsColor);

		ImGui::Text("Bottom clouds Color");
		ImGui::ColorEdit3("##Bottom clouds Color", mBottomCloudsColorFloat4[mSelectedHourIndex][mSelectedWeather]);
		Float4ToU32(mBottomCloudsColorFloat4[mSelectedHourIndex][mSelectedWeather], mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mBottomCloudsColor);

		ImGui::Text("Cloud alpha");
		ImGui::DragInt("##Cloud alpha", &mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mCloudAlpha);

		ImGui::Text("Sun core");
		ImGui::ColorEdit3("##Sun Core", mSunCoreFloat4[mSelectedHourIndex][mSelectedWeather]);
		Float4ToU32(mSunCoreFloat4[mSelectedHourIndex][mSelectedWeather], mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mSunCore);

		ImGui::Text("Unknown 3");
		ImGui::DragFloat("##Unknown 3", &mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mUnknown3, 0.1f);

		ImGui::Text("Unknown 29");
		ImGui::DragFloat("##Unknown 29", &mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mUnknown29, 0.1f);

		ImGui::Text("Unknown 46");
		ImGui::DragFloat("##Unknown 46", &mTimeCycle->mParams[mSelectedHourIndex][mSelectedWeather].mUnknown46, 0.1f);
	}
	
	ImGui::End();
}

void TimecycEditor::DrawSaveWindow()
{
	if(mShowSaveWindow)
	{
		static char errorMessage[256];
		static char fileName[256] = "filename.dat";

		ImGui::Begin("Save As");
		{
			ImGui::InputText("##fileName", fileName, 256);

			if(ImGui::Button("Save"))
			{
				if(mTimeCycle->Save(fileName, errorMessage, 256))
				{
					mShowSaveWindow = false;
				}
			}

			ImGui::SameLine();

			if(ImGui::Button("Cancel"))
			{
				mShowSaveWindow = false;
			}

			ImGui::Text(errorMessage);
		}
		ImGui::End();
	}
}

void TimecycEditor::DrawLoadWindow()
{
	if(mShowLoadWindow)
	{
		static char errorMessage[256];
		static char fileName[256] = "filename.dat";

		ImGui::Begin("Load From");
		{
			ImGui::InputText("##fileName", fileName, 256);

			if(ImGui::Button("Load"))
			{
				if(mTimeCycle->Load(fileName, errorMessage, 256))
				{
					InitializeColors();
					mShowLoadWindow = false;
				}
			}

			ImGui::SameLine();

			if(ImGui::Button("Cancel"))
			{
				mShowLoadWindow = false;
			}

			ImGui::Text(errorMessage);
		}
		ImGui::End();
	}
}

void TimecycEditor::DrawSettingsWindow()
{
	if(mShowSettingsWindow)
	{
		ImGui::Begin("Settings", NULL, ImGuiWindowFlags_NoResize);
		{
			ImGui::SetWindowSize(ImVec2(330.0f, 345.0f));
			
			ImGui::Text("Position");
			ImGui::DragFloat2("##Position", (float*)&mWindowPos, 1.0f, 0.0f, FLT_MAX);
			ImGui::Text("Size");
			ImGui::DragFloat2("##Size", (float*)&mWindowSize, 1.0f, 1.0f, FLT_MAX);
			ImGui::Text("Font Scale");
			ImGui::DragFloat("##Font Scale", &mFontScale, 0.001f, 0.1f, 2.0f);
			ImGui::Text("Item Inner Spacing");
			ImGui::DragFloat("##Item Inner Spacing", &mItemInnerSpacing, 0.1f, 0.1f, 100.0f);

			ImGui::GetIO().FontGlobalScale = mFontScale;
			ImGui::GetStyle().ItemInnerSpacing.x = mItemInnerSpacing;

			//open window key setting
			ImGui::NewLine();

			static ImGuiKey newOpenWindowKey = mOpenWindowKey;
			std::string openEditorKeyStr = "Open Editor Window Key: " + std::string(ImGui::GetKeyName(newOpenWindowKey));
			ImGui::Text(openEditorKeyStr.c_str());

			static bool showChangeWindowKeyPrompt = false;
			if(ImGui::Button("Change"))
			{
				showChangeWindowKeyPrompt = true;
			}

			if(showChangeWindowKeyPrompt)
			{
				ImGui::TextWrapped("Press the Key You Want to Use to Open the Editor Window");

				for(uint32_t i = ImGuiKey_Tab; i < ImGuiKey_KeypadEqual; i++)
				{
					if(ImGui::IsKeyPressed((ImGuiKey)i) && i != mToggleCameraControlKey)
					{
						newOpenWindowKey = (ImGuiKey)i;
						showChangeWindowKeyPrompt = false;
						break;
					}
				}
			}

			ImGui::NewLine();
			
			//toggle mouse control setting
			static ImGuiKey newCameraToggleKey = mToggleCameraControlKey;
			std::string toggleCameraKeyStr = "Toggle Camera Control Key: " + std::string(ImGui::GetKeyName(newCameraToggleKey));
			ImGui::Text(toggleCameraKeyStr.c_str());

			static bool showChangeCameraToggleKeyPrompt = false;
			if(ImGui::Button("Change##2"))
			{
				showChangeCameraToggleKeyPrompt = true;
			}

			if(showChangeCameraToggleKeyPrompt)
			{
				ImGui::TextWrapped("Press the Key You Want to Use to Toggle Camera Control");

				for(uint32_t i = ImGuiKey_Tab; i < ImGuiKey_KeypadEqual; i++)
				{
					if(ImGui::IsKeyPressed((ImGuiKey)i) && i != mOpenWindowKey)
					{
						newCameraToggleKey = (ImGuiKey)i;
						showChangeCameraToggleKeyPrompt = false;
						break;
					}
				}
			}

			ImGui::NewLine();

			if(ImGui::Button("Save"))
			{
				mOpenWindowKey = newOpenWindowKey;
				mToggleCameraControlKey = newCameraToggleKey;
				SaveSettings();
				mShowSettingsWindow = false;
				showChangeWindowKeyPrompt = false;
				showChangeCameraToggleKeyPrompt = false;
			}

			ImGui::SameLine();

			if(ImGui::Button("Cancel"))
			{
				newOpenWindowKey = mOpenWindowKey;
				newCameraToggleKey = mToggleCameraControlKey;
				LoadSettings();
				mShowSettingsWindow = false;
				showChangeWindowKeyPrompt = false;
				showChangeCameraToggleKeyPrompt = false;
			}
		}
		ImGui::End();
	}
}

void TimecycEditor::Float4ToU32(const float *inColor, uint32_t &outColor)
{
	outColor = 0;
	outColor |= ((uint32_t)(inColor[0] * 255.0f + 0.5f)) << 16;
	outColor |= ((uint32_t)(inColor[1] * 255.0f + 0.5f)) << 8;
	outColor |= ((uint32_t)(inColor[2] * 255.0f + 0.5f)) << 0;
	outColor |= ((uint32_t)(inColor[3] * 255.0f + 0.5f)) << 24;
}
void TimecycEditor::U32ToFloat4(const uint32_t inColor, float *outColor)
{
	outColor[0] = ((inColor >> 16) & 0xFF) / 255.0f;
	outColor[1] = ((inColor >> 8)  & 0xFF) / 255.0f;
	outColor[2] = ((inColor >> 0)  & 0xFF) / 255.0f;
	outColor[3] = ((inColor >> 24) & 0xFF) / 255.0f;
}

int32_t TimecycEditor::TimecycTimeIndexToGameTime(const int32_t timeIndex)
{
	const int32_t timecycTimeIndexToGameTime[NUM_HOURS] = {0, 5, 6, 7, 9, 12, 18, 19, 20, 21, 22};

	return timecycTimeIndexToGameTime[timeIndex];
}
int32_t TimecycEditor::GameTimeToTimecycTimeIndex(const int32_t gameTime)
{
	const int32_t gameTimeToTimecycTimeIndex[24] = {0, 0, 0, 0, 0, 1, 2, 3, 3, 4, 4, 4, 5, 5, 5, 5, 5, 5, 6, 7, 8, 9, 10, 10};

	return gameTimeToTimecycTimeIndex[gameTime];
}