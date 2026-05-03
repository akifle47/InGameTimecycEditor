#pragma once

#include "Color.h"

namespace rage
{
    struct SkyHatPerFrameSettings
    {
        float m_fSkyColor[3];
        float field_C;
        float m_fSkyHorizonColor[3];
        float field_1C;
        float m_fSkyEastHorizonColor[3];
        float field_2C;
        float m_fSunsetColor[3];
        float m_fUnknown_3C; // unused
        float m_fSkyHorizonHeight;
        float m_fSkyHorizonBrightness;
        float m_fSunAxisX;
        float m_fSunAxisY;
        float m_fCloudColor[3];
        float field_5C;
        float m_fCloud2ShadowStrength;
        float m_fCloud2Threshold;
        float m_fCloud2Bias1;
        float m_fCloud2ShadowOffset;
        float m_fCloudInScatteringRange;
        float m_fEdgeSmooth;
        float m_fDetailNoiseScale;
        float m_fDetailNoiseMultiplier;
        float m_fCloud2Thickness;
        float m_fCloudWarp;
        float m_fCloudsFadeOut;
        float m_fCloud1Bias;
        float m_fCloud1Detail;
        float m_fCloud1Threshold;
        float m_fCloud1Height;
        float field_9C;
        float m_fTopCloudColor[3];
        float field_AC;
        float m_fTopCloudLight;
        float field_B4[3];
        float m_fSunColor[3];
        float field_CC;
        float m_fUnderlighting;
        float m_fDetailNoiseOffset;
        float m_fStarFieldBrightness;
        float m_fStarFieldThreshold;
        float m_fMoonBrightness;
        float field_E4[3];
        float m_fMoonColor[3];
        float field_FC;
        float m_fMoonGlow;
        float m_fMoonTransparency;
        float m_fSunCenterStart;
        float m_fSunCenterEnd;
        float m_fSunBrightness;
        float field_114;
    };
}
static_assert(sizeof(rage::SkyHatPerFrameSettings) == 0x118);

struct TimeCycle
{
public:
    static bool Save(const char* fileName, char* errMessage, uint32_t errMessageSize);
    static bool Load(const char* fileName, char* errMessage, uint32_t errMessageSize);

public:
    static constexpr uint32_t NUM_HOURS = 11;
    static constexpr uint32_t NUM_WEATHERS = 9;

    struct CColourSet
    {
        Color32 m_Ambient0Color;
        Color32 m_Ambient1Color;
        Color32 m_DirLightColor;
        float m_fDirLightMultiplier;
        float m_fAmbient0Multiplier;
        float m_fAmbient1Multiplier;
        float m_fAOStrength;
        float m_fPedAOStrength;
        float m_fRimLightingMultiplier;
        float m_fSkyLightMultiplier;
        float m_fDirLightSpecMultiplier;
        Color32 m_FogColorDensity;
        Color32 m_SunCore;
        float m_fCoronaBrightness;
        float m_fCoronaSize;
        float m_fDistantCoronaBrightness;
        float m_fDistantCoronaSize;
        float m_fFarClip;
        float m_fFogStart;
        float m_fDOFStart;
        float m_fNearDOFBlur;
        float m_fFarDOFBlur;
        Color32 m_LowCloudsColor;
        Color32 m_BottomCloudsColor;
        Color32 m_Water;
        float field_64[7];
        float m_fWaterReflectionMultiplier;
        float m_fParticleBrightness;
        float m_fExposure;
        float m_fBloomThreshold;
        float m_fMidGrayValue;
        float m_fBloomIntensity;
        Color32 m_ColorCorrection;
        Color32 m_ColorAdd;
        float m_fDesaturation;
        float m_fContrast;
        float m_fGamma;
        float m_fDesaturationFar;
        float m_fContrastFar;
        float m_fGammaFar;
        float m_fDepthFxNear;
        float m_fDepthFxFar;
        float m_fLumMin;
        float m_fLumMax;
        float m_fLumDelay;
        int32_t m_CloudAlpha;
        float field_D0;
        float m_fTemperature;
        float m_fGlobalReflectionMultiplier;
        float field_DC;
        rage::SkyHatPerFrameSettings m_SkyHatSettings;
        float field_1F8[2];
        float m_fSunGlowTransparency;
        float m_fSkyBrightness;
        float field_208;
        int32_t m_FilmGrain;
    };
    static_assert(sizeof(CColourSet) == 0x210);

    static inline CColourSet (*m_ColourSets)[NUM_WEATHERS];
};