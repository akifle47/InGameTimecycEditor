#include "TimecycModifierEditor.h"
#include "Common.h"
#include "TimeCycle.h"
#include "imgui.h"
#include "injector/injector.hpp"

#include <algorithm>

static const char* AMBIENT_LIGHT_0_COLOR_ID   = "##MOD_AMBIENT_LIGHT_0_COLOR";
static const char* AMBIENT_LIGHT_1_COLOR_ID   = "##MOD_AMBIENT_LIGHT_1_COLOR";
static const char* DIRECTIONAL_LIGHT_COLOR_ID = "##MOD_DIRECTIONAL_LIGHT_COLOR";
static const char* FOG_COLOR_ID               = "##MOD_FOG_COLOR";
static const char* NEAR_FOG_COLOR_ID          = "##MOD_NEAR_FOG_COLOR";
static const char* COLOR_CORRECT_ID           = "##MOD_COLOR_CORRECT";
static const char* COLOR_ADD_ID               = "##MOD_COLOR_ADD";

void TimecycModifierEditor::Init()
{
    auto pattern = FindPattern({"E8 ? ? ? ? 0F 57 C9 81 C6",  "E8 ? ? ? ? 8B 44 24 ? 0F 57 C9"});
    BlendColorSetWithModifierO = injector::MakeCALL(pattern.get_first(0), BlendColorSetWithModifierH1).get();

    pattern = FindPattern({"E8 ? ? ? ? F3 0F 10 44 24 ? F3 0F 59 44 24 ? F3 0F 10 0D",  "E8 ? ? ? ? F3 0F 10 15 ? ? ? ? F3 0F 2A 83",  "E8 ? ? ? ? 66 0F 6E 97"});
    injector::MakeCALL(pattern.get_first(0), BlendColorSetWithModifierH2);

    pattern = FindPattern({"E8 ? ? ? ? F3 0F 10 84 B4 ? ? ? ? 51",  "E8 ? ? ? ? ? ? ? ? 51 8B D1"});
    BlendTimeCycleModifiersO = injector::MakeCALL(pattern.get_first(0), BlendTimeCycleModifiersH1).get();

    pattern = FindPattern({"E8 ? ? ? ? 0F 57 C0 0F 2F 86 ? ? ? ? 72",  "E8 ? ? ? ? 8B 4C 24 ? 8B 54 24 ? 0F 57 C0"});
    injector::MakeCALL(pattern.get_first(0), BlendTimeCycleModifiersH2);

    pattern = FindPattern({"89 86 ? ? ? ? 33 C9 8A AC 24",  "89 86 ? ? ? ? F3 0F 11 86 ? ? ? ? 0F B6 8C 24"});
    TimeCycle::m_aModifiers = *(decltype(TimeCycle::m_aModifiers)*)pattern.get_first(2);

    Load();
}

void TimecycModifierEditor::Load()
{
    TimeCycle::LoadModifiers(nullptr, 0);
    ReSetFloatColors();

    msSortedTimecycModifierNames.clear();
    msSortedTimecycModifierNames.reserve(TimeCycle::m_NumModifiers);
    for(uint32_t i = 0; i < TimeCycle::m_NumModifiers; i++)
    {
        std::string name = TimeCycle::GetModifierNameFromIndex(i);
        msSortedTimecycModifierNames.emplace_back(name + "##" + std::to_string(i), i);
    }
    
    std::sort(msSortedTimecycModifierNames.begin(), msSortedTimecycModifierNames.end(), [](const auto& a, const auto& b)
    {
        return std::lexicographical_compare(a.first.begin(), a.first.end(),
                                            b.first.begin(), b.first.end(),
                                            [](char ca, char cb) 
                                            {
                                                return std::tolower(ca) < std::tolower(cb); 
                                            });
    });
}

void TimecycModifierEditor::Save(char* errMessage, uint32_t errMessageSize)
{
    TimeCycle::SaveModifiers(errMessage, errMessageSize);
}

void TimecycModifierEditor::ReSetFloatColors()
{
    for(uint32_t i = 0; i < TimeCycle::m_NumModifiers; i++)
    {
        msColorsAsFloat[AMBIENT_LIGHT_0_COLOR_ID][i]   = ColorfFromColor32(TimeCycle::m_aModifiers[i].m_AmbientColor0);
        msColorsAsFloat[AMBIENT_LIGHT_1_COLOR_ID][i]   = ColorfFromColor32(TimeCycle::m_aModifiers[i].m_AmbientColor1);
        msColorsAsFloat[DIRECTIONAL_LIGHT_COLOR_ID][i] = ColorfFromColor32(TimeCycle::m_aModifiers[i].m_DirectionalLightColor);
        msColorsAsFloat[FOG_COLOR_ID][i]               = ColorfFromColor32(TimeCycle::m_aModifiers[i].m_FogColor);
        msColorsAsFloat[NEAR_FOG_COLOR_ID][i]          = ColorfFromColor32(TimeCycle::m_aModifiers[i].m_NearFogColor);
        msColorsAsFloat[COLOR_CORRECT_ID][i]           = ColorfFromColorRGB8(TimeCycle::m_aModifiers[i].m_ColorCorrect);
        msColorsAsFloat[COLOR_ADD_ID][i]               = ColorfFromColorRGB8(TimeCycle::m_aModifiers[i].m_ColorAdd);
    }
}

void TimecycModifierEditor::DrawInWindow()
{
    static int32_t sSelectedSortedModifierIndex = -1;
    static int32_t sSelectedActiveModifierIndex = -1;

    ImGui::Checkbox("Lock to Selected Modifier", &msLockModifier);

    const char* previewText = sSelectedSortedModifierIndex == -1 ? "None" : msSortedTimecycModifierNames[sSelectedSortedModifierIndex].first.c_str();
    ImGui::Text("TimeCycle Modifiers");
    if(ImGui::BeginCombo("##TimeCycle Modifiers", previewText))
    {
        for(uint32_t i = 0; i < TimeCycle::m_NumModifiers; i++)
        {
            const bool isSelected = (sSelectedSortedModifierIndex == i);
            const char* selectedItemName = msSortedTimecycModifierNames[i].first.c_str();

            if(ImGui::Selectable(selectedItemName, isSelected))
            {
                sSelectedSortedModifierIndex = i;
                sSelectedActiveModifierIndex = -1;
                msSelectedModifierIndex = msSortedTimecycModifierNames[i].second;
            }

            if(isSelected)
                ImGui::SetItemDefaultFocus();
        }

        ImGui::EndCombo();
    }

    ImGui::Separator();

    if(ImGui::TreeNode("Active TimeCycle Modifiers"))
    {
        struct TempActiveModifier
        {
            std::string Name;
            size_t Index = 0;
            float Weight = 0.0f;
        };
        std::vector<TempActiveModifier> activeModifiers;
        activeModifiers.reserve(msActiveExteriorModifiers.size() + msActiveInteriorModifiers.size());

        for(auto& activeExtModifier : msActiveExteriorModifiers)
        {
            auto& activeModifier = activeModifiers.emplace_back();
            activeModifier.Name = TimeCycle::GetModifierNameFromHash(activeExtModifier.first);
            for(size_t i = 0; i < TimeCycle::m_NumModifiers; i++)
            {
                if(TimeCycle::m_aModifiers[i].m_Hash == activeExtModifier.first)
                {
                    activeModifier.Index = i;
                    break;
                }
            }
            activeModifier.Weight = activeExtModifier.second;
        }

        for(auto& activeIntModifier : msActiveInteriorModifiers)
        {
            auto& activeModifier = activeModifiers.emplace_back();
            activeModifier.Name = TimeCycle::GetModifierNameFromHash(activeIntModifier.first);
            for(size_t i = 0; i < TimeCycle::m_NumModifiers; i++)
            {
                if(TimeCycle::m_aModifiers[i].m_Hash == activeIntModifier.first)
                {
                    activeModifier.Index = i;
                    break;
                }
            }
            activeModifier.Weight = activeIntModifier.second.first * activeIntModifier.second.second;
        }

        if(activeModifiers.empty())
        {
            auto& activeModifier = activeModifiers.emplace_back();
            activeModifier.Name = "None";
            activeModifier.Weight = 1.0f;
        }

        ImVec2 outerSize{0.0f, ImGui::GetTextLineHeightWithSpacing() * (float)activeModifiers.size()};
        if(ImGui::BeginTable("Active Modifiers", 1, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_BordersInnerV |
                              ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY, outerSize))
        {
            for(size_t i = 0; i < activeModifiers.size(); i++)
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                auto& activeModifier = activeModifiers[i];
                std::string label = activeModifier.Name + ", Weight: " + std::to_string(activeModifier.Weight);

                if(activeModifier.Name == "None")
                {
                    ImGui::Text("None");
                }
                if(ImGui::Selectable(label.c_str(), sSelectedActiveModifierIndex == i, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap))
                {
                    sSelectedActiveModifierIndex = i;
                    msSelectedModifierIndex = activeModifier.Index;

                    for(size_t j = 0; j < msSortedTimecycModifierNames.size(); j++)
                    {
                        std::string unstrippedSortedName = msSortedTimecycModifierNames[j].first.substr(0, msSortedTimecycModifierNames[j].first.find_first_of("#"));
                        if(unstrippedSortedName == activeModifier.Name)
                        {
                            sSelectedSortedModifierIndex = j;
                            break;
                        }
                    }
                }
            }

            ImGui::EndTable();
        }

        msActiveExteriorModifiers.clear();
        msActiveInteriorModifiers.clear();

        ImGui::TreePop();
    }

    ImGui::Separator();

    auto ColorEdit4 = [](const char* label, Color32& color, ImGuiColorEditFlags flags = 0)
    {
        static std::unordered_map<std::string, Color32> sPrevValues;
        
        std::string uniqueLabel = std::string(label) + "_" + std::to_string(msSelectedModifierIndex);

        Colorf* colorf = &msColorsAsFloat[label][msSelectedModifierIndex];
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
    
    auto ColorEdit3RGB8 = [](const char* label, ColorRGB8& color, ImGuiColorEditFlags flags = 0)
    {
        static std::unordered_map<std::string, ColorRGB8> sPrevValues;

        std::string uniqueLabel = std::string(label) + "_" + std::to_string(msSelectedModifierIndex);

        Colorf* colorf = &msColorsAsFloat[label][msSelectedModifierIndex];
        ImGui::ColorEdit3(uniqueLabel.c_str(), &colorf->Red, flags);

        if(ImGui::IsItemActivated())
            sPrevValues[uniqueLabel] = color;

        color = ColorRGB8FromColorf(*colorf);

        if(ImGui::IsItemDeactivatedAfterEdit())
        {
            ColorRGB8 prevValue = sPrevValues[uniqueLabel];
            ColorRGB8 newValue = color;
            PushUndo([&color, colorf, prevValue]() 
                     { 
                         color = prevValue;
                         *colorf = ColorfFromColorRGB8(color); 
                     },
                     [&color, colorf, newValue]() 
                     {
                         color = newValue;
                         *colorf = ColorfFromColorRGB8(color); 
                     });
        }
    };

    auto DragFloat = [](const char* label, float* value, float speed = 1.0f, float min = 0.0f, float max = 0.0f)
    {
        static std::unordered_map<std::string, float> sPrevValues;
    
        std::string uniqueLabel = std::string(label) + "_" + std::to_string(msSelectedModifierIndex);

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
    
    auto DragInt8 = [](const char* label, uint8_t* value, float speed = 1.0f, int min = 0.0f, int max = 0.0f)
    {
        static std::unordered_map<std::string, uint8_t> sPrevValues;
        
        std::string uniqueLabel = std::string(label) + "_" + std::to_string(msSelectedModifierIndex);

        ImGui::DragScalar(uniqueLabel.c_str(), ImGuiDataType_U8, value, speed, &min, &max);
    
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

    auto& selectedModifier = TimeCycle::m_aModifiers[msSelectedModifierIndex];

    if(ImGui::CollapsingHeader("Lighting"))
    {
        ImGui::SeparatorText("Ambient Light 0");
        {
            ImGui::Text("Color");
            ColorEdit4(AMBIENT_LIGHT_0_COLOR_ID, selectedModifier.m_AmbientColor0);

            ImGui::Text("Multiplier");
            DragFloat("##Ambient Light 0 Multiplier", &selectedModifier.m_fAmbientColor0Multiplier, 0.005f, -1.0f, FLT_MAX);
        }
        ImGui::SeparatorText("Ambient Light 1");
        {
            ImGui::Text("Color");
            ColorEdit4(AMBIENT_LIGHT_1_COLOR_ID, selectedModifier.m_AmbientColor1);

            ImGui::Text("Multiplier");
            DragFloat("##Ambient Light 1 Multiplier", &selectedModifier.m_fAmbientColor1Multiplier, 0.005f, -1.0f, FLT_MAX);
        }
        ImGui::SeparatorText("Directional Light");
        {
            ImGui::Text("Color");
            ColorEdit4(DIRECTIONAL_LIGHT_COLOR_ID, selectedModifier.m_DirectionalLightColor);

            ImGui::Text("Multiplier");
            DragFloat("##Directional Light multiplier", &selectedModifier.m_fDirectionalLightColorMultiplier, 0.005f, -1.0f, FLT_MAX);
        }
        ImGui::SeparatorText("Water");
        {
            ImGui::Text("Reflection Multiplier");
            DragFloat("##Water Reflection Multiplier", &selectedModifier.m_fWaterReflectionMultiplier, 0.005f, -1.0f, FLT_MAX);
        }

        ImGui::SeparatorText("");

        ImGui::Text("Ambient Scale");
        DragFloat("##Ambient Scale", &selectedModifier.m_fAmbientScale, 0.005f, 0.0f, 1.0f);

        ImGui::Text("Rim Lighting Multiplier");
        DragFloat("##Rim Lighting Multiplier", &selectedModifier.m_fRimLightingMultiplier, 0.005f, -1.0f, FLT_MAX);

        ImGui::Text("Sky Light Multiplier");
        DragFloat("##Sky Light Multiplier", &selectedModifier.m_fSkyLightMultiplier, 0.005f, -1.0f, FLT_MAX);
        /*
        ImGui::Text("Global Reflection Multiplier");
        DragFloat("##Global Reflection Multiplier", &selectedModifier.m_fGlobalReflectionMultiplier, 0.005f, -1.0f, FLT_MAX);
        */
        ImGui::Text("AO Strength");
        DragFloat("##AO Strength", &selectedModifier.m_fAoStrength, 0.005f, -1.0f, FLT_MAX);

        ImGui::Text("Ped AO Strength");
        DragFloat("##Ped AO Strength", &selectedModifier.m_fPedAoStrength, 0.005f, -1.0f, FLT_MAX);

        ImGui::Text("Particle Brightness");
        DragFloat("##Particle Brightness", &selectedModifier.m_fParticleIntensity, 0.005f, -1.0f, FLT_MAX);
    }

    if(ImGui::CollapsingHeader("Atmosphere"))
    {
        ImGui::Text("Temperature");
        DragFloat("##Temperature", &selectedModifier.m_fTemperature, 0.1f, -100.0f, 50.0f);

        ImGui::Text("Enable Rain Effects");
        ImGui::SliderInt("##Enable Rain Effects", &(int32_t&)selectedModifier.m_EnableRainEffects, 0, 1);

        if(ImGui::TreeNode("Fog"))
        {
            ImGui::Text("Min Start Distance");
            DragFloat("##Fog Start", &selectedModifier.m_fMinFogStart, 0.2f, -1.0f, selectedModifier.m_fMaxFogStart);

            ImGui::Text("Max Start Distance");
            DragFloat("##Fog Max", &selectedModifier.m_fMaxFogStart, 0.5f, selectedModifier.m_fMinFogStart, FLT_MAX);

            ImGui::Text("Near Color");
            ColorEdit4(NEAR_FOG_COLOR_ID, selectedModifier.m_NearFogColor);

            ImGui::Text("Far Color");
            ColorEdit4(FOG_COLOR_ID, selectedModifier.m_FogColor);

            ImGui::Text("Near Strength");
            DragInt8("##Near Strength", &selectedModifier.m_fNearColorStrength, 0.2f, 0, 255);

            ImGui::TreePop();
        }
    }

    if(ImGui::CollapsingHeader("Post Processing"))
    {
        ImGui::Text("PostFX Strength");
        DragFloat("##PostFX Strength", &selectedModifier.m_fPostFxStrength, 0.005f, -1.0f, 1.0f);

        ImGui::Text("Luminance Min");
        DragFloat("##Luminance Min", &selectedModifier.m_fLumMin, 0.005f, -1.0f, FLT_MAX);

        ImGui::Text("Luminance Max");
        DragFloat("##Luminance Max", &selectedModifier.m_fLumMax, 0.005f, -1.0f, FLT_MAX);

        ImGui::Text("Exposure");
        DragFloat("##Exposure", &selectedModifier.m_fExposure, 0.001f, -1.0f, FLT_MAX);

        ImGui::Text("Exposure Multiplier");
        DragFloat("##Exposure Multiplier", &selectedModifier.m_fExposureMultiplier, 0.001f, -1.0f, FLT_MAX);

        ImGui::Text("Color Correction");
        ColorEdit3RGB8(COLOR_CORRECT_ID, selectedModifier.m_ColorCorrect);

        ImGui::Text("Color Add");
        ColorEdit3RGB8(COLOR_ADD_ID, selectedModifier.m_ColorAdd);

        ImGui::Text("Bloom Threshold");
        DragFloat("##Bloom Threshold", &selectedModifier.m_fBloomThreshold, 0.005f, -1.0f, FLT_MAX);

        ImGui::Text("Bloom Intensity");
        DragFloat("##Bloom Intensity", &selectedModifier.m_fBloomIntensity, 0.005f, -1.0f, FLT_MAX);

        ImGui::Text("Mid Gray Value");
        DragFloat("##Mid Gray Value", &selectedModifier.m_fMidGrayValue, 0.005f, -1.0f, FLT_MAX);

        ImGui::Text("Desaturation");
        DragFloat("##Desaturation", &selectedModifier.m_fDesaturation, 0.005f, -1.0f, 1.0f);

        ImGui::Text("Desaturation Far");
        DragFloat("##Desaturation Far", &selectedModifier.m_fDesaturationFar, 0.005f, -1.0f, 1.0f);

        ImGui::Text("Gamma");
        DragFloat("##Gamma", &selectedModifier.m_fGamma, 0.005f, -1.0f, FLT_MAX);

        ImGui::Text("Gamma Far");
        DragFloat("##Gamma Far", &selectedModifier.m_fGammaFar, 0.005f, -1.0f, FLT_MAX);

        ImGui::Text("DepthFX Near");
        DragFloat("##DepthFX Near", &selectedModifier.m_fDepthFxNear, 0.5f, -1.0f, selectedModifier.m_fDepthFxFar);

        ImGui::Text("DepthFX Far");
        DragFloat("##DepthFX Far", &selectedModifier.m_fDepthFxFar, 0.5f, -1.0f, FLT_MAX);

        ImGui::Text("Min Near DOF");
        DragFloat("##Min Near DOF", &selectedModifier.m_fMinNearDof, 0.2f, -1.0f, FLT_MAX);

        ImGui::Text("Min Far DOF");
        DragFloat("##Min Far DOF", &selectedModifier.m_fMinFarDof, 0.3f, -1.0f, selectedModifier.m_fMaxFarDof);

        ImGui::Text("Max Far DOF");
        DragFloat("##Max Far DOF", &selectedModifier.m_fMaxFarDof, 0.3f, -1.0f, FLT_MAX);

        ImGui::Text("Mid DOF Blur");
        DragFloat("##Mid DOF Blur", &selectedModifier.m_fMidDofBlur, 0.005f, -1.0f, selectedModifier.m_fFarDofBlur);

        ImGui::Text("Far DOF Blur");
        DragFloat("##Far DOF Blur", &selectedModifier.m_fFarDofBlur, 0.005f, -1.0f, 1.0f);
    }

    if(ImGui::CollapsingHeader("Other"))
    {
        ImGui::Text("Min Far Clip");
        DragFloat("##Min Far Clip", &selectedModifier.m_fMinFarClip, 0.5f, -1.0f, selectedModifier.m_fMaxFarClip);

        ImGui::Text("Max Far Clip");
        DragFloat("##Max Far Clip", &selectedModifier.m_fMaxFarClip, 0.5f, -1.0f, FLT_MAX);
        /*
        ImGui::Text("Contrast");
        DragFloat("##Contrast", &selectedModifier.m_fContrast, 0.005f, -1.0f, FLT_MAX);

        ImGui::Text("Contrast Far");
        DragFloat("##Contrast Far", &selectedModifier.m_fContrastFar, 0.005f, -1.0f, FLT_MAX);
        */
    }
}

void __fastcall TimecycModifierEditor::BlendColorSetWithModifierH1(TimeCycle::CColourSet* pthis, void* edx, TimeCycle::CTimeCycleModifier* modifier, float weight, bool a4)
{
    msActiveExteriorModifiers[modifier->m_Hash] = weight;

    if(msLockModifier)
        return;
    
    BlendColorSetWithModifierO(pthis, modifier, weight, a4);
}

void __fastcall TimecycModifierEditor::BlendColorSetWithModifierH2(TimeCycle::CColourSet* pthis, void* edx, TimeCycle::CTimeCycleModifier* modifier, float weight, bool a4)
{
    for(auto& activeExteriorMods : msActiveExteriorModifiers)
    {
        float globalExteriorWeight = 1.0f - weight;
        activeExteriorMods.second *= globalExteriorWeight;
    }

    if(msLockModifier)
    {
        BlendColorSetWithModifierO(pthis, &TimeCycle::m_aModifiers[msSelectedModifierIndex], 1.0f, true);
        return;
    }

    BlendColorSetWithModifierO(pthis, modifier, weight, a4);
}

void __fastcall TimecycModifierEditor::BlendTimeCycleModifiersH1(TimeCycle::CTimeCycleModifier* pthis, void* edx, TimeCycle::CTimeCycleModifier* that, float weight)
{
    msActiveInteriorModifiers[that->m_Hash].first = std::max(msActiveInteriorModifiers[that->m_Hash].first, weight);
    pthis->m_Hash = that->m_Hash;

    if(msLockModifier)
        return;

    BlendTimeCycleModifiersO(pthis, that, weight);
}

void __fastcall TimecycModifierEditor::BlendTimeCycleModifiersH2(TimeCycle::CTimeCycleModifier* pthis, void* edx, TimeCycle::CTimeCycleModifier* that, float weight)
{
    float currWeightMul = msActiveInteriorModifiers[that->m_Hash].second;
    msActiveInteriorModifiers[that->m_Hash].second = std::max(currWeightMul, weight);

    if(msLockModifier)
        return;

    BlendTimeCycleModifiersO(pthis, that, weight);
}