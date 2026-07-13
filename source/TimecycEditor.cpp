#include "TimecycEditor.h"
#include "TimecycModifierEditor.h"
#include "EmbeddedFont.h"
#include "Hooking.Patterns.h"
#include "injector/injector.hpp"
#include "Common.h"

#include "imgui/backends/imgui_impl_win32.h"

#include <d3d9.h>
#include <fstream>
#include <sstream>

std::unordered_map<std::string, std::array<Colorf, TimeCycle::NUM_HOURS * TimeCycle::NUM_WEATHERS>> sColorsAsFloat;

static const char* AMBIENT_LIGHT_0_COLOR_ID   = "##AMBIENT_LIGHT_0_COLOR";
static const char* AMBIENT_LIGHT_1_COLOR_ID   = "##AMBIENT_LIGHT_1_COLOR";
static const char* DIRECTIONAL_LIGHT_COLOR_ID = "##DIRECTIONAL_LIGHT_COLOR";
static const char* FOG_COLOR_ID               = "##FOG_COLOR";
static const char* COLOR_CORRECT_ID           = "##COLOR_CORRECT";
static const char* COLOR_ADD_ID               = "##COLOR_ADD";
static const char* WATER_COLOR_ID             = "##WATER_COLOR";

void TimecycEditor::Initialize()
{
    auto pattern = FindPattern({"81 C1 ? ? ? ? 52 51",  "? 05 ? ? ? ? 89 0D ? ? ? ? 50"});
    *(uint32_t*)&TimeCycle::m_ColourSets = *(uint32_t*)pattern.get_first(2);
    
    pattern = hook::pattern("8B 15 ? ? ? ? 3B C8");
    mHours = *(int32_t**)pattern.get_first(2);

    pattern = hook::pattern("3B 1D ? ? ? ? 7D ? 6A");
    mMinutes = *(int32_t**)pattern.get_first(2);

    pattern = FindPattern({"8B 15 ? ? ? ? 56 8B F1 2B F0",  "8B 3D ? ? ? ? 2B C1 3B C7 7E"});
    mTimerLength = *(uint32_t**)pattern.get_first(2);

    pattern = hook::pattern("8B 44 24 ? 39 05 ? ? ? ? 75");
    if(pattern.size() != 1)
        pattern = hook::pattern("8B 4C 24 ? 39 0D ? ? ? ? 75");
    ForceWeather = (decltype(ForceWeather))pattern.get_first(0);

    pattern = FindPattern({"E8 ? ? ? ? 8B 44 24 ? 39 05",  "E8 ? ? ? ? 8B 4C 24 ? 8B 74 24 ? 39 0D"});
    ReleaseWeather = injector::GetBranchDestination(pattern.get_first(0)).get();

    auto searchStringPattern = [](const char* str)
    {
        char patternStr[512]{};
        for(size_t i = 0; i < strlen(str); i++)
        {
            char hex[3]{};
            sprintf(hex, "%X", str[i]);
            strcat(patternStr, hex);
            if(i != sizeof(str) - 1)
                strcat(patternStr, " ");
        }

        hook::pattern dataPattern{};
        HMODULE base = GetModuleHandle(NULL);
        IMAGE_DOS_HEADER* dos = (IMAGE_DOS_HEADER*)base;
        IMAGE_NT_HEADERS* nt = (IMAGE_NT_HEADERS*)((uint8_t*)(base)+dos->e_lfanew);
        IMAGE_SECTION_HEADER* section = IMAGE_FIRST_SECTION(nt);
        for(WORD i = 0; i < nt->FileHeader.NumberOfSections; i++, section++)
        {
            if(strncmp((char*)section->Name, ".rdata", 8) == 0)
            {
                uintptr_t rDataBase = (uintptr_t)base + section->VirtualAddress;
                uintptr_t rDataEnd = rDataBase + section->Misc.VirtualSize;
                dataPattern = hook::make_range_pattern(rDataBase, rDataEnd, patternStr);
                dataPattern.get(0);
                break;
            }
        }

        return dataPattern;
    };

    pattern = hook::pattern("68 ? ? ? ? 68 AC 27 CF 79");
    if(pattern.empty())
    {
        uintptr_t strAddr = (uintptr_t)searchStringPattern("SET_TIME_ONE_DAY_FORWARD").get_first(0);
        char patternStr[32]{};
        sprintf(patternStr, "68 ? ? ? ? 68 %02X %02X %02X %02X", uint8_t(strAddr), uint8_t(strAddr >> 8), uint8_t(strAddr >> 16), uint8_t(strAddr >> 24));
        pattern = hook::pattern(patternStr);
    }
    SetTimeOneDayForward = *(decltype(SetTimeOneDayForward)*)pattern.get_first(1);

    pattern = hook::pattern("68 ? ? ? ? 68 17 62 13 18");
    if(pattern.empty())
    {
        uintptr_t strAddr = (uintptr_t)searchStringPattern("SET_TIME_ONE_DAY_BACK").get_first(0);
        char patternStr[32]{};
        sprintf(patternStr, "68 ? ? ? ? 68 %02X %02X %02X %02X", uint8_t(strAddr), uint8_t(strAddr >> 8), uint8_t(strAddr >> 16), uint8_t(strAddr >> 24));
        pattern = hook::pattern(patternStr);
    }
    SetTimeOneDayBack = *(decltype(SetTimeOneDayBack)*)pattern.get_first(1);

    TimecycModifierEditor::Init();

    LoadSettings();
    TimeCycle::Load("pc/data/timecyc.dat", nullptr, 0);
    ReSetFloatColors();

    mTimecycParamNameOffsetAndType[0] = {"Ambient Color 0", 0x0, TIMECYCPARAMTYPE_COLOR_U32};
    mTimecycParamNameOffsetAndType[1] = {"Ambient Color 1", 0x4, TIMECYCPARAMTYPE_COLOR_U32};
    mTimecycParamNameOffsetAndType[2] = {"Directional Light Color 1", 0x8, TIMECYCPARAMTYPE_COLOR_U32};
    mTimecycParamNameOffsetAndType[3] = {"Directional Light Color Multiplier", 0xC, TIMECYCPARAMTYPE_FLOAT};
    mTimecycParamNameOffsetAndType[4] = {"Ambient Color 0 Multiplier", 0x10, TIMECYCPARAMTYPE_FLOAT};
    mTimecycParamNameOffsetAndType[5] = {"Ambient Color 1 Multiplier", 0x14, TIMECYCPARAMTYPE_FLOAT};
    mTimecycParamNameOffsetAndType[6] = {"AO Strength", 0x18, TIMECYCPARAMTYPE_FLOAT};
    mTimecycParamNameOffsetAndType[7] = {"Ped AO Strength", 0x1C, TIMECYCPARAMTYPE_FLOAT};
    mTimecycParamNameOffsetAndType[8] = {"Rim Lighting Multiplier", 0x20, TIMECYCPARAMTYPE_FLOAT};
    mTimecycParamNameOffsetAndType[9] = {"Sky Light Multiplier", 0x24, TIMECYCPARAMTYPE_FLOAT};
    mTimecycParamNameOffsetAndType[10] = {"Fog Color and Density", 0x2C, TIMECYCPARAMTYPE_COLOR_U32};
    mTimecycParamNameOffsetAndType[11] = {"Sun Core", 0x30, TIMECYCPARAMTYPE_COLOR_U32};
    mTimecycParamNameOffsetAndType[12] = {"Corona Brightness", 0x34, TIMECYCPARAMTYPE_FLOAT};
    mTimecycParamNameOffsetAndType[13] = {"Corona Size", 0x3C, TIMECYCPARAMTYPE_FLOAT};
    mTimecycParamNameOffsetAndType[14] = {"Distant Corona Brightness", 0x40, TIMECYCPARAMTYPE_FLOAT};
    mTimecycParamNameOffsetAndType[15] = {"Far Clip", 0x44, TIMECYCPARAMTYPE_FLOAT};
    mTimecycParamNameOffsetAndType[16] = {"Fog Start", 0x48, TIMECYCPARAMTYPE_FLOAT};
    mTimecycParamNameOffsetAndType[17] = {"DOF Start", 0x4C, TIMECYCPARAMTYPE_FLOAT};
    mTimecycParamNameOffsetAndType[18] = {"Near DOF Blur", 0x50, TIMECYCPARAMTYPE_FLOAT};
    mTimecycParamNameOffsetAndType[19] = {"Far DOF Blur", 0x54, TIMECYCPARAMTYPE_FLOAT};
    mTimecycParamNameOffsetAndType[20] = {"Far DOF Blur", 0x54, TIMECYCPARAMTYPE_FLOAT};
    mTimecycParamNameOffsetAndType[21] = {"Low Clouds Color", 0x58, TIMECYCPARAMTYPE_COLOR_U32};
    mTimecycParamNameOffsetAndType[22] = {"Bottom Clouds Color", 0x5C, TIMECYCPARAMTYPE_COLOR_U32};
    mTimecycParamNameOffsetAndType[23] = {"Water", 0x60, TIMECYCPARAMTYPE_COLOR_U32};
    mTimecycParamNameOffsetAndType[24] = {"Water Reflection Multiplier", 0x80, TIMECYCPARAMTYPE_FLOAT};
    mTimecycParamNameOffsetAndType[25] = {"Particle Brightness", 0x84, TIMECYCPARAMTYPE_FLOAT};
    mTimecycParamNameOffsetAndType[26] = {"Exposure", 0x88, TIMECYCPARAMTYPE_FLOAT};
    mTimecycParamNameOffsetAndType[27] = {"Bloom Threshold", 0x8C, TIMECYCPARAMTYPE_FLOAT};
    mTimecycParamNameOffsetAndType[28] = {"Mid Gray Value", 0x90, TIMECYCPARAMTYPE_FLOAT};
    mTimecycParamNameOffsetAndType[29] = {"Bloom Intensity", 0x94, TIMECYCPARAMTYPE_FLOAT};
    mTimecycParamNameOffsetAndType[30] = {"Color Correction", 0x98, TIMECYCPARAMTYPE_COLOR_U32};
    mTimecycParamNameOffsetAndType[31] = {"Color Add", 0x9C, TIMECYCPARAMTYPE_COLOR_U32};
    mTimecycParamNameOffsetAndType[32] = {"Desaturation", 0xA0, TIMECYCPARAMTYPE_FLOAT};
    mTimecycParamNameOffsetAndType[33] = {"Contrast", 0xA4, TIMECYCPARAMTYPE_FLOAT};
    mTimecycParamNameOffsetAndType[34] = {"Gamma", 0xA8, TIMECYCPARAMTYPE_FLOAT};
    mTimecycParamNameOffsetAndType[35] = {"Desaturation Far", 0xAC, TIMECYCPARAMTYPE_FLOAT};
    mTimecycParamNameOffsetAndType[36] = {"Contrast Far", 0xB0, TIMECYCPARAMTYPE_FLOAT};
    mTimecycParamNameOffsetAndType[37] = {"Gamma Far", 0xB4, TIMECYCPARAMTYPE_FLOAT};
    mTimecycParamNameOffsetAndType[38] = {"DepthFX Near", 0xB8, TIMECYCPARAMTYPE_FLOAT};
    mTimecycParamNameOffsetAndType[39] = {"DepthFX Far", 0xBC, TIMECYCPARAMTYPE_FLOAT};
    mTimecycParamNameOffsetAndType[40] = {"Luminance Min", 0xC0, TIMECYCPARAMTYPE_FLOAT};
    mTimecycParamNameOffsetAndType[41] = {"Luminance Max", 0xC4, TIMECYCPARAMTYPE_FLOAT};
    mTimecycParamNameOffsetAndType[42] = {"Luminance Delay", 0xC8, TIMECYCPARAMTYPE_FLOAT};
    mTimecycParamNameOffsetAndType[43] = {"Cloud Alpha", 0xCC, TIMECYCPARAMTYPE_INT};
    mTimecycParamNameOffsetAndType[44] = {"Temperature", 0xD4, TIMECYCPARAMTYPE_FLOAT};
    mTimecycParamNameOffsetAndType[45] = {"Global Reflection Multiplier", 0xD8, TIMECYCPARAMTYPE_FLOAT};
    mTimecycParamNameOffsetAndType[46] = {"Sky Color", 0xE4, TIMECYCPARAMTYPE_COLOR_FLOAT3};
    mTimecycParamNameOffsetAndType[47] = {"Sky Horizon Color", 0xF4, TIMECYCPARAMTYPE_COLOR_FLOAT3};
    mTimecycParamNameOffsetAndType[48] = {"Sky East Horizon Color", 0x104, TIMECYCPARAMTYPE_COLOR_FLOAT3};
    mTimecycParamNameOffsetAndType[49] = {"Cloud 1 Color", 0x114, TIMECYCPARAMTYPE_COLOR_FLOAT3};
    mTimecycParamNameOffsetAndType[50] = {"Sky Horizon Height", 0x120, TIMECYCPARAMTYPE_FLOAT};
    mTimecycParamNameOffsetAndType[51] = {"Sky Horizon Brightness", 0x124, TIMECYCPARAMTYPE_FLOAT};
    mTimecycParamNameOffsetAndType[52] = {"Cloud 2 Color", 0x134, TIMECYCPARAMTYPE_COLOR_FLOAT3};
    mTimecycParamNameOffsetAndType[53] = {"Cloud 2 Shadow Strength", 0x140, TIMECYCPARAMTYPE_FLOAT};
    mTimecycParamNameOffsetAndType[54] = {"Cloud 2 Threshold", 0x144, TIMECYCPARAMTYPE_FLOAT};
    mTimecycParamNameOffsetAndType[55] = {"Cloud 2 Bias 1", 0x148, TIMECYCPARAMTYPE_FLOAT};
    mTimecycParamNameOffsetAndType[56] = {"Cloud 2 Scale", 0x14C, TIMECYCPARAMTYPE_FLOAT};
    mTimecycParamNameOffsetAndType[57] = {"Cloud In Scattering", 0x150, TIMECYCPARAMTYPE_FLOAT};
    mTimecycParamNameOffsetAndType[58] = {"Cloud 2 Bias 2", 0x154, TIMECYCPARAMTYPE_FLOAT};
    mTimecycParamNameOffsetAndType[59] = {"Detail Noise Scale", 0x158, TIMECYCPARAMTYPE_FLOAT};
    mTimecycParamNameOffsetAndType[60] = {"Detail Noise Multiplier", 0x15C, TIMECYCPARAMTYPE_FLOAT};
    mTimecycParamNameOffsetAndType[61] = {"Cloud 2 Offset", 0x160, TIMECYCPARAMTYPE_FLOAT};
    mTimecycParamNameOffsetAndType[62] = {"Clouds Fadeout", 0x168, TIMECYCPARAMTYPE_FLOAT};
    mTimecycParamNameOffsetAndType[63] = {"Cloud 1 Bias", 0x16C, TIMECYCPARAMTYPE_FLOAT};
    mTimecycParamNameOffsetAndType[64] = {"Cloud 1 Detail", 0x170, TIMECYCPARAMTYPE_FLOAT};
    mTimecycParamNameOffsetAndType[65] = {"Cloud 1 Threshold", 0x174, TIMECYCPARAMTYPE_FLOAT};
    mTimecycParamNameOffsetAndType[66] = {"Cloud 1 Height", 0x178, TIMECYCPARAMTYPE_FLOAT};
    mTimecycParamNameOffsetAndType[67] = {"Cloud 3 Color", 0x184, TIMECYCPARAMTYPE_COLOR_FLOAT3};
    mTimecycParamNameOffsetAndType[68] = {"Sun Color", 0x1A4, TIMECYCPARAMTYPE_COLOR_FLOAT3};
    mTimecycParamNameOffsetAndType[69] = {"Clouds Brightness", 0x1B0, TIMECYCPARAMTYPE_FLOAT};
    mTimecycParamNameOffsetAndType[70] = {"Detail Noise Offset", 0x1B4, TIMECYCPARAMTYPE_FLOAT};
    mTimecycParamNameOffsetAndType[71] = {"Stars Brightness", 0x1B8, TIMECYCPARAMTYPE_FLOAT};
    mTimecycParamNameOffsetAndType[72] = {"Visible Stars", 0x1BC, TIMECYCPARAMTYPE_FLOAT};
    mTimecycParamNameOffsetAndType[73] = {"Moon Brightness", 0x1C0, TIMECYCPARAMTYPE_FLOAT};
    mTimecycParamNameOffsetAndType[74] = {"Moon Color", 0x1D4, TIMECYCPARAMTYPE_COLOR_FLOAT3};
    mTimecycParamNameOffsetAndType[75] = {"Moon Glow", 0x1E0, TIMECYCPARAMTYPE_FLOAT};
    mTimecycParamNameOffsetAndType[76] = {"Moon Transparency", 0x1E4, TIMECYCPARAMTYPE_FLOAT};
    mTimecycParamNameOffsetAndType[77] = {"Sun Size", 0x1F0, TIMECYCPARAMTYPE_FLOAT};
    mTimecycParamNameOffsetAndType[78] = {"Sky Brightness", 0x204, TIMECYCPARAMTYPE_FLOAT};
    mTimecycParamNameOffsetAndType[79] = {"Film Grain", 0x20C, TIMECYCPARAMTYPE_INT};
}

void TimecycEditor::ReSetFloatColors()
{
    for(uint32_t time = 0; time < TimeCycle::NUM_HOURS; time++)
    {
        for(uint32_t weather = 0; weather < TimeCycle::NUM_WEATHERS; weather++)
        {
            sColorsAsFloat[AMBIENT_LIGHT_0_COLOR_ID][time * TimeCycle::NUM_WEATHERS + weather]   = ColorfFromColor32(TimeCycle::m_ColourSets[time][weather].m_Ambient0Color);
            sColorsAsFloat[AMBIENT_LIGHT_1_COLOR_ID][time * TimeCycle::NUM_WEATHERS + weather]   = ColorfFromColor32(TimeCycle::m_ColourSets[time][weather].m_Ambient1Color);
            sColorsAsFloat[DIRECTIONAL_LIGHT_COLOR_ID][time * TimeCycle::NUM_WEATHERS + weather] = ColorfFromColor32(TimeCycle::m_ColourSets[time][weather].m_DirLightColor);
            sColorsAsFloat[FOG_COLOR_ID][time * TimeCycle::NUM_WEATHERS + weather]               = ColorfFromColor32(TimeCycle::m_ColourSets[time][weather].m_FogColorDensity);
            sColorsAsFloat[COLOR_CORRECT_ID][time * TimeCycle::NUM_WEATHERS + weather]           = ColorfFromColor32(TimeCycle::m_ColourSets[time][weather].m_ColorCorrection);
            sColorsAsFloat[COLOR_ADD_ID][time * TimeCycle::NUM_WEATHERS + weather]               = ColorfFromColor32(TimeCycle::m_ColourSets[time][weather].m_ColorAdd);
            sColorsAsFloat[WATER_COLOR_ID][time * TimeCycle::NUM_WEATHERS + weather]             = ColorfFromColor32(TimeCycle::m_ColourSets[time][weather].m_Water);
        }
    }
}

void TimecycEditor::ReCreateFont()
{
    ImFontConfig conf = {};
    conf.SizePixels = 13.0f * mFontScale;
    conf.OversampleH = 2;
    conf.OversampleV = 2;

    static const ImWchar ranges[] =
    {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0,
    };
    const ImWchar* glyph_ranges = ranges;

    auto& io = ImGui::GetIO();
    io.Fonts->Clear();
    io.Fonts->AddFontFromMemoryCompressedBase85TTF(gCousineRegularCompressedDataBase85, conf.SizePixels, &conf, glyph_ranges);
    io.Fonts->Build();

    ImGui_ImplDX9_InvalidateDeviceObjects();
    ImGui_ImplDX9_CreateDeviceObjects();
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
        
        io.IniFilename = nullptr;
        
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

            style->SeparatorTextBorderSize = 3;

            style->TabRounding = 0;
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

        ReCreateFont();

        mIsImGuiInitialized = true;
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
    //file.write((char*)&mItemInnerSpacing, sizeof(float));
    file.write((char*)&mLockTimeAndWeatherIfClosed, sizeof(bool));
}

void TimecycEditor::LoadSettings()
{
    std::ifstream file("InGameTimecycSettings.bin", std::ios::binary);

    if(!file.good())
    {
        return;
    }
    
    file.read((char*)&mSettingsFileMajorVersion, sizeof(char));
    file.read((char*)&mSettingsFileMinorVersion, sizeof(char));
    file.read((char*)&mWindowPos.x, sizeof(float));
    file.read((char*)&mWindowPos.y, sizeof(float));
    file.read((char*)&mWindowSize.x, sizeof(float));
    file.read((char*)&mWindowSize.y, sizeof(float));
    file.read((char*)&mFontScale, sizeof(float));
    file.read((char*)&mOpenWindowKey, sizeof(ImGuiKey));
    file.read((char*)&mToggleCameraControlKey, sizeof(ImGuiKey));
    //file.read((char*)&mItemInnerSpacing, sizeof(float));

    if(mSettingsFileMajorVersion == 1 && mSettingsFileMinorVersion == 3)
    {
        file.read((char*)&mLockTimeAndWeatherIfClosed, sizeof(bool));
    }

    if(mIsImGuiInitialized)
    {
        ImGui::GetIO().FontGlobalScale = mFontScale;
        //ImGui::GetStyle().ItemInnerSpacing.x = 4.0f;
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
        if(mToggleCameraControlKey != ImGuiKey_None && ImGui::IsKeyPressed(mToggleCameraControlKey))
        {
            mDisableMouseControl = !mDisableMouseControl;
        }

        if(ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Z))
        {
            if(sUndoStack.empty())
                return;
            auto action = sUndoStack.top();
            sUndoStack.pop();
            action.Undo();
            sRedoStack.push(action);
        }
        if(ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Y))
        {
            if(sRedoStack.empty())
                return;
            auto action = sRedoStack.top();
            sRedoStack.pop();
            action.Redo();
            sUndoStack.push(action);
        }

        static float prevFontScale = mFontScale;
        if(mFontScale != prevFontScale)
        {
            ReCreateFont();
            prevFontScale = mFontScale;
        }
    }
    
    if((mLockTimeAndWeather && mShowWindow) || (mLockTimeAndWeather && mLockTimeAndWeatherIfClosed))
    {
        *mHours = mSelectedHour;
        *mMinutes = mSelectedMinutes;

        ForceWeather(mSelectedWeather);
    }

    bool windowWasJustClosed = prevShowWindow && !mShowWindow;
    if(windowWasJustClosed && !mLockTimeAndWeatherIfClosed)
    {
        ReleaseWeather();
        *mTimerLength = DEFAULT_MILLISECONDS_PER_GAME_MINUTE;
    }

    bool windowWasJustOpened = !prevShowWindow && mShowWindow;
    if(windowWasJustOpened && (mLockTimeAndWeather || (mLockTimeAndWeather && mLockTimeAndWeatherIfClosed)))
    {
        *mTimerLength = LOCKED_MILLISECONDS_PER_GAME_MINUTE;
    }

    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    if(mShowWindow)
    {
        DrawMainWindow();
        //ImGui::ShowDemoWindow();
        DrawSaveWindow();
        DrawLoadWindow();
        DrawSettingsWindow();
        DrawSetParamOnAllHoursAndWeathersWindow();
        DrawLicensesWindow();
    }

    ImGui::EndFrame();
    ImGui::Render();
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
    ImDrawData* drawData = ImGui::GetDrawData();
    if(!drawData)
        return;

    InitializeImGui(d3d9Device);

    ImGui_ImplDX9_RenderDrawData(drawData);
}

void TimecycEditor::DrawMainWindow()
{
    const char* timeOfDayNames[TimeCycle::NUM_HOURS] = {"Midnight", "5AM", "6AM", "7AM", "9AM", "Midday", "6PM", "7PM", "8PM", "9PM", "10PM"};
    const char* timeOfDayName = timeOfDayNames[mSelectedHourIndex];
    const char* weatherNames[TimeCycle::NUM_WEATHERS - 1] = {"EXTRASUNNY", "SUNNY", "SUNNY_WINDY", "CLOUDY", "RAIN", "DRIZZLE", "FOGGY", "LIGHTNING"};
    const char* weatherName = weatherNames[mSelectedWeather];

    ImGui::Begin("TimeCycle Editor 1.3", nullptr, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus);
    ImGui::SetWindowPos(mWindowPos);
    ImGui::SetWindowSize(mWindowSize);

    if(ImGui::BeginMenuBar())
    {
        if(ImGui::BeginMenu("Load"))
        {
            if(ImGui::MenuItem("timecyc.dat##Load"))
            {
                TimeCycle::Load("pc/data/timecyc.dat", nullptr, 0);
                ReSetFloatColors();
            }

            if(ImGui::MenuItem("Load From"))
            {
                mShowLoadWindow = true;
            }

            if(ImGui::MenuItem("Load Modifiers"))
            {
                TimecycModifierEditor::Load();
            }

            ImGui::EndMenu();
        }

        if(ImGui::BeginMenu("Save"))
        {
            if(ImGui::MenuItem("timecyc.dat##Save"))
            {
                TimeCycle::Save("pc/data/timecyc.dat", nullptr, 0);
            }

            if(ImGui::MenuItem("Save As"))
            {
                mShowSaveWindow = true;
            }

            if(ImGui::MenuItem("Save Modifiers"))
            {
                TimecycModifierEditor::Save(nullptr, 0);
            }

            ImGui::EndMenu();
        }

        if(ImGui::BeginMenu("Settings"))
        {
            mShowSettingsWindow = true;
            ImGui::EndMenu();
        }

        if(ImGui::BeginMenu("Other"))
        {
            if(ImGui::MenuItem("Set Param On All Hours And Weathers"))
            {
                mShowSetParamOnAllHoursAndWeathersWindow = true;
            }

            if(ImGui::MenuItem("Licenses"))
            {
                mShowLicensesWindow = true;
            }

            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    ImGui::PushStyleVarX(ImGuiStyleVar_FramePadding, 20.0f);
    if(ImGui::BeginTabBar("TimeCycle Tab Bar"))
    {
        if(ImGui::BeginTabItem("TimeCycle"))
        {
            ImGui::PopStyleVar();

            ImGui::Checkbox("Lock to Selected Time and Weather", &mLockTimeAndWeather);
            if(ImGui::IsItemEdited())
            {
                if(mLockTimeAndWeather)
                {
                    *mTimerLength = LOCKED_MILLISECONDS_PER_GAME_MINUTE;
                }
                else
                {
                    ReleaseWeather();
                    *mTimerLength = DEFAULT_MILLISECONDS_PER_GAME_MINUTE;
                }
            }

            ImGui::Checkbox("Only Show TimeCycle Times", &mOnlyTimecycTimes);
    
            ImGui::NewLine();

            static float buttonWidth = 0.0f;

            ImGui::Text("Day");

            ImGui::SameLine();
            ImGui::TextDisabled("(?)");
            if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort) && ImGui::BeginTooltip())
            {
                ImGui::TextUnformatted("Affects clouds.");
                ImGui::EndTooltip();
            }

            if(ImGui::Button("One Day Forward", ImVec2(buttonWidth - 6.0f, 0)))
            {
                SetTimeOneDayForward();
            }

            ImGui::SameLine();

            if(ImGui::Button("One Day Back", ImVec2(buttonWidth, 0)))
            {
                SetTimeOneDayBack();
            }

            if(mOnlyTimecycTimes)
            {
                mSelectedHour = TimecycTimeIndexToGameTime(mSelectedHourIndex);

                ImGui::Text("Time of Day");
                ImGui::SliderInt("##Hour", &mSelectedHourIndex, 0, TimeCycle::NUM_HOURS - 1, timeOfDayName, ImGuiSliderFlags_NoInput);

                buttonWidth = ImGui::GetItemRectSize().x * 0.5f;
            }
            else
            {
                mSelectedHourIndex = GameTimeToTimecycTimeIndex(mSelectedHour);

                std::stringstream label;
                label << "Time of Day (Editing " << timeOfDayNames[mSelectedHourIndex] << ")";

                ImGui::Text(label.str().c_str());
                ImGui::SliderInt("##Hour", &mSelectedHour, 0, 23);

                buttonWidth = ImGui::GetItemRectSize().x * 0.5f;
            }

            ImGui::SliderInt("##Minutes", &mSelectedMinutes, 0, 59, "%d");

            ImGui::Text("Weather");
            ImGui::SliderInt("##Weather", &mSelectedWeather, 0, TimeCycle::NUM_WEATHERS - 2, weatherName, ImGuiSliderFlags_NoInput);

            ImGui::NewLine();

            auto ColorEdit3 = [this](const char* label, Color32& color, ImGuiColorEditFlags flags = 0)
            {
                static std::unordered_map<std::string, Color32> sPrevValues;

                std::string uniqueLabel = std::string(label) + "_" + std::to_string(mSelectedHourIndex) + "_" + std::to_string(mSelectedWeather);

                Colorf* colorf = &sColorsAsFloat[label][mSelectedHourIndex * TimeCycle::NUM_WEATHERS + mSelectedWeather];
                ImGui::ColorEdit3(uniqueLabel.c_str(), &colorf->Red, flags);

                if(ImGui::IsItemActivated())
                    sPrevValues[uniqueLabel] = color;

                uint8_t alpha = color.Alpha;
                color = Color32FromColorf(*colorf);
                color.Alpha = alpha;

                if(ImGui::IsItemDeactivatedAfterEdit())
                {
                    Color32 prevValue = sPrevValues[uniqueLabel];
                    Color32 newValue = color;
                    PushUndo([&color, colorf, prevValue]() 
                             { 
                                 uint8_t alpha = color.Alpha;
                                 color = prevValue;
                                 *colorf = ColorfFromColor32(color); 
                                 color.Alpha = alpha;
                             },
                             [&color, colorf, newValue]() 
                             {
                                 uint8_t alpha = color.Alpha;
                                 color = newValue;
                                 *colorf = ColorfFromColor32(color); 
                                 color.Alpha = alpha;
                             });
                }
            };
    
            auto ColorEdit3f = [this](const char* label, Color3f& color, ImGuiColorEditFlags flags = 0)
            {
                static std::unordered_map<std::string, Color3f> sPrevValues;

                std::string uniqueLabel = std::string(label) + "_" + std::to_string(mSelectedHourIndex) + "_" + std::to_string(mSelectedWeather);

                Color3f c = color;
                ImGui::ColorEdit3(uniqueLabel.c_str(), &c.Red, flags);

                if(ImGui::IsItemActivated())
                    sPrevValues[uniqueLabel] = color;

                color = c;

                if(ImGui::IsItemDeactivatedAfterEdit())
                {
                    Color3f prevValue = sPrevValues[uniqueLabel];
                    Color3f newValue = color;
                    PushUndo([&color, prevValue]() { color = prevValue; },
                             [&color, newValue]()  { color = newValue; });
                }
            };

            auto ColorEdit4 = [this](const char* label, Color32& color, ImGuiColorEditFlags flags = 0)
            {
                static std::unordered_map<std::string, Color32> sPrevValues;

                std::string uniqueLabel = std::string(label) + "_" + std::to_string(mSelectedHourIndex) + "_" + std::to_string(mSelectedWeather);

                Colorf* colorf = &sColorsAsFloat[label][mSelectedHourIndex * TimeCycle::NUM_WEATHERS + mSelectedWeather];
                ImGui::ColorEdit4(uniqueLabel.c_str(), &colorf->Red, flags);

                if(ImGui::IsItemActivated())
                    sPrevValues[uniqueLabel] = color;

                color = Color32FromColorf(*colorf);

                if(ImGui::IsItemDeactivatedAfterEdit())
                {
                    Color32 prevValue = sPrevValues[uniqueLabel];
                    Color32 newValue = color;
                    PushUndo([&color, colorf, prevValue]() 
                             { 
                                 color = prevValue;
                                 *colorf = ColorfFromColor32(color); 
                             },
                             [&color, colorf, newValue]() 
                             {
                                 color = newValue;
                                 *colorf = ColorfFromColor32(color); 
                             });
                }
            };

            auto DragFloat = [this](const char* label, float* value, float speed = 1.0f, float min = 0.0f, float max = 0.0f)
            {
                static std::unordered_map<std::string, float> sPrevValues;

                std::string uniqueLabel = std::string(label) + "_" + std::to_string(mSelectedHourIndex) + "_" + std::to_string(mSelectedWeather);

                ImGui::DragFloat(uniqueLabel.c_str(), value, speed, min, max);

                if(ImGui::IsItemActivated())
                    sPrevValues[uniqueLabel] = *value;

                if(ImGui::IsItemDeactivatedAfterEdit())
                {
                    float prevValue = sPrevValues[uniqueLabel];
                    float newValue = *value;
                    PushUndo([value, prevValue]() { *value = prevValue; },
                             [value, newValue]()  { *value = newValue; });
                }
            };

            auto DragInt = [this](const char* label, int* value, float speed = 1.0f, int min = 0.0f, int max = 0.0f)
            {
                static std::unordered_map<std::string, int> sPrevValues;

                std::string uniqueLabel = std::string(label) + "_" + std::to_string(mSelectedHourIndex) + "_" + std::to_string(mSelectedWeather);

                ImGui::DragInt(uniqueLabel.c_str(), value, speed, min, max);

                if(ImGui::IsItemActivated())
                    sPrevValues[uniqueLabel] = *value;

                if(ImGui::IsItemDeactivatedAfterEdit())
                {
                    int prevValue = sPrevValues[uniqueLabel];
                    int newValue = *value;
                    PushUndo([value, prevValue]() { *value = prevValue; },
                             [value, newValue]()  { *value = newValue; });
                }
            };

            if(ImGui::CollapsingHeader("Lighting"))
            {
                ImGui::SeparatorText("Ambient Light 0");
                {
                    ImGui::Text("Color");
                    ColorEdit3(AMBIENT_LIGHT_0_COLOR_ID, TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_Ambient0Color);

                    ImGui::Text("Multiplier");
                    DragFloat("##Ambient Light 0 Multiplier", &TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_fAmbient0Multiplier, 0.005f, 0.0f, FLT_MAX);
                }
                ImGui::SeparatorText("Ambient Light 1");
                {
                    ImGui::Text("Color");
                    ColorEdit3(AMBIENT_LIGHT_1_COLOR_ID, TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_Ambient1Color);

                    ImGui::Text("Multiplier");
                    DragFloat("##Ambient Light 1 Multiplier", &TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_fAmbient1Multiplier, 0.005f, 0.0f, FLT_MAX);
                }
                ImGui::SeparatorText("Directional Light");
                {
                    ImGui::Text("Color");
                    ColorEdit3(DIRECTIONAL_LIGHT_COLOR_ID, TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_DirLightColor);

                    ImGui::Text("Multiplier");
                    DragFloat("##Directional Light multiplier", &TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_fDirLightMultiplier, 0.005f, 0.0f, FLT_MAX);

                    ImGui::Text("Specular Multiplier");
                    DragFloat("##Directional Light Specular Multiplier", &TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_fDirLightSpecMultiplier, 0.005f, 0.0f, FLT_MAX);
                }
                ImGui::SeparatorText("Water");
                {
                    ImGui::Text("Color");
                    ColorEdit4(WATER_COLOR_ID, TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_Water);

                    ImGui::Text("Reflection Multiplier");
                    DragFloat("##Water Reflection Multiplier", &TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_fWaterReflectionMultiplier, 0.005f, 0.0f, FLT_MAX);
                }
                ImGui::SeparatorText("");

                ImGui::Text("Rim Lighting Multiplier");
                DragFloat("##Rim Lighting Multiplier", &TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_fRimLightingMultiplier, 0.005f, 0.0f, FLT_MAX);

                ImGui::Text("Sky Light Multiplier");
                DragFloat("##Sky Light Multiplier", &TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_fSkyLightMultiplier, 0.005f, 0.0f, FLT_MAX);

                ImGui::Text("Global Reflection Multiplier");
                DragFloat("##Global Reflection Multiplier", &TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_fGlobalReflectionMultiplier, 0.005f, 0.0f, FLT_MAX);

                ImGui::Text("AO Strength");
                DragFloat("##AO Strength", &TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_fAOStrength, 0.005f, 0.0f, FLT_MAX);

                ImGui::Text("Ped AO Strength");
                DragFloat("##Ped AO Strength", &TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_fPedAOStrength, 0.005f, 0.0f, FLT_MAX);

                ImGui::Text("Corona Size");
                DragFloat("##Corona Size", &TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_fCoronaSize, 0.005f, 0.0f, FLT_MAX);

                ImGui::Text("Corona Brightness");
                DragFloat("##Corona Brightness", &TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_fCoronaBrightness, 0.005f, 0.0f, FLT_MAX);

                ImGui::Text("Distant Corona Size");
                DragFloat("##Distant Corona Size", &TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_fDistantCoronaSize, 0.05f, 0.0f, FLT_MAX);

                ImGui::Text("Distant Corona Brightness");
                DragFloat("##Distant Corona Brightness", &TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_fDistantCoronaBrightness, 0.05f, 0.0f, FLT_MAX);

                ImGui::Text("Particle Brightness");
                DragFloat("##Particle Brightness", &TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_fParticleBrightness, 0.005f, 0.0f, FLT_MAX);
            }
    
            if(ImGui::CollapsingHeader("Atmosphere"))
            {
                ImGui::Text("Temperature");
                DragFloat("##Temperature", &TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_fTemperature, 0.1f, -40.0f, 50.0f);

                if(ImGui::TreeNode("Fog"))
                {
                    ImGui::Text("Start");
                    DragFloat("##Fog Start", &TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_fFogStart, 0.5f);

                    ImGui::Text("Color");
                    ColorEdit3(FOG_COLOR_ID, TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_FogColorDensity);

                    {
                        static std::unordered_map<std::string, uint8_t> sPrevValues;

                        uint8_t* density = (uint8_t*)&TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_FogColorDensity.BGRA;
                        density = &density[3];
                        uint32_t min = 0, max = 255;

                        std::string uniqueLabel = std::string("##Fog Density_") + std::to_string(mSelectedHourIndex) + "_" + std::to_string(mSelectedWeather);

                        ImGui::Text("Density");
                        ImGui::DragScalar(uniqueLabel.c_str(), ImGuiDataType_U8, density, 0.15f, &min, &max);

                        if(ImGui::IsItemActivated())
                            sPrevValues[uniqueLabel] = *density;

                        if(ImGui::IsItemDeactivatedAfterEdit())
                        {
                            uint8_t prevValue = sPrevValues[uniqueLabel];
                            uint8_t newValue = *density;
                            PushUndo([density, prevValue]() { *density = prevValue; },
                                     [density, newValue]()  { *density = newValue; });
                        }
                    }

                    ImGui::TreePop();
                }

                if(ImGui::TreeNode("Sky"))
                {
                    ImGui::Text("Brightness");
                    DragFloat("##Sky Brightness", &TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_fSkyBrightness, 0.005f, 0.0f, FLT_MAX);

                    ImGui::Text("Color");
                    ColorEdit3f("##Sky Color", TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_SkyHatSettings.m_SkyColor, ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float);

                    ImGui::Text("East Horizon Color");
                    ColorEdit3f("##Sky East Horizon Color", TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_SkyHatSettings.m_SkyHorizonColor, ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float);

                    ImGui::Text("West Horizon Color");
                    ColorEdit3f("##Sky West Horizon Color", TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_SkyHatSettings.m_SkyEastHorizonColor, ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float);

                    ImGui::Text("Horizon Brightness");
                    DragFloat("##Sky East Horizon Brightness", &TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_SkyHatSettings.m_fSkyHorizonBrightness, 0.005f, 0.0f, FLT_MAX);

                    ImGui::Text("Horizon Height Fade Out");
                    DragFloat("##Sky Horizon Height Fade Out", &TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_SkyHatSettings.m_fSkyHorizonHeight, 0.05f, 0.0f, FLT_MAX);

                    ImGui::SeparatorText("Sun");

                    ImGui::Text("Color");
                    ColorEdit3f("##Sun Color", TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_SkyHatSettings.m_SunColor, ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float);
            
                    ImGui::Text("Brightness");
                    DragFloat("##Sun Brightness", &TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_SkyHatSettings.m_fSunBrightness, 0.0005f, 0.0f, FLT_MAX);

                    ImGui::Text("Glow Transparency");
                    DragFloat("##Glow Transparency", &TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_fSunGlowTransparency, 0.001f, 0.0f, FLT_MAX);

                    ImGui::TreePop();
                }

                if(ImGui::TreeNode("Clouds"))
                {
                    ImGui::Text("Sunset Color");
                    ColorEdit3f("##Cloud Sunset Color", TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_SkyHatSettings.m_SunsetColor, ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float);

                    ImGui::Text("Global Color");
                    ColorEdit3f("##Cloud Global Color 2", TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_SkyHatSettings.m_CloudColor, ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float);

                    ImGui::Text("Layer 2 Color");
                    ColorEdit3f("##Cloud Layer 2 Color", TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_SkyHatSettings.m_TopCloudColor, ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float);

                    ImGui::Text("Underlighting");
                    DragFloat("##Cloud Underlighting", &TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_SkyHatSettings.m_fUnderlighting, 0.05f, 0.0f, FLT_MAX);

                    ImGui::Text("Height Fade Out");
                    DragFloat("##Clouds Fade Out", &TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_SkyHatSettings.m_fCloudsFadeOut, 0.025f, 0.0f, FLT_MAX);

                    ImGui::SeparatorText("Detail Noise");
                    {
                        ImGui::Text("Scale");
                        DragFloat("##Detail Noise Scale", &TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_SkyHatSettings.m_fDetailNoiseScale, 0.5f, 0.0f, FLT_MAX);

                        ImGui::Text("Offset/Scrolling Speed");
                        DragFloat("##Detail Noise Offset", &TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_SkyHatSettings.m_fDetailNoiseOffset, 0.05f);

                        ImGui::Text("Multiplier");
                        DragFloat("##Detail Noise Multiplier", &TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_SkyHatSettings.m_fDetailNoiseMultiplier, 0.005f, FLT_MIN, FLT_MAX);
                    }

                    ImGui::SeparatorText("Layer 2");
                    {
                        ImGui::Text("Threshold");
                        DragFloat("##Clouds 1 Threshold", &TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_SkyHatSettings.m_fCloud1Threshold, 0.005f, 0.0f, FLT_MAX);

                        ImGui::Text("Bias");
                        DragFloat("##Clouds 1 Bias", &TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_SkyHatSettings.m_fCloud1Bias, 0.005f,	0.0f, FLT_MAX);

                        ImGui::Text("Detail");
                        DragFloat("##Clouds 1 Detail", &TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_SkyHatSettings.m_fCloud1Detail, 0.005f, 0.0f, FLT_MAX);

                        ImGui::Text("Height");
                        DragFloat("##Clouds 1 Height", &TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_SkyHatSettings.m_fCloud1Height, 0.005f, 0.0f, FLT_MAX);
                    }

                    ImGui::SeparatorText("Layer 1");
                    {
                        ImGui::Text("Threshold");
                        DragFloat("##Clouds 2 Threshold", &TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_SkyHatSettings.m_fCloud2Threshold, 0.005f, 0.0f, FLT_MAX);

                        ImGui::Text("Bias");
                        DragFloat("##Clouds 2 Bias", &TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_SkyHatSettings.m_fCloud2Bias1, 0.005f, 0.0f, FLT_MAX);

                        ImGui::Text("Edge Smoothing");
                        DragFloat("##Clouds 2 Edge Smooth", &TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_SkyHatSettings.m_fEdgeSmooth, 0.005f, 0.0f, FLT_MAX);

                        ImGui::Text("Thickness");
                        DragFloat("##Clouds 2 Thickness", &TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_SkyHatSettings.m_fCloud2Thickness, 0.005f, 0.0f, FLT_MAX);

                        ImGui::Text("Shadow Offset");
                        DragFloat("##Clouds 2 Shadow Offset", &TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_SkyHatSettings.m_fCloud2ShadowOffset, 0.002f, -5.0f, 5.0f);

                        ImGui::Text("Shadow Strength");
                        DragFloat("##Clouds 2 Shadow Strength", &TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_SkyHatSettings.m_fCloud2ShadowStrength, 0.005f, 0.0f, 1.0f);
                    }

                    ImGui::TreePop();
                }

                if(ImGui::TreeNode("Night Sky"))
                {
                    ImGui::Text("Star Field Threshold");
                    DragFloat("##Star Field Threshold", &TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_SkyHatSettings.m_fStarFieldBrightness, 0.005f, 0.0f, FLT_MAX);

                    ImGui::Text("Star Field Brightness");
                    DragFloat("##Star Field Brightness", &TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_SkyHatSettings.m_fStarFieldThreshold, 0.005f, 0.0f, FLT_MAX);

                    ImGui::SeparatorText("Moon");

                    ImGui::Text("Color");
                    ColorEdit3f("##Moon Color", TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_SkyHatSettings.m_MoonColor, ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float);

                    ImGui::Text("Brightness");
                    DragFloat("##Moon Brightness", &TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_SkyHatSettings.m_fMoonBrightness, 0.001f, 0.0f, FLT_MAX);

                    ImGui::Text("Glow");
                    DragFloat("##Moon Glow", &TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_SkyHatSettings.m_fMoonGlow, 0.005f, 0.0f, FLT_MAX);

                    ImGui::Text("Transparency");
                    DragFloat("##Moon Transparency", &TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_SkyHatSettings.m_fMoonTransparency, 0.015f, 0.0f, 12.0f);

                    ImGui::TreePop();
                }
            }

            if(ImGui::CollapsingHeader("Post Processing"))
            {
                ImGui::Text("Film Grain");
                DragInt("##Film Grain", &TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_FilmGrain, 0.5f, 0, INT32_MAX);

                ImGui::Text("Luminance Min");
                DragFloat("##Luminance Min", &TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_fLumMin, 0.005f, 0.0f, TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_fLumMax);

                ImGui::Text("Luminance Max");
                DragFloat("##Luminance Max", &TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_fLumMax, 0.005f, TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_fLumMin, FLT_MAX);

                ImGui::Text("Luminance Delay");
                DragFloat("##Luminance Delay", &TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_fLumDelay, 0.005f, 0.0f, FLT_MAX);

                ImGui::Text("Exposure");
                DragFloat("##Exposure", &TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_fExposure, 0.001f, 0.0f, FLT_MAX);

                ImGui::Text("Color Correction");
                ColorEdit3(COLOR_CORRECT_ID, TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_ColorCorrection);

                ImGui::Text("Color Add");
                ColorEdit3(COLOR_ADD_ID, TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_ColorAdd);

                ImGui::Text("Bloom Threshold");
                DragFloat("##Bloom Threshold", &TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_fBloomThreshold, 0.005f, 0.0f, FLT_MAX);

                ImGui::Text("Bloom Intensity");
                DragFloat("##Bloom Intensity", &TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_fBloomIntensity, 0.005f, 0.0f, FLT_MAX);

                ImGui::Text("Mid Gray Value");
                DragFloat("##Mid Gray Value", &TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_fMidGrayValue, 0.005f, 0.0f, FLT_MAX);

                ImGui::Text("Desaturation");
                DragFloat("##Desaturation", &TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_fDesaturation, 0.005f, 0.0f, 1.0f);

                ImGui::Text("Desaturation Far");
                DragFloat("##Desaturation Far", &TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_fDesaturationFar, 0.005f, 0.0f, 1.0f);

                ImGui::Text("Gamma");
                DragFloat("##Gamma", &TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_fGamma, 0.005f, 0.0f, FLT_MAX);

                ImGui::Text("Gamma Far");
                DragFloat("##Gamma Far", &TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_fGammaFar, 0.005f, 0.0f, FLT_MAX);

                ImGui::Text("DepthFX Near");
                DragFloat("##DepthFX Near", &TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_fDepthFxNear, 0.5f, 0.0f, TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_fDepthFxFar);

                ImGui::Text("DepthFX Far");
                DragFloat("##DepthFX Far", &TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_fDepthFxFar, 0.5f, TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_fDepthFxNear, FLT_MAX);

                ImGui::Text("DOF Start");
                DragFloat("##DOF Start", &TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_fDOFStart, 0.5f, 0.0f, FLT_MAX);

                ImGui::Text("Near DOF Blur");
                DragFloat("##Near DOF Blur", &TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_fNearDOFBlur, 0.005f, 0.0f, TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_fFarDOFBlur);

                ImGui::Text("Far DOF Blur");
                DragFloat("##Far DOF Blur", &TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_fFarDOFBlur, 0.005f, TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_fNearDOFBlur, 1.0f);
            }

            if(ImGui::CollapsingHeader("Other"))
            {
                ImGui::Text("Far Clip");
                DragFloat("##Far Clip", &TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_fFarClip, 0.5f, 0.0f, FLT_MAX);
            }

            ImGui::EndTabItem();
            /*
            if(ImGui::CollapsingHeader("Likely Unused"))
            {
                ImGui::Text("Contrast Far");
                ImGui::DragFloat("##Contrast Far", &TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_fContrastFar, 0.1f);

                ImGui::Text("Contrast");
                ImGui::DragFloat("##Contrast", &TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_fContrast, 0.1f);

                ImGui::Text("Low Clouds Color");
                ColorEdit3("##Low Clouds Color", TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_LowCloudsColor);

                ImGui::Text("Bottom Clouds Color");
                ColorEdit3("##Bottom Clouds Color", TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_BottomCloudsColor);

                ImGui::Text("Cloud Alpha");
                ImGui::DragInt("##Cloud Alpha", &TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_CloudAlpha);

                ImGui::Text("Sun Core");
                ColorEdit3("##Sun Core", TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_SunCore);

                ImGui::Text("Unknown 3");
                ImGui::DragFloat("##Unknown 3", &TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_SkyHatSettings.m_fUnknown_3C, 0.1f);

                ImGui::Text("Unknown 29");
                ImGui::DragFloat("##Unknown 29", &TimeCycle::m_ColourSets[mSelectedHourIndex][mSelectedWeather].m_SkyHatSettings.m_fTopCloudLight, 0.1f);
            }
            */
            }

        if(ImGui::BeginTabItem("TimeCycle Modifier"))
        {
            ImGui::PopStyleVar();

            TimecycModifierEditor::DrawInWindow();
            ImGui::EndTabItem();
        }
    
        ImGui::EndTabBar();
    }

    ImGui::End();
}

void CenterNextWindow()
{
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
}

void TimecycEditor::DrawSaveWindow()
{
    if(mShowSaveWindow)
    {
        CenterNextWindow();

        static char errorMessage[256];
        static char fileName[256] = "filename.dat";

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
        ImGui::Begin("Save As", &mShowSaveWindow, ImGuiWindowFlags_AlwaysAutoResize);
        {
            ImGui::InputText("##fileName", fileName, 256);

            if(ImGui::Button("Save"))
            {
                if(TimeCycle::Save(fileName, errorMessage, 256))
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
        ImGui::PopStyleVar();
    }
}

void TimecycEditor::DrawLoadWindow()
{
    if(mShowLoadWindow)
    {
        CenterNextWindow();

        static char errorMessage[256];
        static char fileName[256] = "filename.dat";

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
        ImGui::Begin("Load From", &mShowLoadWindow, ImGuiWindowFlags_AlwaysAutoResize);
        {
            ImGui::InputText("##fileName", fileName, 256);

            if(ImGui::Button("Load"))
            {
                if(TimeCycle::Load(fileName, errorMessage, 256))
                {
                    mShowLoadWindow = false;
                    ReSetFloatColors();
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
        ImGui::PopStyleVar();
    }
}

void TimecycEditor::DrawSettingsWindow()
{
    if(mShowSettingsWindow)
    {
        CenterNextWindow();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16, 16));
        ImGui::Begin("Settings", &mShowSettingsWindow, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize);
        {
            ImGui::Text("Position");
            ImGui::DragFloat2("##Position", (float*)&mWindowPos, 1.0f, 0.0f, FLT_MAX);
            ImGui::Text("Size");
            ImGui::DragFloat2("##Size", (float*)&mWindowSize, 1.0f, 1.0f, FLT_MAX);
            ImGui::Text("Font Scale");
            ImGui::DragFloat("##Font Scale", &mFontScale, 0.01f, 0.1f, 2.0f);

            ImGui::GetIO().FontGlobalScale = mFontScale;

            // open window key

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

            // toggle player control

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

            // lock time and weather if editor window is closed

            ImGui::Checkbox("Keep Time and Weather Locked If The Editor Window Is Closed.", &mLockTimeAndWeatherIfClosed);

            // saving and loading

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
        ImGui::PopStyleVar();
    }
}

void TimecycEditor::DrawSetParamOnAllHoursAndWeathersWindow()
{
    if(mShowSetParamOnAllHoursAndWeathersWindow)
    {
        CenterNextWindow();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(48, 11));
        ImGui::Begin("Set Param On All Hours And Weathers", &mShowSetParamOnAllHoursAndWeathersWindow, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize);
        {
            static int currentItemIndex = 0;

            ImGui::Text("Parameter");
            if(ImGui::BeginCombo("##Parameter", std::get<0>(mTimecycParamNameOffsetAndType[currentItemIndex]).c_str(), 0))
            {
                for(uint32_t i = 0; i < mTimecycParamNameOffsetAndType.size(); i++)
                {
                    const bool isSelected = currentItemIndex == i;
                    if(ImGui::Selectable(std::get<0>(mTimecycParamNameOffsetAndType[i]).c_str(), isSelected))
                    {
                        currentItemIndex = i;
                    }
                }
                ImGui::EndCombo();
            }

            static float valueFloat = 0.0f;
            static int32_t valueInt = 0;
            static Colorf valueColor{};

            switch(std::get<2>(mTimecycParamNameOffsetAndType[currentItemIndex]))
            {
                case TIMECYCPARAMTYPE_COLOR_U32:
                    ImGui::Text("Value");
                    ImGui::ColorEdit4("##Value", &valueColor.Red);

                    if(ImGui::Button("Apply"))
                    {
                        Color32 color32 = Color32FromColorf(valueColor);

                        for(uint32_t weather = 0; weather < TimeCycle::NUM_WEATHERS; weather++)
                        {
                            for(uint32_t hour = 0; hour < TimeCycle::NUM_HOURS; hour++)
                            {
                                uint32_t address = (uint32_t)&TimeCycle::m_ColourSets[hour][weather];
                                *(Color32*)(address + std::get<1>(mTimecycParamNameOffsetAndType[currentItemIndex])) = color32;
                            }
                        }

                        ReSetFloatColors();
                        mShowSetParamOnAllHoursAndWeathersWindow = false;
                    }
                break;

                case TIMECYCPARAMTYPE_COLOR_FLOAT3:
                    ImGui::Text("Value");
                    ImGui::ColorEdit4("##Value", &valueColor.Red, ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float);

                    if(ImGui::Button("Apply"))
                    {
                        for(uint32_t weather = 0; weather < TimeCycle::NUM_WEATHERS; weather++)
                        {
                            for(uint32_t hour = 0; hour < TimeCycle::NUM_HOURS; hour++)
                            {
                                uint32_t address = (uint32_t)&TimeCycle::m_ColourSets[hour][weather];
                                *(float*)(address + std::get<1>(mTimecycParamNameOffsetAndType[currentItemIndex]) + 0x0) = valueColor.Red;
                                *(float*)(address + std::get<1>(mTimecycParamNameOffsetAndType[currentItemIndex]) + 0x4) = valueColor.Green;
                                *(float*)(address + std::get<1>(mTimecycParamNameOffsetAndType[currentItemIndex]) + 0x8) = valueColor.Blue;
                            }
                        }

                        mShowSetParamOnAllHoursAndWeathersWindow = false;
                    }
                break;

                case TIMECYCPARAMTYPE_FLOAT:
                    ImGui::Text("Value");
                    ImGui::DragFloat("##Value", &valueFloat, 0.005f);

                    if(ImGui::Button("Apply"))
                    {
                        for(uint32_t weather = 0; weather < TimeCycle::NUM_WEATHERS; weather++)
                        {
                            for(uint32_t hour = 0; hour < TimeCycle::NUM_HOURS; hour++)
                            {
                                uint32_t address = (uint32_t)&TimeCycle::m_ColourSets[hour][weather];
                                *(float *)(address + std::get<1>(mTimecycParamNameOffsetAndType[currentItemIndex])) = valueFloat;
                            }
                        }

                        mShowSetParamOnAllHoursAndWeathersWindow = false;
                    }
                break;

                case TIMECYCPARAMTYPE_INT:
                    ImGui::Text("Value");
                    ImGui::DragInt("##Value", &valueInt, 0.5f);

                    if(ImGui::Button("Apply"))
                    {
                        for(uint32_t weather = 0; weather < TimeCycle::NUM_WEATHERS; weather++)
                        {
                            for(uint32_t hour = 0; hour < TimeCycle::NUM_HOURS; hour++)
                            {
                                uint32_t address = (uint32_t)&TimeCycle::m_ColourSets[hour][weather];
                                *(int32_t *)(address + std::get<1>(mTimecycParamNameOffsetAndType[currentItemIndex])) = valueInt;
                            }
                        }

                        mShowSetParamOnAllHoursAndWeathersWindow = false;
                    }
                break;
            }

            ImGui::SameLine();
            if(ImGui::Button("Cancel"))
            {
                mShowSetParamOnAllHoursAndWeathersWindow = false;
            }
        }
        ImGui::End();
        ImGui::PopStyleVar();
    }
}

void TimecycEditor::DrawLicensesWindow()
{
    if(mShowLicensesWindow)
    {
        CenterNextWindow();

        ImGui::Begin("Licenses", &mShowLicensesWindow);

        ImGui::SeparatorText("Hooking.Patterns");
        static const char* HookingPatternsLicense = 
R"(Copyright (c) 2014 Bas Timmer/NTAuthority et al.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.)";
        ImGui::Text(HookingPatternsLicense);


        ImGui::SeparatorText("ImGui");
        static const char* ImGuiLicense = 
R"(The MIT License (MIT)

Copyright (c) 2014-2023 Omar Cornut

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
)";
        ImGui::Text(ImGuiLicense);


        ImGui::SeparatorText("injector");
        ImGui::Text("Copyright (C) 2012-2014 LINK/2012 <dma_2012@hotmail.com>");

        ImGui::End();
    }
}

int32_t TimecycEditor::TimecycTimeIndexToGameTime(const int32_t timeIndex)
{
    const int32_t timecycTimeIndexToGameTime[TimeCycle::NUM_HOURS] = {0, 5, 6, 7, 9, 12, 18, 19, 20, 21, 22};

    return timecycTimeIndexToGameTime[timeIndex];
}
int32_t TimecycEditor::GameTimeToTimecycTimeIndex(const int32_t gameTime)
{
    const int32_t gameTimeToTimecycTimeIndex[24] = {0, 0, 0, 0, 0, 1, 2, 3, 3, 4, 4, 4, 5, 5, 5, 5, 5, 5, 6, 7, 8, 9, 10, 10};

    return gameTimeToTimecycTimeIndex[gameTime];
}