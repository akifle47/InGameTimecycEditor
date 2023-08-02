#pragma once
#include <cstdint>
#include <string>

const uint32_t NUM_HOURS = 11;
const uint32_t NUM_WEATHERS = 9;

class TimeCycleParams
{
public:
	//unknown means its a timecyc.dat parameter and i dont know what it does
	//unused means its not a timecyc.dat parameter and i dont care what it does
	uint32_t mAmbient0Color;
	uint32_t mAmbient1Color;
	uint32_t mDirLightColor;
	float mDirLightMultiplier;
	float mAmbient0Multiplier;
	float mAmbient1Multiplier;
	float mAOStrength;
	float mPedAOStrength;
	float mRimLightingMultiplier;
	float mSkyLightMultiplier;
	float mDirLightSpecMultiplier;
	uint32_t mSkyBottomColorFogDensity;
	uint32_t mSunCore;
	float mCoronaBrightness;
	float mCoronaSize;
	float mDistantCoronaBrightness;
	float mDistantCoronaSize;
	float mFarClip;
	float mFogStart;
	float mDOFStart;
	float mNearDOFBlur;
	float mFarDOFBlur;
	uint32_t mLowCloudsColor;
	uint32_t mBottomCloudsColor;
	uint32_t mWater;
	float mUnused1[7];
	float mWaterReflectionMultiplier;
	float mParticleBrightness;
	float mExposure;
	float mBloomThreshold;
	float mMidGrayValue;
	float mBloomIntensity;
	uint32_t mColorCorrection;
	uint32_t mColorAdd;
	float mDesaturation;
	float mContrast;
	float mGamma;
	float mDesaturationFar;
	float mContrastFar;
	float mGammaFar;
	float mDepthFxNear;
	float mDepthFxFar;
	float mLumMin;
	float mLumMax;
	float mLumDelay;
	int32_t mCloudAlpha;
	float mUnused2;
	float mTemperature;
	float mGlobalReflectionMultiplier;
	float mUnused3;
	float mSkyColor[3];
	float mUnused4;
	float mSkyHorizonColor[3];
	float mUnused5;
	float mSkyEastHorizonColor[3];
	float mUnused6;
	float mCloud1Color[3];
	float mUnknown3;
	float mSkyHorizonHeight;
	float mSkyHorizonBrightness;
	float mSunAxisX;
	float mSunAxisY;
	float mCloud2Color[3];
	float mUnused7;
	float mCloud2ShadowStrength;
	float mCloud2Threshold;
	float mCloud2Bias1;
	float mCloud2Scale;
	float mCloudInScatteringRange;
	float mCloud2Bias2;
	float mDetailNoiseScale;
	float mDetailNoiseMultiplier;
	float mCloud2Offset;
	float mCloudWarp;
	float mCloudsFadeOut;
	float mCloud1Bias;
	float mCloud1Detail;
	float mCloud1Threshold;
	float mCloud1Height;
	float mUnused8;
	float mCloud3Color[3];
	float mUnused9;
	float mUnknown29;
	float mUnused10[3];
	float mSunColor[3];
	float mUnused11;
	float mCloudsBrightness;
	float mDetailNoiseOffset;
	float mStarsBrightness;
	float mVisibleStars;
	float mMoonBrightness;
	float mUnused12[3];
	float mMoonColor[3];
	float mUnused13;
	float mMoonGlow;
	float mMoonParam3;
	float SunCenterStart;
	float SunCenterEnd;
	float mSunSize;
	float mUnused14[3];
	float mUnknown46;
	float mSkyBrightness;
	float mUnused15;
	int32_t mFilmGrain;
};

class Timecycle
{
public:
	TimeCycleParams mParams[NUM_HOURS][NUM_WEATHERS];

	bool Save(const char *fileName, char *errMessage, uint32_t errMessageSize);
	bool Load(const char *fileName, char *errMessage, uint32_t errMessageSize);
};