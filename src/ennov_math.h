#if !defined(ENNOV_MATH_H)
#include "ennov_platform.h"

struct vec2 
{
  union {
    struct {
      real32 x, y;
    };
    struct {
      real32 u, v;
    };
    real32 data[2];
  };
};

struct vec3
{
  union {
    struct {
      real32 x, y, z;
    };
    struct {
      vec2 xy;
      real32 __z;
    };
    struct {
      real32 __x;
      vec2 yz;
    };
    struct {
      real32 r, g, b;
    };
    real32 data[3];
  };
};

struct vec4
{
  union {
    struct{
      real32 x, y, z, w;
    };
    struct {
      vec2 xy;
      vec2 zw;
    };
    struct {
      vec2 __xy;
      vec2 uv;
    };
    struct {
      vec3 xyz;
      real32 __w;
    };
    struct {
      real32 r, g, b, a;
    };
    real32 data[4];
  };
};

inline vec2
operator-(vec2& Vector)
{
  vec2 Negative;
  Negative.x = -(Vector.x);
  Negative.y = -(Vector.y);

  return Negative;
}

inline vec2
operator-(vec2& Vec1, vec2& Vec2)
{
  vec2 Final;
  Final.x = Vec1.x - Vec2.x;
  Final.y = Vec2.y - Vec2.y;
  return Final;
}

inline vec2
operator+(vec2& Vec1, vec2& Vec2)
{ 
  vec2 Final;
  Final.x = Vec1.x + Vec2.x;
  Final.y = Vec2.y + Vec2.y;
  return Final;
}

struct rect
{
    vec2 Pos;
    vec2 Dimensions;
};

struct rect_projection_data
{
    vec2 Min;
    vec2 Max;
};

inline rect_projection_data GetRectangleProjectionData(rect Rectangle)
{
   rect_projection_data NewProjection;
   NewProjection.Min.x = Rectangle.Pos.x;
   NewProjection.Min.y = Rectangle.Pos.y;
   NewProjection.Max.x = Rectangle.Pos.x + Rectangle.Dimensions.x;
   NewProjection.Max.y = Rectangle.Pos.y + Rectangle.Dimensions.y;

   return NewProjection;
};

inline bool32 RectangleColloide(rect Rectangle1, rect Rectangle2)
{
  bool32 ColloideX = Rectangle1.Pos.x + Rectangle1.Dimensions.x >=
    Rectangle2.Pos.x && Rectangle2.Pos.x + Rectangle2.Dimensions.x >= Rectangle1.Pos.x;

  bool32 ColloideY = Rectangle1.Pos.y + Rectangle1.Dimensions.y >=
    Rectangle2.Pos.y && Rectangle2.Pos.y + Rectangle2.Dimensions.y >= Rectangle1.Pos.y;

  return ColloideX && ColloideY;
};

inline bool32 RectangleContainsPoint(rect Rectangle, vec2 Point)
{
   rect_projection_data Projection = GetRectangleProjectionData(Rectangle);

   return ((Projection.Max.x >= Point.x && Projection.Min.x <= Point.x) &&
           (Projection.Max.y >= Point.y && Projection.Min.y <= Point.y));
}

#define ENNOV_MATH_H
#endif
