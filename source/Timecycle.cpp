#include "TimeCycle.h"
#include <fstream>
#include <string>
#include <sstream>
#include <Windows.h>

bool Timecycle::Save(const char *fileName, char *errMessage, uint32_t errMessageSize)
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

	const uint32_t redShift = 16, greenShift = 8, blueShift = 0, alphaShift = 24;
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

			file << ((mParams[time][weather].mAmbient0Color >> redShift) & 0xFF) << " ";
			file << ((mParams[time][weather].mAmbient0Color >> greenShift) & 0xFF) << " ";
			file << ((mParams[time][weather].mAmbient0Color >> blueShift) & 0xFF) << "  ";
			
			file << ((mParams[time][weather].mAmbient1Color >> redShift) & 0xFF) << " ";
			file << ((mParams[time][weather].mAmbient1Color >> greenShift) & 0xFF) << " ";
			file << ((mParams[time][weather].mAmbient1Color >> blueShift) & 0xFF) << "  ";

			file << ((mParams[time][weather].mDirLightColor >> redShift) & 0xFF) << " ";
			file << ((mParams[time][weather].mDirLightColor >> greenShift) & 0xFF) << " ";
			file << ((mParams[time][weather].mDirLightColor >> blueShift) & 0xFF) << "  ";

			file << unusedParam << "  ";
			file << mParams[time][weather].mFilmGrain << "  ";
			file << ((mParams[time][weather].mSkyBottomColorFogDensity >> alphaShift) & 0xFF) << "  ";

			file << ((mParams[time][weather].mSkyBottomColorFogDensity >> redShift) & 0xFF) << " ";
			file << ((mParams[time][weather].mSkyBottomColorFogDensity >> greenShift) & 0xFF) << " ";
			file << ((mParams[time][weather].mSkyBottomColorFogDensity >> blueShift) & 0xFF) << "  ";

			file << ((mParams[time][weather].mSunCore >> redShift) & 0xFF) << " ";
			file << ((mParams[time][weather].mSunCore >> greenShift) & 0xFF) << " ";
			file << ((mParams[time][weather].mSunCore >> blueShift) & 0xFF) << "  ";

			file << int32_t(unusedParam) << "  " << int32_t(unusedParam) << "  " << int32_t(unusedParam) << "  ";
			file << unusedParam << "  ";
			
			file << (mParams[time][weather].mCoronaBrightness / 10.0f) << "  ";
			file << mParams[time][weather].mFarClip << "  ";
			file << mParams[time][weather].mFogStart << "  ";

			file << ((mParams[time][weather].mLowCloudsColor >> redShift) & 0xFF) << " ";
			file << ((mParams[time][weather].mLowCloudsColor >> greenShift) & 0xFF) << " ";
			file << ((mParams[time][weather].mLowCloudsColor >> blueShift) & 0xFF) << "  ";

			file << ((mParams[time][weather].mBottomCloudsColor >> redShift) & 0xFF) << " ";
			file << ((mParams[time][weather].mBottomCloudsColor >> greenShift) & 0xFF) << " ";
			file << ((mParams[time][weather].mBottomCloudsColor >> blueShift) & 0xFF) << "  ";

			file << ((mParams[time][weather].mWater >> redShift) & 0xFF) << " ";
			file << ((mParams[time][weather].mWater >> greenShift) & 0xFF) << " ";
			file << ((mParams[time][weather].mWater >> blueShift) & 0xFF) << " ";
			file << ((mParams[time][weather].mWater >> alphaShift) & 0xFF) << "  ";

			file << mParams[time][weather].mExposure << "  ";
			file << mParams[time][weather].mBloomThreshold << "  ";
			file << mParams[time][weather].mMidGrayValue << "  ";
			file << mParams[time][weather].mBloomIntensity << "  ";

			file << ((mParams[time][weather].mColorCorrection >> redShift) & 0xFF) << " ";
			file << ((mParams[time][weather].mColorCorrection >> greenShift) & 0xFF) << " ";
			file << ((mParams[time][weather].mColorCorrection >> blueShift) & 0xFF) << "  ";

			file << ((mParams[time][weather].mColorAdd >> redShift) & 0xFF) << " ";
			file << ((mParams[time][weather].mColorAdd >> greenShift) & 0xFF) << " ";
			file << ((mParams[time][weather].mColorAdd >> blueShift) & 0xFF) << "  ";

			file << mParams[time][weather].mDesaturation << "  ";
			file << mParams[time][weather].mContrast << "  ";
			file << mParams[time][weather].mGamma << "  ";
			file << mParams[time][weather].mDesaturationFar << "  ";
			file << mParams[time][weather].mContrastFar << "  ";
			file << mParams[time][weather].mGammaFar << "  ";
			file << mParams[time][weather].mDepthFxNear << "  ";
			file << mParams[time][weather].mDepthFxFar << "  ";
			file << mParams[time][weather].mLumMin << "  ";
			file << mParams[time][weather].mLumMax << "  ";
			file << mParams[time][weather].mLumDelay << "  ";
			file << mParams[time][weather].mCloudAlpha << "  ";
			file << mParams[time][weather].mDirLightMultiplier << "  ";
			file << mParams[time][weather].mAmbient0Multiplier << "  ";
			file << mParams[time][weather].mAmbient1Multiplier << "  ";
			file << mParams[time][weather].mSkyLightMultiplier << "  ";
			file << mParams[time][weather].mDirLightSpecMultiplier << "  ";
			file << int32_t(mParams[time][weather].mTemperature) << "  ";
			file << mParams[time][weather].mGlobalReflectionMultiplier << "  ";

			file << (mParams[time][weather].mSkyColor[0]) << " ";
			file << (mParams[time][weather].mSkyColor[1]) << " ";
			file << (mParams[time][weather].mSkyColor[2]) << "  ";

			file << (mParams[time][weather].mSkyHorizonColor[0]) << " ";
			file << (mParams[time][weather].mSkyHorizonColor[1]) << " ";
			file << (mParams[time][weather].mSkyHorizonColor[2]) << "  ";

			file << (mParams[time][weather].mSkyEastHorizonColor[0]) << " ";
			file << (mParams[time][weather].mSkyEastHorizonColor[1]) << " ";
			file << (mParams[time][weather].mSkyEastHorizonColor[2]) << "  ";

			file << (mParams[time][weather].mCloud1Color[0]) << " ";
			file << (mParams[time][weather].mCloud1Color[1]) << " ";
			file << (mParams[time][weather].mCloud1Color[2]) << "  ";

			file << mParams[time][weather].mUnknown3 << "  ";
			file << mParams[time][weather].mSkyHorizonHeight << "  ";
			file << mParams[time][weather].mSkyHorizonBrightness << "  ";
			file << unusedParam << "  ";
			file << unusedParam << "  ";

			file << (mParams[time][weather].mCloud2Color[0]) << " ";
			file << (mParams[time][weather].mCloud2Color[1]) << " ";
			file << (mParams[time][weather].mCloud2Color[2]) << "  ";

			file << mParams[time][weather].mCloud2ShadowStrength << "  ";
			file << mParams[time][weather].mCloud2Threshold << "  ";
			file << mParams[time][weather].mCloud2Bias1 << "  ";
			file << mParams[time][weather].mCloud2Scale << "  ";
			file << unusedParam << " ";
			file << mParams[time][weather].mCloud2Bias2 << "  ";
			file << mParams[time][weather].mDetailNoiseScale << "  ";
			file << mParams[time][weather].mDetailNoiseMultiplier << "  ";
			file << mParams[time][weather].mCloud2Offset << "  ";
			file << unusedParam << "  ";
			file << mParams[time][weather].mCloudsFadeOut << "  ";
			file << mParams[time][weather].mCloud1Bias << "  ";
			file << mParams[time][weather].mCloud1Detail << "  ";
			file << mParams[time][weather].mCloud1Threshold << "  ";
			file << mParams[time][weather].mCloud1Height << "  ";

			file << (mParams[time][weather].mCloud3Color[0]) << " ";
			file << (mParams[time][weather].mCloud3Color[1]) << " ";
			file << (mParams[time][weather].mCloud3Color[2]) << "  ";

			file << mParams[time][weather].mUnknown29 << "  ";

			file << (mParams[time][weather].mSunColor[0]) << " ";
			file << (mParams[time][weather].mSunColor[1]) << " ";
			file << (mParams[time][weather].mSunColor[2]) << "  ";

			file << mParams[time][weather].mCloudsBrightness << "  ";
			file << mParams[time][weather].mDetailNoiseOffset << "  ";
			file << mParams[time][weather].mStarsBrightness << "  ";
			file << mParams[time][weather].mVisibleStars << "  ";
			file << mParams[time][weather].mMoonBrightness << "  ";

			file << (mParams[time][weather].mMoonColor[0]) << " ";
			file << (mParams[time][weather].mMoonColor[1]) << " ";
			file << (mParams[time][weather].mMoonColor[2]) << "  ";

			file << mParams[time][weather].mMoonGlow << "  ";
			file << mParams[time][weather].mMoonParam3 << "  ";
			file << unusedParam << "  ";
			file << unusedParam << "  ";
			file << mParams[time][weather].mSunSize << "  ";
			file << mParams[time][weather].mUnknown46 << "  ";
			file << mParams[time][weather].mDOFStart << "  ";
			file << unusedParam << "  ";
			file << unusedParam << "  ";
			file << mParams[time][weather].mNearDOFBlur << "  ";
			file << mParams[time][weather].mFarDOFBlur << "  ";
			file << mParams[time][weather].mWaterReflectionMultiplier << "  ";
			file << mParams[time][weather].mParticleBrightness << "  ";
			file << mParams[time][weather].mCoronaSize << "  ";
			file << mParams[time][weather].mSkyBrightness << "  ";
			file << mParams[time][weather].mAOStrength << "  ";
			file << mParams[time][weather].mRimLightingMultiplier << "  ";
			file << (mParams[time][weather].mDistantCoronaBrightness / 10.0f) << "  ";
			file << mParams[time][weather].mDistantCoronaSize << "  ";
			file << mParams[time][weather].mPedAOStrength << "\n";
		}
	}

	return true;
}

bool Timecycle::Load(const char *fileName, char *errMessage, uint32_t errMessageSize)
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
	uint32_t color[4] = {};

	const uint32_t redShift = 16, greenShift = 8, blueShift = 0, alphaShift = 24;

	while(file)
	{
		std::getline(file, currLine);

		if(currLine[0] == '/' || currLine[0] == '\0')
			continue;

		for(uint32_t time = 0; time < NUM_HOURS; time++)
		{
			currLineSS.clear();
			currLineSS.str(currLine);

			currLineSS >> color[0] >> color[1] >> color[2];
			mParams[time][weather].mAmbient0Color = 0;
			mParams[time][weather].mAmbient0Color |= color[0] << redShift;
			mParams[time][weather].mAmbient0Color |= color[1] << greenShift;
			mParams[time][weather].mAmbient0Color |= color[2] << blueShift;

			currLineSS >> color[0] >> color[1] >> color[2];
			mParams[time][weather].mAmbient1Color = 0;
			mParams[time][weather].mAmbient1Color |= color[0] << redShift;
			mParams[time][weather].mAmbient1Color |= color[1] << greenShift;
			mParams[time][weather].mAmbient1Color |= color[2] << blueShift;

			currLineSS >> color[0] >> color[1] >> color[2];
			mParams[time][weather].mDirLightColor = 0;
			mParams[time][weather].mDirLightColor |= color[0] << redShift;
			mParams[time][weather].mDirLightColor |= color[1] << greenShift;
			mParams[time][weather].mDirLightColor |= color[2] << blueShift;
			
			currLineSS >> tempUnusedParam;
			currLineSS >> mParams[time][weather].mFilmGrain;
			
			currLineSS >> color[3] >> color[0] >> color[1] >> color[2];
			mParams[time][weather].mSkyBottomColorFogDensity = 0;
			mParams[time][weather].mSkyBottomColorFogDensity |= color[0] << redShift;
			mParams[time][weather].mSkyBottomColorFogDensity |= color[1] << greenShift;
			mParams[time][weather].mSkyBottomColorFogDensity |= color[2] << blueShift;
			mParams[time][weather].mSkyBottomColorFogDensity |= color[3] << alphaShift;

			currLineSS >> color[0] >> color[1] >> color[2];
			mParams[time][weather].mSunCore = 0;
			mParams[time][weather].mSunCore |= color[0] << redShift;
			mParams[time][weather].mSunCore |= color[1] << greenShift;
			mParams[time][weather].mSunCore |= color[2] << blueShift;

			currLineSS >> tempUnusedParam >> tempUnusedParam >> tempUnusedParam >> tempUnusedParam;
			currLineSS >> mParams[time][weather].mCoronaBrightness;
			mParams[time][weather].mCoronaBrightness *= 10.0f;

			currLineSS >> mParams[time][weather].mFarClip;
			currLineSS >> mParams[time][weather].mFogStart;

			currLineSS >> color[0] >> color[1] >> color[2];
			mParams[time][weather].mLowCloudsColor = 0;
			mParams[time][weather].mLowCloudsColor |= color[0] << redShift;
			mParams[time][weather].mLowCloudsColor |= color[1] << greenShift;
			mParams[time][weather].mLowCloudsColor |= color[2] << blueShift;

			currLineSS >> color[0] >> color[1] >> color[2];
			mParams[time][weather].mBottomCloudsColor = 0;
			mParams[time][weather].mBottomCloudsColor |= color[0] << redShift;
			mParams[time][weather].mBottomCloudsColor |= color[1] << greenShift;
			mParams[time][weather].mBottomCloudsColor |= color[2] << blueShift;

			currLineSS >> color[0] >> color[1] >> color[2] >> color[3];
			mParams[time][weather].mWater = 0;
			mParams[time][weather].mWater |= color[0] << redShift;
			mParams[time][weather].mWater |= color[1] << greenShift;
			mParams[time][weather].mWater |= color[2] << blueShift;
			mParams[time][weather].mWater |= color[3] << alphaShift;

			currLineSS >> mParams[time][weather].mExposure;
			currLineSS >> mParams[time][weather].mBloomThreshold;
			currLineSS >> mParams[time][weather].mMidGrayValue;
			currLineSS >> mParams[time][weather].mBloomIntensity;

			currLineSS >> color[0] >> color[1] >> color[2];
			mParams[time][weather].mColorCorrection = 0;
			mParams[time][weather].mColorCorrection |= color[0] << redShift;
			mParams[time][weather].mColorCorrection |= color[1] << greenShift;
			mParams[time][weather].mColorCorrection |= color[2] << blueShift;

			currLineSS >> color[0] >> color[1] >> color[2];
			mParams[time][weather].mColorAdd = 0;
			mParams[time][weather].mColorAdd |= color[0] << redShift;
			mParams[time][weather].mColorAdd |= color[1] << greenShift;
			mParams[time][weather].mColorAdd |= color[2] << blueShift;

			currLineSS >> mParams[time][weather].mDesaturation;
			currLineSS >> mParams[time][weather].mContrast;
			currLineSS >> mParams[time][weather].mGamma;
			currLineSS >> mParams[time][weather].mDesaturationFar;
			currLineSS >> mParams[time][weather].mContrastFar;
			currLineSS >> mParams[time][weather].mGammaFar;
			currLineSS >> mParams[time][weather].mDepthFxNear;
			currLineSS >> mParams[time][weather].mDepthFxFar;
			currLineSS >> mParams[time][weather].mLumMin;
			currLineSS >> mParams[time][weather].mLumMax;
			currLineSS >> mParams[time][weather].mLumDelay;
			currLineSS >> mParams[time][weather].mCloudAlpha;
			currLineSS >> mParams[time][weather].mDirLightMultiplier;
			currLineSS >> mParams[time][weather].mAmbient0Multiplier;
			currLineSS >> mParams[time][weather].mAmbient1Multiplier;
			currLineSS >> mParams[time][weather].mSkyLightMultiplier;
			currLineSS >> mParams[time][weather].mDirLightSpecMultiplier;
			currLineSS >> mParams[time][weather].mTemperature;
			currLineSS >> mParams[time][weather].mGlobalReflectionMultiplier;

			currLineSS >> mParams[time][weather].mSkyColor[0];
			currLineSS >> mParams[time][weather].mSkyColor[1];
			currLineSS >> mParams[time][weather].mSkyColor[2];

			currLineSS >> mParams[time][weather].mSkyHorizonColor[0];
			currLineSS >> mParams[time][weather].mSkyHorizonColor[1];
			currLineSS >> mParams[time][weather].mSkyHorizonColor[2];
			
			currLineSS >> mParams[time][weather].mSkyEastHorizonColor[0];
			currLineSS >> mParams[time][weather].mSkyEastHorizonColor[1];
			currLineSS >> mParams[time][weather].mSkyEastHorizonColor[2];

			currLineSS >> mParams[time][weather].mCloud1Color[0];
			currLineSS >> mParams[time][weather].mCloud1Color[1];
			currLineSS >> mParams[time][weather].mCloud1Color[2];

			currLineSS >> mParams[time][weather].mUnknown3;
			currLineSS >> mParams[time][weather].mSkyHorizonHeight;
			currLineSS >> mParams[time][weather].mSkyHorizonBrightness;

			currLineSS >> tempUnusedParam >> tempUnusedParam;
			
			currLineSS >> mParams[time][weather].mCloud2Color[0];
			currLineSS >> mParams[time][weather].mCloud2Color[1];
			currLineSS >> mParams[time][weather].mCloud2Color[2];

			currLineSS >> mParams[time][weather].mCloud2ShadowStrength;
			currLineSS >> mParams[time][weather].mCloud2Threshold;
			currLineSS >> mParams[time][weather].mCloud2Bias1;
			currLineSS >> mParams[time][weather].mCloud2Scale;
			currLineSS >> tempUnusedParam;
			currLineSS >> mParams[time][weather].mCloud2Bias2;
			currLineSS >> mParams[time][weather].mDetailNoiseScale;
			currLineSS >> mParams[time][weather].mDetailNoiseMultiplier;
			currLineSS >> mParams[time][weather].mCloud2Offset;
			currLineSS >> tempUnusedParam;
			currLineSS >> mParams[time][weather].mCloudsFadeOut;
			currLineSS >> mParams[time][weather].mCloud1Bias;
			currLineSS >> mParams[time][weather].mCloud1Detail;
			currLineSS >> mParams[time][weather].mCloud1Threshold;
			currLineSS >> mParams[time][weather].mCloud1Height;

			currLineSS >> mParams[time][weather].mCloud3Color[0];
			currLineSS >> mParams[time][weather].mCloud3Color[1];
			currLineSS >> mParams[time][weather].mCloud3Color[2];

			currLineSS >> mParams[time][weather].mUnknown29;

			currLineSS >> mParams[time][weather].mSunColor[0];
			currLineSS >> mParams[time][weather].mSunColor[1];
			currLineSS >> mParams[time][weather].mSunColor[2];

			currLineSS >> mParams[time][weather].mCloudsBrightness;
			currLineSS >> mParams[time][weather].mDetailNoiseOffset;
			currLineSS >> mParams[time][weather].mStarsBrightness;
			currLineSS >> mParams[time][weather].mVisibleStars;
			currLineSS >> mParams[time][weather].mMoonBrightness;

			currLineSS >> mParams[time][weather].mMoonColor[0];
			currLineSS >> mParams[time][weather].mMoonColor[1];
			currLineSS >> mParams[time][weather].mMoonColor[2];

			currLineSS >> mParams[time][weather].mMoonGlow;
			currLineSS >> mParams[time][weather].mMoonParam3;

			currLineSS >> tempUnusedParam >> tempUnusedParam;
			currLineSS >> mParams[time][weather].mSunSize;
			currLineSS >> mParams[time][weather].mUnknown46;
			currLineSS >> mParams[time][weather].mDOFStart;
			currLineSS >> tempUnusedParam >> tempUnusedParam;
			currLineSS >> mParams[time][weather].mNearDOFBlur;
			currLineSS >> mParams[time][weather].mFarDOFBlur;
			currLineSS >> mParams[time][weather].mWaterReflectionMultiplier;
			currLineSS >> mParams[time][weather].mParticleBrightness;
			currLineSS >> mParams[time][weather].mCoronaSize;
			currLineSS >> mParams[time][weather].mSkyBrightness;
			currLineSS >> mParams[time][weather].mAOStrength;
			currLineSS >> mParams[time][weather].mRimLightingMultiplier;
			currLineSS >> mParams[time][weather].mDistantCoronaBrightness;
			mParams[time][weather].mDistantCoronaBrightness *= 10.0f;
			currLineSS >> mParams[time][weather].mDistantCoronaSize;
			currLineSS >> mParams[time][weather].mPedAOStrength;

			std::getline(file, currLine);
			while(currLine[0] == '/')
				std::getline(file, currLine);
		}

		weather++;
	}

	unusedParam = tempUnusedParam;

	return true;
}