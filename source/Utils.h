#pragma once

#include <cstdint>

namespace Utils
{
    static void Float4ToU32(const float *inColor, uint32_t &outColor)
    {
        outColor = 0;
        outColor |= ((uint32_t)(inColor[0] * 255.0f + 0.5f)) << 16;
        outColor |= ((uint32_t)(inColor[1] * 255.0f + 0.5f)) << 8;
        outColor |= ((uint32_t)(inColor[2] * 255.0f + 0.5f)) << 0;
        outColor |= ((uint32_t)(inColor[3] * 255.0f + 0.5f)) << 24;
    }

    static void U32ToFloat4(const uint32_t inColor, float *outColor)
    {
        outColor[0] = ((inColor >> 16) & 0xFF) / 255.0f;
        outColor[1] = ((inColor >> 8)  & 0xFF) / 255.0f;
        outColor[2] = ((inColor >> 0)  & 0xFF) / 255.0f;
        outColor[3] = ((inColor >> 24) & 0xFF) / 255.0f;
    }

};
