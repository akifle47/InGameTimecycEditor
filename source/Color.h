#pragma once

#include <cstdint>

struct Colorf
{
    float Red;
    float Green;
    float Blue;
    float Alpha;
};

struct Color32
{
    union
    {
        struct
        {
            uint32_t Blue  : 8;
            uint32_t Green : 8;
            uint32_t Red   : 8;
            uint32_t Alpha : 8;
        };
        uint32_t BGRA;
    };
};


static inline Color32 Color32FromColorf(Colorf color)
{
    Color32 color32;
    color32.Red   = uint8_t(color.Red   * 255.0f + 0.5f);
    color32.Green = uint8_t(color.Green * 255.0f + 0.5f);
    color32.Blue  = uint8_t(color.Blue  * 255.0f + 0.5f);
    color32.Alpha = uint8_t(color.Alpha * 255.0f + 0.5f);

    return color32;
}

static inline Colorf ColorfFromColor32(Color32 color)
{
    return Colorf
    {
        .Red   = (float)color.Red   / 255.0f,
        .Green = (float)color.Green / 255.0f,
        .Blue  = (float)color.Blue  / 255.0f,
        .Alpha = (float)color.Alpha / 255.0f,
    };
}