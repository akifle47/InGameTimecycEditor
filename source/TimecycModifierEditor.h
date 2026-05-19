#pragma once

#include "TimeCycle.h"

#include <unordered_map>
#include <array>
#include <string>

class TimecycModifierEditor
{
public:
    static void Init();
    static void Load();
    static void Save(char* errMessage, uint32_t errMessageSize);

    static void DrawInWindow();

private:
    TimecycModifierEditor() = delete;

    static void ReSetFloatColors();

    // hooks in the function TimeCycle::CalcColoursForPoint

    // called at the beginning of the function
    static void __fastcall BlendColorSetWithModifierH1(TimeCycle::CColourSet* pthis, void* edx, TimeCycle::CTimeCycleModifier* modifier, float weight, bool a4);
    // called at the end of the function (only called once)
    static void __fastcall BlendColorSetWithModifierH2(TimeCycle::CColourSet* pthis, void* edx, TimeCycle::CTimeCycleModifier* modifier, float weight, bool a4);

    // first call in the loop
    static void __fastcall BlendTimeCycleModifiersH1(TimeCycle::CTimeCycleModifier* pthis, void* edx, TimeCycle::CTimeCycleModifier* that, float weight);
    // second call in the loop
    static void __fastcall BlendTimeCycleModifiersH2(TimeCycle::CTimeCycleModifier* pthis, void* edx, TimeCycle::CTimeCycleModifier* that, float weight);

private:
    static inline void(__thiscall* BlendColorSetWithModifierO)(TimeCycle::CColourSet* pthis, TimeCycle::CTimeCycleModifier* modifier, float weight, bool a4) = nullptr;
    static inline void(__thiscall* BlendTimeCycleModifiersO)(TimeCycle::CTimeCycleModifier* pthis, TimeCycle::CTimeCycleModifier* that, float weight) = nullptr;

    static inline uint32_t msSelectedModifierIndex = 0;

    static inline bool msLockModifier = false;

    static inline std::unordered_map<uint32_t, float>                   msActiveExteriorModifiers;
    static inline std::unordered_map<uint32_t, std::pair<float, float>> msActiveInteriorModifiers;

    static inline std::unordered_map<std::string, std::array<Colorf, TimeCycle::MAX_MODIFIERS>> msColorsAsFloat;
    // pair of name and index in the TimeCycle::m_aModifiers array
    static inline std::vector<std::pair<std::string, uint32_t>> msSortedTimecycModifierNames;
};