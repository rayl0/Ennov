#if !defined(ENNOV_MATH_H)

// TODO(rajat): Some things are needed no matter what
#include <stdio.h>

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

inline f32 dot(const vec2& a, const vec2& b)
{
    return (a.x * b.x + a.y * b.y);
}

inline f32 length(const vec2& a)
{
    return (sqrt(a.x * a.x + a.y * a.y));
}

inline vec2 normal(const vec2& a)
{
    f32 Length = length(a);
    return {a.x / Length, a.y / Length};
}

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
    union
    {
        struct
        {
            vec2 Pos;
            vec2 Dimensions;
        };
        struct
        {
            vec2 p;
            vec2 d;
        };
        struct
        {
            f32 x, y, w, h;
        };
        f32 data[4];
    };
};

struct rect_projection_data
{
    vec2 Min;
    vec2 Max;
};

inline bool32 RectangleColloide(rect Rectangle1, rect Rectangle2)
{
  bool32 ColloideX = Rectangle1.Pos.x + Rectangle1.Dimensions.x >=
    Rectangle2.Pos.x && Rectangle2.Pos.x + Rectangle2.Dimensions.x >= Rectangle1.Pos.x;

  bool32 ColloideY = Rectangle1.Pos.y + Rectangle1.Dimensions.y >=
    Rectangle2.Pos.y && Rectangle2.Pos.y + Rectangle2.Dimensions.y >= Rectangle1.Pos.y;

  return ColloideX && ColloideY;
};

inline bool32 RectangleContainsPoint(rect Projection, vec2 Point)
{
   return ((Projection.p.x + Projection.d.x >= Point.x && Projection.p.x <= Point.x) &&
           (Projection.p.y + Projection.d.y >= Point.y && Projection.p.y <= Point.y));
}

#define ENNOV_MATH_H
#endif
