#if !defined(ENNOV_MATH_H)
#include "ennov_platform.h"

struct vec2 
{
    real32 X, Y;
};

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
   NewProjection.Min.X = Rectangle.Pos.X;
   NewProjection.Min.Y = Rectangle.Pos.Y;
   NewProjection.Max.X = Rectangle.Pos.X + Rectangle.Dimensions.X;
   NewProjection.Max.Y = Rectangle.Pos.Y + Rectangle.Dimensions.Y;

   return NewProjection;
};

inline bool32 RectangleContainsPoint(rect Rectangle, vec2 Point)
{
   rect_projection_data Projection = GetRectangleProjectionData(Rectangle);

   return ((Projection.Max.X > Point.X && Projection.Min.X < Point.X) &&
          (Projection.Max.Y > Point.Y && Projection.Min.Y < Point.Y));
}

#define ENNOV_MATH_H
#endif
