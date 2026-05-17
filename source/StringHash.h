#pragma once

namespace rage
{
    constexpr uint32_t atStringHash(const char* str, uint32_t seed = 0)
    {
        if(*str == '"')
            str++;

        for(char chr = *str; *str; chr = *++str)
        {
            if(*str == '"')
                break;

            if((uint8_t)(chr - 'A') > 25)
            {
                if(chr == '\\')
                    chr = '/';
            }
            else
            {
                chr += 0x20;
            }

            seed = ((0x401 * (seed + chr)) >> 6) ^ (0x401 * (seed + chr));
        }

        return 32769 * ((9 * seed) ^ ((9 * seed) >> 11));
    }
}