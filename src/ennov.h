#if !defined(ENNOV_H)
#include "ennov_platform.h"
#include "ennov_math.h"

struct rect_draw_attribs
{
  vec2 Position;
  vec2 Dimensions;
  vec4 Color;
  loaded_bitmap* Texture;
  uint32 Id;
};

enum RectangleDrawFlags
{
    RECTANGLE_FILL_TEXCOLOR = 0,
    RECTANGLE_FILL_COLOR = (1 << 0),
    RECTANGLE_FILL_TEXTURE = (1 << 1)
};

void DrawRectangle(rect_draw_attribs* DrawAttribs, uint32 DrawFlags);

#define ENNOV_H
#endif 
