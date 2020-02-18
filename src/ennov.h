#if !defined(ENNOV_H)
#include "ennov_platform.h"
#include "ennov_math.h"
#include "ennov_gl.h"

struct character_glyph
{
    texture TextureId;
    s32 Width, Height;
    vec2 Bearing;
    u32 Advance;
};

#define ENNOV_H
#endif
