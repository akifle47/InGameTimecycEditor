#include "TimeCycle.h"
#include "StringHash.h"

#include <fstream>
#include <string>
#include <sstream>
#include <assert.h>

struct SerializationContext
{
    std::fstream File;
    std::stringstream LineStream;
    bool Writing = false;
} gSerializationContext;

inline void WriteColorRGB(std::ofstream& file, Color32 color)
{
    file << (uint32_t)color.Red << " " << (uint32_t)color.Green << " " << (uint32_t)color.Blue << " ";
}

inline Color32 ReadColorRGB(std::stringstream& ss)
{
    Color32 color32;
    uint32_t color[3];

    ss >> color[0] >> color[1] >> color[2];
    color32.Red = color[0];
    color32.Green = color[1];
    color32.Blue = color[2];
    color32.Alpha = 0;

    return color32;
}

inline void SerializeInt(uint8_t& value)
{
    if(gSerializationContext.Writing)
    {
        gSerializationContext.File << (int32_t)value << "    ";
    }
    else
    {
        uint32_t temp;
        gSerializationContext.LineStream >> temp;
        value = temp;
    }
}

inline void SerializeInt(int32_t& value)
{
    if(gSerializationContext.Writing)
    {
        gSerializationContext.File << value << "    ";
    }
    else
    {
        gSerializationContext.LineStream >> value;
    }
}

inline void SerializeInt(uint32_t& value)
{
    if(gSerializationContext.Writing)
    {
        gSerializationContext.File << value << "    ";
    }
    else
    {
        gSerializationContext.LineStream >> value;
    }
}

inline void SerializeFloat(float& value)
{
    if(gSerializationContext.Writing)
    {
        gSerializationContext.File << value << "    ";
    }
    else
    {
        gSerializationContext.LineStream >> value;
    }
}

inline void SerializeRGBA32(Color32& color)
{
    if(gSerializationContext.Writing)
    {
        gSerializationContext.File << (uint32_t)color.Red << " " << (uint32_t)color.Green << " " << (uint32_t)color.Blue << " " << (uint32_t)color.Alpha << "    ";
    }
    else
    {
        uint32_t temp[4];

        gSerializationContext.LineStream >> temp[0] >> temp[1] >> temp[2] >> temp[3];
        color.Red   = temp[0];
        color.Green = temp[1];
        color.Blue  = temp[2];
        color.Alpha = temp[3];
    }
}

inline void SerializeRGB8(ColorRGB8& color)
{
    if(gSerializationContext.Writing)
    {
        gSerializationContext.File << (uint32_t)color.Red << " " << (uint32_t)color.Green << " " << (uint32_t)color.Blue << " " << "    ";
    }
    else
    {
        uint32_t temp[3];

        gSerializationContext.LineStream >> temp[0] >> temp[1] >> temp[2];
        color.Red   = temp[0];
        color.Green = temp[1];
        color.Blue  = temp[2];
    }
}


bool TimeCycle::Save(const char *fileName, char *errMessage, uint32_t errMessageSize)
{
    std::ofstream file(fileName);

    if(!file.good())
    {
        if(errMessageSize > 0)
        {
            memset(errMessage, 0, errMessageSize);
            strerror_s(errMessage, errMessageSize, errno);
        }

        return false;
    }

    const char *timeOfDayNames[NUM_HOURS] = {"Midnight", "5AM", "6AM", "7AM", "9AM", "Midday", "6PM", "7PM", "8PM", "9PM", "10PM"};
    const char *weatherNames[NUM_WEATHERS] = {"EXTRASUNNY", "SUNNY", "SUNNY_WINDY", "CLOUDY", "RAIN", "DRIZZLE", "FOGGY", "LIGHTNING", "TEMP"};
    const float unusedParam = -1.0f;

    for(uint32_t weather = 0; weather < NUM_WEATHERS; weather++)
    {
        if(weather > 0)
            file << "\n";

        file << "//" << weatherNames[weather] << "\n";

        for(uint32_t time = 0; time < NUM_HOURS; time++)
        {
            file << "//" << timeOfDayNames[time] << "\n";

            WriteColorRGB(file, m_ColourSets[time][weather].m_Ambient0Color);
            file << "   ";
            WriteColorRGB(file, m_ColourSets[time][weather].m_Ambient1Color);
            file << "   ";
            WriteColorRGB(file, m_ColourSets[time][weather].m_DirLightColor);
            file << "   ";

            file << unusedParam << "    ";

            file << m_ColourSets[time][weather].m_FilmGrain << "    ";

            file << (uint32_t)m_ColourSets[time][weather].m_FogColorDensity.Alpha << " ";
            WriteColorRGB(file, m_ColourSets[time][weather].m_FogColorDensity);
            file << "   ";

            WriteColorRGB(file, m_ColourSets[time][weather].m_SunCore);
            file << "   ";

            file << int32_t(unusedParam) << "    " << int32_t(unusedParam) << "    " << int32_t(unusedParam) << "    ";
            file << unusedParam << "    ";
            
            file << (m_ColourSets[time][weather].m_fCoronaBrightness / 10.0f) << "    ";
            file << m_ColourSets[time][weather].m_fFarClip << "    ";
            file << m_ColourSets[time][weather].m_fFogStart << "    ";

            WriteColorRGB(file, m_ColourSets[time][weather].m_LowCloudsColor);
            file << "   ";
            WriteColorRGB(file, m_ColourSets[time][weather].m_BottomCloudsColor);
            file << "   ";

            WriteColorRGB(file, m_ColourSets[time][weather].m_Water);
            file << (uint32_t)m_ColourSets[time][weather].m_Water.Alpha << "    ";

            file << m_ColourSets[time][weather].m_fExposure << "    ";
            file << m_ColourSets[time][weather].m_fBloomThreshold << "    ";
            file << m_ColourSets[time][weather].m_fMidGrayValue << "    ";
            file << m_ColourSets[time][weather].m_fBloomIntensity << "    ";

            WriteColorRGB(file, m_ColourSets[time][weather].m_ColorCorrection);
            file << "   ";
            WriteColorRGB(file, m_ColourSets[time][weather].m_ColorAdd);
            file << "   ";

            file << m_ColourSets[time][weather].m_fDesaturation << "    ";
            file << m_ColourSets[time][weather].m_fContrast << "    ";
            file << m_ColourSets[time][weather].m_fGamma << "    ";
            file << m_ColourSets[time][weather].m_fDesaturationFar << "    ";
            file << m_ColourSets[time][weather].m_fContrastFar << "    ";
            file << m_ColourSets[time][weather].m_fGammaFar << "    ";
            file << m_ColourSets[time][weather].m_fDepthFxNear << "    ";
            file << m_ColourSets[time][weather].m_fDepthFxFar << "    ";
            file << m_ColourSets[time][weather].m_fLumMin << "    ";
            file << m_ColourSets[time][weather].m_fLumMax << "    ";
            file << m_ColourSets[time][weather].m_fLumDelay << "    ";
            file << m_ColourSets[time][weather].m_CloudAlpha << "    ";
            file << m_ColourSets[time][weather].m_fDirLightMultiplier << "    ";
            file << m_ColourSets[time][weather].m_fAmbient0Multiplier << "    ";
            file << m_ColourSets[time][weather].m_fAmbient1Multiplier << "    ";
            file << m_ColourSets[time][weather].m_fSkyLightMultiplier << "    ";
            file << m_ColourSets[time][weather].m_fDirLightSpecMultiplier << "    ";
            file << (int32_t)m_ColourSets[time][weather].m_fTemperature << "    ";
            file << m_ColourSets[time][weather].m_fGlobalReflectionMultiplier << "    ";

            file << (m_ColourSets[time][weather].m_SkyHatSettings.m_SkyColor.Red) << " ";
            file << (m_ColourSets[time][weather].m_SkyHatSettings.m_SkyColor.Green) << " ";
            file << (m_ColourSets[time][weather].m_SkyHatSettings.m_SkyColor.Blue) << "    ";

            file << (m_ColourSets[time][weather].m_SkyHatSettings.m_SkyHorizonColor.Red) << " ";
            file << (m_ColourSets[time][weather].m_SkyHatSettings.m_SkyHorizonColor.Green) << " ";
            file << (m_ColourSets[time][weather].m_SkyHatSettings.m_SkyHorizonColor.Blue) << "    ";

            file << (m_ColourSets[time][weather].m_SkyHatSettings.m_SkyEastHorizonColor.Red) << " ";
            file << (m_ColourSets[time][weather].m_SkyHatSettings.m_SkyEastHorizonColor.Green) << " ";
            file << (m_ColourSets[time][weather].m_SkyHatSettings.m_SkyEastHorizonColor.Blue) << "    ";

            file << (m_ColourSets[time][weather].m_SkyHatSettings.m_SunsetColor.Red) << " ";
            file << (m_ColourSets[time][weather].m_SkyHatSettings.m_SunsetColor.Green) << " ";
            file << (m_ColourSets[time][weather].m_SkyHatSettings.m_SunsetColor.Blue) << "    ";

            file << m_ColourSets[time][weather].m_SkyHatSettings.m_fUnknown_3C << "    ";
            file << m_ColourSets[time][weather].m_SkyHatSettings.m_fSkyHorizonHeight << "    ";
            file << m_ColourSets[time][weather].m_SkyHatSettings.m_fSkyHorizonBrightness << "    ";
            file << unusedParam << "    ";
            file << unusedParam << "    ";

            file << (m_ColourSets[time][weather].m_SkyHatSettings.m_CloudColor.Red) << " ";
            file << (m_ColourSets[time][weather].m_SkyHatSettings.m_CloudColor.Green) << " ";
            file << (m_ColourSets[time][weather].m_SkyHatSettings.m_CloudColor.Blue) << "    ";

            file << m_ColourSets[time][weather].m_SkyHatSettings.m_fCloud2ShadowStrength << "    ";
            file << m_ColourSets[time][weather].m_SkyHatSettings.m_fCloud2Threshold << "    ";
            file << m_ColourSets[time][weather].m_SkyHatSettings.m_fCloud2Bias1 << "    ";
            file << m_ColourSets[time][weather].m_SkyHatSettings.m_fCloud2ShadowOffset << "    ";
            file << unusedParam << " ";
            file << m_ColourSets[time][weather].m_SkyHatSettings.m_fEdgeSmooth << "    ";
            file << m_ColourSets[time][weather].m_SkyHatSettings.m_fDetailNoiseScale << "    ";
            file << m_ColourSets[time][weather].m_SkyHatSettings.m_fDetailNoiseMultiplier << "    ";
            file << m_ColourSets[time][weather].m_SkyHatSettings.m_fCloud2Thickness << "    ";
            file << unusedParam << "    ";
            file << m_ColourSets[time][weather].m_SkyHatSettings.m_fCloudsFadeOut << "    ";
            file << m_ColourSets[time][weather].m_SkyHatSettings.m_fCloud1Bias << "    ";
            file << m_ColourSets[time][weather].m_SkyHatSettings.m_fCloud1Detail << "    ";
            file << m_ColourSets[time][weather].m_SkyHatSettings.m_fCloud1Threshold << "    ";
            file << m_ColourSets[time][weather].m_SkyHatSettings.m_fCloud1Height << "    ";

            file << (m_ColourSets[time][weather].m_SkyHatSettings.m_TopCloudColor.Red) << " ";
            file << (m_ColourSets[time][weather].m_SkyHatSettings.m_TopCloudColor.Green) << " ";
            file << (m_ColourSets[time][weather].m_SkyHatSettings.m_TopCloudColor.Blue) << "    ";

            file << m_ColourSets[time][weather].m_SkyHatSettings.m_fTopCloudLight << "    ";

            file << (m_ColourSets[time][weather].m_SkyHatSettings.m_SunColor.Red) << " ";
            file << (m_ColourSets[time][weather].m_SkyHatSettings.m_SunColor.Green) << " ";
            file << (m_ColourSets[time][weather].m_SkyHatSettings.m_SunColor.Blue) << "    ";

            file << m_ColourSets[time][weather].m_SkyHatSettings.m_fUnderlighting << "    ";
            file << m_ColourSets[time][weather].m_SkyHatSettings.m_fDetailNoiseOffset << "    ";
            file << m_ColourSets[time][weather].m_SkyHatSettings.m_fStarFieldBrightness << "    ";
            file << m_ColourSets[time][weather].m_SkyHatSettings.m_fStarFieldThreshold << "    ";
            file << m_ColourSets[time][weather].m_SkyHatSettings.m_fMoonBrightness << "    ";

            file << (m_ColourSets[time][weather].m_SkyHatSettings.m_MoonColor.Red) << " ";
            file << (m_ColourSets[time][weather].m_SkyHatSettings.m_MoonColor.Green) << " ";
            file << (m_ColourSets[time][weather].m_SkyHatSettings.m_MoonColor.Blue) << "    ";

            file << m_ColourSets[time][weather].m_SkyHatSettings.m_fMoonGlow << "    ";
            file << m_ColourSets[time][weather].m_SkyHatSettings.m_fMoonTransparency << "    ";
            file << unusedParam << "    ";
            file << unusedParam << "    ";
            file << m_ColourSets[time][weather].m_SkyHatSettings.m_fSunBrightness << "    ";
            file << m_ColourSets[time][weather].m_fSunGlowTransparency << "    ";
            file << m_ColourSets[time][weather].m_fDOFStart << "    ";
            file << unusedParam << "    ";
            file << unusedParam << "    ";
            file << m_ColourSets[time][weather].m_fNearDOFBlur << "    ";
            file << m_ColourSets[time][weather].m_fFarDOFBlur << "    ";
            file << m_ColourSets[time][weather].m_fWaterReflectionMultiplier << "    ";
            file << m_ColourSets[time][weather].m_fParticleBrightness << "    ";
            file << m_ColourSets[time][weather].m_fCoronaSize << "    ";
            file << m_ColourSets[time][weather].m_fSkyBrightness << "    ";
            file << m_ColourSets[time][weather].m_fAOStrength << "    ";
            file << m_ColourSets[time][weather].m_fRimLightingMultiplier << "    ";
            file << (m_ColourSets[time][weather].m_fDistantCoronaBrightness / 10.0f) << "    ";
            file << m_ColourSets[time][weather].m_fDistantCoronaSize << "    ";
            file << m_ColourSets[time][weather].m_fPedAOStrength << "\n";
        }
    }

    return true;
}

bool TimeCycle::Load(const char *fileName, char *errMessage, uint32_t errMessageSize)
{
    std::ifstream file(fileName);

    if(!file.good())
    {
        if(errMessageSize > 0)
        {
            memset(errMessage, 0, errMessageSize);
            strerror_s(errMessage, errMessageSize, errno);
        }

        return false;
    }

    float tempUnusedParam = 0.0f;
    volatile float unusedParam = 0.0f;
    std::stringstream currLineSS;
    std::string currLine;
    uint32_t weather = 0;
    uint32_t alpha = 0;

    while(file)
    {
        std::getline(file, currLine);

        if(currLine[0] == '/' || currLine[0] == '\0')
            continue;

        for(uint32_t time = 0; time < NUM_HOURS; time++)
        {
            currLineSS.clear();
            currLineSS.str(currLine);

            m_ColourSets[time][weather].m_Ambient0Color = ReadColorRGB(currLineSS);
            m_ColourSets[time][weather].m_Ambient1Color = ReadColorRGB(currLineSS);
            m_ColourSets[time][weather].m_DirLightColor = ReadColorRGB(currLineSS);
            
            currLineSS >> tempUnusedParam;
            currLineSS >> m_ColourSets[time][weather].m_FilmGrain;
            
            currLineSS >> alpha;
            m_ColourSets[time][weather].m_FogColorDensity = ReadColorRGB(currLineSS);
            m_ColourSets[time][weather].m_FogColorDensity.Alpha = alpha;

            m_ColourSets[time][weather].m_SunCore = ReadColorRGB(currLineSS);

            currLineSS >> tempUnusedParam >> tempUnusedParam >> tempUnusedParam >> tempUnusedParam;
            currLineSS >> m_ColourSets[time][weather].m_fCoronaBrightness;
            m_ColourSets[time][weather].m_fCoronaBrightness *= 10.0f;

            currLineSS >> m_ColourSets[time][weather].m_fFarClip;
            currLineSS >> m_ColourSets[time][weather].m_fFogStart;

            m_ColourSets[time][weather].m_LowCloudsColor = ReadColorRGB(currLineSS);
            m_ColourSets[time][weather].m_BottomCloudsColor = ReadColorRGB(currLineSS);

            m_ColourSets[time][weather].m_Water = ReadColorRGB(currLineSS);
            currLineSS >> alpha;
            m_ColourSets[time][weather].m_Water.Alpha = alpha;

            currLineSS >> m_ColourSets[time][weather].m_fExposure;
            currLineSS >> m_ColourSets[time][weather].m_fBloomThreshold;
            currLineSS >> m_ColourSets[time][weather].m_fMidGrayValue;
            currLineSS >> m_ColourSets[time][weather].m_fBloomIntensity;

            m_ColourSets[time][weather].m_ColorCorrection = ReadColorRGB(currLineSS);
            m_ColourSets[time][weather].m_ColorAdd = ReadColorRGB(currLineSS);

            currLineSS >> m_ColourSets[time][weather].m_fDesaturation;
            currLineSS >> m_ColourSets[time][weather].m_fContrast;
            currLineSS >> m_ColourSets[time][weather].m_fGamma;
            currLineSS >> m_ColourSets[time][weather].m_fDesaturationFar;
            currLineSS >> m_ColourSets[time][weather].m_fContrastFar;
            currLineSS >> m_ColourSets[time][weather].m_fGammaFar;
            currLineSS >> m_ColourSets[time][weather].m_fDepthFxNear;
            currLineSS >> m_ColourSets[time][weather].m_fDepthFxFar;
            currLineSS >> m_ColourSets[time][weather].m_fLumMin;
            currLineSS >> m_ColourSets[time][weather].m_fLumMax;
            currLineSS >> m_ColourSets[time][weather].m_fLumDelay;
            currLineSS >> m_ColourSets[time][weather].m_CloudAlpha;
            currLineSS >> m_ColourSets[time][weather].m_fDirLightMultiplier;
            currLineSS >> m_ColourSets[time][weather].m_fAmbient0Multiplier;
            currLineSS >> m_ColourSets[time][weather].m_fAmbient1Multiplier;
            currLineSS >> m_ColourSets[time][weather].m_fSkyLightMultiplier;
            currLineSS >> m_ColourSets[time][weather].m_fDirLightSpecMultiplier;
            currLineSS >> m_ColourSets[time][weather].m_fTemperature;
            currLineSS >> m_ColourSets[time][weather].m_fGlobalReflectionMultiplier;

            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_SkyColor.Red;
            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_SkyColor.Green;
            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_SkyColor.Blue;

            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_SkyHorizonColor.Red;
            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_SkyHorizonColor.Green;
            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_SkyHorizonColor.Blue;
            
            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_SkyEastHorizonColor.Red;
            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_SkyEastHorizonColor.Green;
            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_SkyEastHorizonColor.Blue;

            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_SunsetColor.Red;
            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_SunsetColor.Green;
            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_SunsetColor.Blue;
            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_fUnknown_3C;
            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_fSkyHorizonHeight;
            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_fSkyHorizonBrightness;

            currLineSS >> tempUnusedParam >> tempUnusedParam;
            
            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_CloudColor.Red;
            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_CloudColor.Green;
            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_CloudColor.Blue;

            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_fCloud2ShadowStrength;
            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_fCloud2Threshold;
            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_fCloud2Bias1;
            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_fCloud2ShadowOffset;
            currLineSS >> tempUnusedParam;
            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_fEdgeSmooth;
            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_fDetailNoiseScale;
            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_fDetailNoiseMultiplier;
            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_fCloud2Thickness;
            currLineSS >> tempUnusedParam;
            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_fCloudsFadeOut;
            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_fCloud1Bias;
            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_fCloud1Detail;
            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_fCloud1Threshold;
            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_fCloud1Height;

            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_TopCloudColor.Red;
            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_TopCloudColor.Green;
            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_TopCloudColor.Blue;

            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_fTopCloudLight;

            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_SunColor.Red;
            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_SunColor.Green;
            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_SunColor.Blue;

            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_fUnderlighting;
            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_fDetailNoiseOffset;
            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_fStarFieldBrightness;
            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_fStarFieldThreshold;
            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_fMoonBrightness;

            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_MoonColor.Red;
            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_MoonColor.Green;
            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_MoonColor.Blue;
            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_fMoonGlow;
            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_fMoonTransparency;

            currLineSS >> tempUnusedParam >> tempUnusedParam;
            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_fSunBrightness;
            currLineSS >> m_ColourSets[time][weather].m_fSunGlowTransparency;
            currLineSS >> m_ColourSets[time][weather].m_fDOFStart;
            currLineSS >> tempUnusedParam >> tempUnusedParam;
            currLineSS >> m_ColourSets[time][weather].m_fNearDOFBlur;
            currLineSS >> m_ColourSets[time][weather].m_fFarDOFBlur;
            currLineSS >> m_ColourSets[time][weather].m_fWaterReflectionMultiplier;
            currLineSS >> m_ColourSets[time][weather].m_fParticleBrightness;
            currLineSS >> m_ColourSets[time][weather].m_fCoronaSize;
            currLineSS >> m_ColourSets[time][weather].m_fSkyBrightness;
            currLineSS >> m_ColourSets[time][weather].m_fAOStrength;
            currLineSS >> m_ColourSets[time][weather].m_fRimLightingMultiplier;
            currLineSS >> m_ColourSets[time][weather].m_fDistantCoronaBrightness;
            m_ColourSets[time][weather].m_fDistantCoronaBrightness *= 10.0f;
            currLineSS >> m_ColourSets[time][weather].m_fDistantCoronaSize;
            currLineSS >> m_ColourSets[time][weather].m_fPedAOStrength;

            std::getline(file, currLine);
            while(currLine[0] == '/')
                std::getline(file, currLine);
        }

        weather++;
    }

    unusedParam = tempUnusedParam;

    return true;
}

bool TimeCycle::SerializeModifier(const char* fileName, char* errMessage, uint32_t errMessageSize, bool writing, size_t fromIndex, size_t toindex)
{
    gSerializationContext = {};

    gSerializationContext.File = std::fstream(fileName, writing ? std::ios::out : std::ios::in);
    gSerializationContext.Writing = writing;

    if(!gSerializationContext.File.good())
    {
        if(errMessageSize > 0)
        {
            memset(errMessage, 0, errMessageSize);
            strerror_s(errMessage, errMessageSize, errno);
        }

        return false;
    }

    std::string currLine;
    size_t index = fromIndex;
    size_t largestName = 0;
    float unused = -0.0f;

    if(writing)
    {
        gSerializationContext.File << MODIFIER_PRELUDE << '\n';

        for(size_t i = fromIndex; i < toindex; i++)
        {
            size_t nameLen = strlen(m_ModifierNames[i]) - 1;
            largestName = nameLen > largestName ? nameLen : largestName;
        }
    }

    while(gSerializationContext.File)
    {
        if(!writing)
        {
            std::getline(gSerializationContext.File, currLine);
            gSerializationContext.LineStream.clear();
            gSerializationContext.LineStream.str(currLine);

            if(currLine[0] == '/' || currLine[0] == '\0')
                continue;
        }

        auto& modifier = m_aModifiers[index];

        if(writing)
        {
            const char* name = m_ModifierNames[index];
            size_t nameLen = strlen(name) - 1;
            gSerializationContext.File << name << "    ";

            if(nameLen < largestName)
            {
                char alignmentSpaces[32]{};
                memset(alignmentSpaces, ' ', largestName - nameLen - 1);
                gSerializationContext.File << alignmentSpaces;
            }
        }
        else
        {
            gSerializationContext.LineStream >> m_ModifierNames[index];
            modifier.m_Hash = rage::atStringHash(m_ModifierNames[index]);
        }

        uint32_t nearFogAxis;
        if(writing)
            nearFogAxis = modifier.m_EnableRainEffects ? 1 : 0;

        SerializeFloat(modifier.m_fMinFarClip);
        SerializeFloat(modifier.m_fMaxFarClip);
        SerializeFloat(modifier.m_fMinFogStart);
        SerializeFloat(modifier.m_fMaxFogStart);
        SerializeRGBA32(modifier.m_AmbientColor0);
        SerializeFloat(modifier.m_fAmbientColor0Multiplier);
        SerializeRGBA32(modifier.m_AmbientColor1);
        SerializeFloat(modifier.m_fAmbientColor1Multiplier);
        SerializeRGBA32(modifier.m_DirectionalLightColor);
        SerializeFloat(modifier.m_fDirectionalLightColorMultiplier);
        SerializeFloat(modifier.m_fAmbientScale);
        SerializeRGBA32(modifier.m_FogColor);
        SerializeRGBA32(modifier.m_NearFogColor);
        SerializeFloat(unused); // fog mul
        SerializeInt(nearFogAxis);
        SerializeFloat(modifier.m_fTemperature);
        SerializeFloat(modifier.m_fPostFxStrength);
        SerializeFloat(modifier.m_fExposure);
        SerializeFloat(modifier.m_fExposureMultiplier);
        SerializeFloat(modifier.m_fBloomThreshold);
        SerializeFloat(modifier.m_fMidGrayValue);
        SerializeFloat(modifier.m_fBloomIntensity);
        SerializeRGB8(modifier.m_ColorCorrect);
        SerializeRGB8(modifier.m_ColorAdd);
        SerializeFloat(modifier.m_fDesaturation);
        SerializeFloat(modifier.m_fContrast);
        SerializeFloat(modifier.m_fGamma);
        SerializeFloat(modifier.m_fDesaturationFar);
        SerializeFloat(modifier.m_fContrastFar);
        SerializeFloat(modifier.m_fGammaFar);
        SerializeFloat(modifier.m_fDepthFxNear);
        SerializeFloat(modifier.m_fDepthFxFar);
        SerializeFloat(modifier.m_fLumMin);
        SerializeFloat(modifier.m_fLumMax);
        SerializeFloat(modifier.m_fGlobalReflectionMultiplier);
        SerializeFloat(modifier.m_fMinFarDof);
        SerializeFloat(modifier.m_fMaxFarDof);
        SerializeFloat(modifier.m_fMinNearDof);
        SerializeFloat(unused); // max near dof
        SerializeFloat(unused); // near blur
        SerializeFloat(unused); // max blur
        SerializeFloat(modifier.m_fMidDofBlur);
        SerializeFloat(modifier.m_fFarDofBlur);
        SerializeFloat(modifier.m_fWaterReflectionMultiplier);
        SerializeFloat(modifier.m_fParticleIntensity);
        SerializeFloat(modifier.m_fAoStrength);
        SerializeFloat(modifier.m_fRimLightingMultiplier);
        SerializeFloat(modifier.m_fSkyLightMultiplier);
        SerializeFloat(modifier.m_fPedAoStrength);
        SerializeInt(modifier.m_fNearColorStrength);

        if(!writing)
            modifier.m_EnableRainEffects = nearFogAxis > 0;

        index++;

        if(!writing)
            m_NumModifiers++;
        if(writing && index >= toindex)
            break;
        if(writing)
            gSerializationContext.File << '\n';
    }

    gSerializationContext = {};

    return true;
}

bool TimeCycle::LoadModifiers(char* errMessage, uint32_t errMessageSize)
{
    m_NumModifiers = 0;
    m_ModifierIndices[0] = 0;
    if(!SerializeModifier("pc/data/timecyclemodifiers.dat", errMessage, errMessageSize, false, m_ModifierIndices[0], m_ModifierIndices[1]))
        return false;
    m_ModifierIndices[1] = m_NumModifiers;
    if(!SerializeModifier("pc/data/timecyclemodifiers2.dat", errMessage, errMessageSize, false, m_ModifierIndices[1], m_ModifierIndices[2]))
        return false;
    m_ModifierIndices[2] = m_NumModifiers;
    if(!SerializeModifier("pc/data/timecyclemodifiers3.dat", errMessage, errMessageSize, false, m_ModifierIndices[2], m_ModifierIndices[3]))
        return false;
    m_ModifierIndices[3] = m_NumModifiers;
    if(!SerializeModifier("pc/data/timecyclemodifiers4.dat", errMessage, errMessageSize, false, m_ModifierIndices[3], m_NumModifiers))
        return false;

    return true;
}

bool TimeCycle::SaveModifiers(char* errMessage, uint32_t errMessageSize)
{
    if(!SerializeModifier("pc/data/timecyclemodifiers.dat", errMessage, errMessageSize, true, m_ModifierIndices[0], m_ModifierIndices[1]))
        return false;
    if(!SerializeModifier("pc/data/timecyclemodifiers2.dat", errMessage, errMessageSize, true, m_ModifierIndices[1], m_ModifierIndices[2]))
        return false;
    if(!SerializeModifier("pc/data/timecyclemodifiers3.dat", errMessage, errMessageSize, true, m_ModifierIndices[2], m_ModifierIndices[3]))
        return false;
    if(!SerializeModifier("pc/data/timecyclemodifiers4.dat", errMessage, errMessageSize, true, m_ModifierIndices[3], m_NumModifiers))
        return false;

    return true;
}

const char* TimeCycle::GetModifierNameFromIndex(uint32_t modifierIndex)
{
    assert(modifierIndex < m_NumModifiers);
    return m_ModifierNames[modifierIndex];
}

const char* TimeCycle::GetModifierNameFromHash(uint32_t hash)
{
    for(size_t i = 0; i < m_NumModifiers; i++)
    {
        if(m_aModifiers[i].m_Hash == hash)
            return m_ModifierNames[i];
    }

    return nullptr;
}