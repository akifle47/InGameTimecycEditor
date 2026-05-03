#include "TimeCycle.h"
#include <fstream>
#include <string>
#include <sstream>

void WriteColorRGB(std::ofstream& file, Color32 color)
{
    file << (uint32_t)color.Red << " " << (uint32_t)color.Green << " " << (uint32_t)color.Blue << " ";
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

            file << (m_ColourSets[time][weather].m_SkyHatSettings.m_fSkyColor[0]) << " ";
            file << (m_ColourSets[time][weather].m_SkyHatSettings.m_fSkyColor[1]) << " ";
            file << (m_ColourSets[time][weather].m_SkyHatSettings.m_fSkyColor[2]) << "    ";

            file << (m_ColourSets[time][weather].m_SkyHatSettings.m_fSkyHorizonColor[0]) << " ";
            file << (m_ColourSets[time][weather].m_SkyHatSettings.m_fSkyHorizonColor[1]) << " ";
            file << (m_ColourSets[time][weather].m_SkyHatSettings.m_fSkyHorizonColor[2]) << "    ";

            file << (m_ColourSets[time][weather].m_SkyHatSettings.m_fSkyEastHorizonColor[0]) << " ";
            file << (m_ColourSets[time][weather].m_SkyHatSettings.m_fSkyEastHorizonColor[1]) << " ";
            file << (m_ColourSets[time][weather].m_SkyHatSettings.m_fSkyEastHorizonColor[2]) << "    ";

            file << (m_ColourSets[time][weather].m_SkyHatSettings.m_fSunsetColor[0]) << " ";
            file << (m_ColourSets[time][weather].m_SkyHatSettings.m_fSunsetColor[1]) << " ";
            file << (m_ColourSets[time][weather].m_SkyHatSettings.m_fSunsetColor[2]) << "    ";

            file << m_ColourSets[time][weather].m_SkyHatSettings.m_fUnknown_3C << "    ";
            file << m_ColourSets[time][weather].m_SkyHatSettings.m_fSkyHorizonHeight << "    ";
            file << m_ColourSets[time][weather].m_SkyHatSettings.m_fSkyHorizonBrightness << "    ";
            file << unusedParam << "    ";
            file << unusedParam << "    ";

            file << (m_ColourSets[time][weather].m_SkyHatSettings.m_fCloudColor[0]) << " ";
            file << (m_ColourSets[time][weather].m_SkyHatSettings.m_fCloudColor[1]) << " ";
            file << (m_ColourSets[time][weather].m_SkyHatSettings.m_fCloudColor[2]) << "    ";

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

            file << (m_ColourSets[time][weather].m_SkyHatSettings.m_fTopCloudColor[0]) << " ";
            file << (m_ColourSets[time][weather].m_SkyHatSettings.m_fTopCloudColor[1]) << " ";
            file << (m_ColourSets[time][weather].m_SkyHatSettings.m_fTopCloudColor[2]) << "    ";

            file << m_ColourSets[time][weather].m_SkyHatSettings.m_fTopCloudLight << "    ";

            file << (m_ColourSets[time][weather].m_SkyHatSettings.m_fSunColor[0]) << " ";
            file << (m_ColourSets[time][weather].m_SkyHatSettings.m_fSunColor[1]) << " ";
            file << (m_ColourSets[time][weather].m_SkyHatSettings.m_fSunColor[2]) << "    ";

            file << m_ColourSets[time][weather].m_SkyHatSettings.m_fUnderlighting << "    ";
            file << m_ColourSets[time][weather].m_SkyHatSettings.m_fDetailNoiseOffset << "    ";
            file << m_ColourSets[time][weather].m_SkyHatSettings.m_fStarFieldBrightness << "    ";
            file << m_ColourSets[time][weather].m_SkyHatSettings.m_fStarFieldThreshold << "    ";
            file << m_ColourSets[time][weather].m_SkyHatSettings.m_fMoonBrightness << "    ";

            file << (m_ColourSets[time][weather].m_SkyHatSettings.m_fMoonColor[0]) << " ";
            file << (m_ColourSets[time][weather].m_SkyHatSettings.m_fMoonColor[1]) << " ";
            file << (m_ColourSets[time][weather].m_SkyHatSettings.m_fMoonColor[2]) << "    ";

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

Color32 ReadColorRGB(std::stringstream& ss)
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

            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_fSkyColor[0];
            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_fSkyColor[1];
            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_fSkyColor[2];

            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_fSkyHorizonColor[0];
            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_fSkyHorizonColor[1];
            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_fSkyHorizonColor[2];
            
            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_fSkyEastHorizonColor[0];
            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_fSkyEastHorizonColor[1];
            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_fSkyEastHorizonColor[2];

            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_fSunsetColor[0];
            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_fSunsetColor[1];
            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_fSunsetColor[2];

            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_fUnknown_3C;
            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_fSkyHorizonHeight;
            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_fSkyHorizonBrightness;

            currLineSS >> tempUnusedParam >> tempUnusedParam;
            
            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_fCloudColor[0];
            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_fCloudColor[1];
            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_fCloudColor[2];

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

            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_fTopCloudColor[0];
            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_fTopCloudColor[1];
            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_fTopCloudColor[2];

            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_fTopCloudLight;

            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_fSunColor[0];
            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_fSunColor[1];
            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_fSunColor[2];

            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_fUnderlighting;
            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_fDetailNoiseOffset;
            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_fStarFieldBrightness;
            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_fStarFieldThreshold;
            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_fMoonBrightness;

            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_fMoonColor[0];
            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_fMoonColor[1];
            currLineSS >> m_ColourSets[time][weather].m_SkyHatSettings.m_fMoonColor[2];
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